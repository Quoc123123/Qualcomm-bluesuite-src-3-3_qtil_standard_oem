//*******************************************************************************
//
//  HidDfuEngine.cpp
//
//  Copyright (c) 2012-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Internal functions implementing Device Firmware Update using HID driver.
//
//*******************************************************************************

#include <sstream>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <iomanip>
#include <sstream>
#include <string>

#include "HidDfu.h"
#include "HidDfuEngine.h"
#include "HidDfuDeviceApplication.h"
#include "HidDfuDeviceLoader.h"

#include "DFUEngine\CRC.h"
#include "time\hi_res_clock.h"
#include "time\stop_watch.h"

#include <setupapi.h>
extern "C"
{
#include <hidsdi.h>
}

////////////////////////////////////////////////////////////////////////////////

const CHidDfuEngine::DfuFileSuffix CHidDfuEngine::STANDARD_DFU_SUFFIX = 
{
    0x0001,         // bcdDevice
    0xFFFE,         // idProduct
    0x0A12,         // idVendor
    0x0100,         // bcdDfu
    {'U','F','D'},  // ucDfuSignature
    0x10,           // bLength
    0               // dwCrc - this value is ignored (CRC calculated)
};

////////////////////////////////////////////////////////////////////////////////

CHidDfuEngine::CHidDfuEngine()
: mProgress(0)
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuEngine::~CHidDfuEngine()
{
    FUNCTION_DEBUG_SENTRY;

    DisconnectDevice();
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::ConnectDevice(uint16 aVid, uint16 aPid, uint16 aUsage,
                                   uint16 aUsagePage, uint16* apCount)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // If device objects already exist return failure
    if ((mHidDfuDevices.size() > 0) || (mHidDevInfo.size() > 0))
    {
        retVal = mLastError.SetMsg(HIDDFU_ERROR_CONNECTION, "Already connected");
    }
    else
    {
        *apCount = 0;
        GUID guid;
        // Get device intereface GUID
        HidD_GetHidGuid(&guid);
        // Get a handle to the device information set (for HID Class devices)
        // that contains requested device information elements
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (deviceInfoSet == INVALID_HANDLE_VALUE)
        {
            retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Failed to get Device Information Set");
        }
        else
        {
            // Iterate through the devices found - look for one that matches the VID, PID, usage and usage page
            SP_INTERFACE_DEVICE_DATA interfaceDeviceData;
            interfaceDeviceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
            for (uint32 index = 0;
                SetupDiEnumDeviceInterfaces(deviceInfoSet, 0, &guid, index, &interfaceDeviceData);
                ++index)
            {
                // Attempt to open the device

                DWORD requiredSize;
                // Get requiredSize to allocate the heap, to get details about the device interface,
                // following function is expected to return false/failure, hence ignore the return value
                (void)SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &interfaceDeviceData,
                        NULL, 0, &requiredSize, NULL);

                PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData =
                    (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(),
                        HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, requiredSize);

                // Get Device Information Data, to retrieve the Device Instance
                SP_DEVINFO_DATA devInfoData;
                ZeroMemory(&devInfoData, sizeof(devInfoData));
                devInfoData.cbSize = sizeof(devInfoData);

                deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                // Get details about the device interface
                if(!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &interfaceDeviceData,
                    deviceInterfaceDetailData, requiredSize, NULL, &devInfoData))
                {
                    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Failed to get Device Interface Detail");
                }
                else
                {
                    HANDLE deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

                    if (deviceHandle != INVALID_HANDLE_VALUE)
                    {
                        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Check capabilities, device: %s", deviceInterfaceDetailData->DevicePath);

                        HIDD_ATTRIBUTES attributes;
                        HidD_GetAttributes(deviceHandle, &attributes);

                        if (attributes.VendorID == aVid && attributes.ProductID == aPid)
                        {
                            PHIDP_PREPARSED_DATA preparsedData;
                            HidD_GetPreparsedData(deviceHandle, &preparsedData);

                            HIDP_CAPS capabilities;
                            HidP_GetCaps(preparsedData, &capabilities);

                            // Check usage and usage page match
                            // usage and usagePage check is optional (zero is reserved 
                            // value in both cases, so using zero as "don't check").
                            if ((aUsage == 0 || capabilities.Usage == aUsage) &&
                                (aUsagePage == 0 || capabilities.UsagePage == aUsagePage))
                            {
                                // Increment the number of matching HID devices
                                (*apCount)++;

                                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Setting DeviceHandle: %d", deviceHandle);
                                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Setting DevicePath: %s", deviceInterfaceDetailData->DevicePath);

                                // Get and save the USB Hub Device Instance, so that it can be used after the device restart,
                                // to get the HID Device Path
                                DEVINST usbHubDevInst;
                                retVal = GetUsbHubDevInst(devInfoData.DevInst, usbHubDevInst, aVid);

                                CHidDevInfo* pDevInfo = new CHidDevInfo(deviceHandle, deviceInterfaceDetailData->DevicePath,
                                    capabilities.FeatureReportByteLength, usbHubDevInst,
                                    aVid, aPid, aUsage, aUsagePage);
                                mHidDevInfo.push_back(pDevInfo);
                            }
                            else
                            {
                                // Opened wrong device (capabilities), close it.
                                CloseHandle(deviceHandle);
                                deviceHandle = INVALID_HANDLE_VALUE;
                            }
                        }
                        else
                        {
                            // Opened wrong device (vid/pid), close it.
                            CloseHandle(deviceHandle);
                            deviceHandle = INVALID_HANDLE_VALUE;
                        }
                    }
                    else
                    {
                        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Failed to access the device");
                    }
                }
                HeapFree(GetProcessHeap(), 0, deviceInterfaceDetailData);
            }
        }
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }

    // If devInfo is not populated, no devices found
    if (mHidDevInfo.size() == 0)
    {
        std::ostringstream msg;
        msg << "No devices found with VID = 0x"
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw(4) << aVid
            << " and PID = 0x"
            << std::setw(4) << aPid;
        retVal = mLastError.SetMsg(HIDDFU_ERROR_CONNECTION, msg.str());
    }

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Found %d matching HID devices for connection", *apCount);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
int32 CHidDfuEngine::CreateHidDeviceObjects()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (mHidDevInfo.size() == 0)
    {
        retVal = mLastError.SetMsg(HIDDFU_ERROR_SEQUENCE, "Invalid sequence");
    }
    else if (mHidDfuDevices.size() == 0)
    {
        for (std::vector <CHidDevInfo *>::const_iterator itHidDevInfo = mHidDevInfo.begin();
                (itHidDevInfo != mHidDevInfo.end() && (retVal == HIDDFU_ERROR_NONE));
                ++itHidDevInfo)
        {
            CHidDfuDevice* pDevice = NULL;
            // Populate mHidDfuDevices with the device information based on device type
            if (typeid(T) == typeid(CHidDfuDeviceApplication))
            {
                // Store USB Hub Device Instance (etc.) with the HidDfuDeviceApplication object, for reconnection
                pDevice = new CHidDfuDeviceApplication((*itHidDevInfo)->mDeviceHandle,
                            (*itHidDevInfo)->mDevPath.c_str(),
                            (*itHidDevInfo)->mUsbHubDevInst,
                            (*itHidDevInfo)->mVid, (*itHidDevInfo)->mPid,
                            (*itHidDevInfo)->mUsage, (*itHidDevInfo)->mUsagePage);
            }
            else if (typeid(T) == typeid(CHidDfuDeviceLoader))
            {
                // Get FeatureReportLength from USB descriptor and store with the HidDfuDeviceLoader object
                pDevice = new CHidDfuDeviceLoader((*itHidDevInfo)->mDeviceHandle, (*itHidDevInfo)->mFeatureReportLength);
            }
            else
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Unknown device type");
            }

            // Initialise and add device to the CHidDfuDevice Vector
            if (retVal == HIDDFU_ERROR_NONE)
            {
                retVal = pDevice->Initialise();
                if (retVal == HIDDFU_ERROR_NONE)
                {
                    mHidDfuDevices.push_back(pDevice);
                }
                else
                {
                    retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN, pDevice->GetLastError());
                }
            }
        }
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "HidDfuDevices already exists, size = %d", mHidDfuDevices.size());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

void CHidDfuEngine::DeleteHidDevInfo()
{
    FUNCTION_DEBUG_SENTRY;

    // Delete mHidDevInfo
    for (std::vector <CHidDevInfo *>::iterator itHidDevInfo = mHidDevInfo.begin();
        (itHidDevInfo != mHidDevInfo.end());
        ++itHidDevInfo)
    {
        delete *itHidDevInfo;
    }
    mHidDevInfo.clear();
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::DisconnectDevice()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // For each HidDfuDevice, check IsActive(), then delete the device
    for(HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
        (itHidDfuDevices != mHidDfuDevices.end()) && (retVal == HIDDFU_ERROR_NONE);
        )
    {
        // Check if the thread is active
        if ((*itHidDfuDevices)->IsThreadActive())
        {
            retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
            ++itHidDfuDevices;
        }
        else
        {
            delete *itHidDfuDevices;
            itHidDfuDevices = mHidDfuDevices.erase(itHidDfuDevices);
        }
    }

    // Delete mHidDevInfo (since the handle in mHidDevInfo would be invalid after mHidDfuDevices is deleted)
    if (retVal == HIDDFU_ERROR_NONE)
    {
        DeleteHidDevInfo();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::GetFirmwareVersion(char* apVersionString, uint16* apMaxLength, bool aCheckMatch)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceApplication>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Check length before writing to the apVersionString buffer. It will have -
        //      uint16 versionMajor, uint16 versionMinor, uint16 configVersion; ...
        // A uint16 will take maximum 5 characters, so for 3 uint16's allocate 15 character
        // and each uint16 number is followed by a comma/semicolon so allocate another 3 for 3 uint16 numbers
        // which makes it a total of 18 character for 1 device.
        const uint16 multiplier = 18;
        // Add an additional byte for NULL character.
        const uint16 MAX_LENGTH = (static_cast<uint16>(mHidDfuDevices.size()) * multiplier) + 1;

        if (*apMaxLength < MAX_LENGTH)
        {
            *apMaxLength = MAX_LENGTH;
            retVal = mLastError.SetMsg(HIDDFU_ERROR_PARAM_TOO_SMALL,
                "hidDfuGetFirmwareVersion: Insufficient memory allocated to version buffer");
        }

        std::stringstream deviceVersions;
        std::string firstVersion;
        bool sameVersion = true;

        for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
            (itHidDfuDevices != mHidDfuDevices.end()) && (retVal == HIDDFU_ERROR_NONE);
            ++itHidDfuDevices)
        {
            uint16 versionMajor = 0;
            uint16 versionMinor = 0;
            uint16 configVersion = 0;

            retVal = (*itHidDfuDevices)->GetDevFirmwareVersion(versionMajor, versionMinor, configVersion);

            if (retVal == HIDDFU_ERROR_NONE)
            {
                std::stringstream version;
                version << versionMajor << "," << versionMinor << "," << configVersion << ";";
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Version: %s", version.str().c_str());

                deviceVersions << version.str();

                if (itHidDfuDevices == mHidDfuDevices.begin())
                {
                    firstVersion = version.str();
                }
                else
                {
                    if (firstVersion != version.str())
                    {
                        sameVersion = false;
                    }
                }
            }
        }

        if (retVal == HIDDFU_ERROR_NONE)
        {
            deviceVersions << '\0';
            strcpy(apVersionString, deviceVersions.str().c_str());

            if ((aCheckMatch == true) && (sameVersion == false))
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_VERSION_MISMATCH,
                    "hidDfuGetFirmwareVersion: Version mismatch on the devices.");
            }
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

uint8 CHidDfuEngine::GetProgress()
{
    uint8 meanProgress = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint8, meanProgress);

    uint32 progress = 0;

    for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
        itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
    {
        int32 devResult = (*itHidDfuDevices)->GetDevResult();
        if ((devResult == HIDDFU_ERROR_NONE) || (devResult == HIDDFU_ERROR_BUSY))
        {
            progress += (*itHidDfuDevices)->GetProgress();
        }
        else
        {
            progress += 100;
        }

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "progress = %d%%", progress);
    }

    if (mHidDfuDevices.size() > 0)
    {
        meanProgress = static_cast<uint8>(progress / mHidDfuDevices.size());
    }

    return meanProgress;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::GetResult()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    int32 deviceResult = HIDDFU_ERROR_NONE;
    std::vector<int32> deviceError;
    std::string errorMsg;

    for(HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
        itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
    {
        deviceResult = (*itHidDfuDevices)->GetDevResult();

        if (deviceResult != HIDDFU_ERROR_NONE)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "deviceResult = %d", deviceResult);
        }

        deviceError.push_back(deviceResult);
        if (errorMsg.empty())
        {
            errorMsg = (*itHidDfuDevices)->GetLastError();
        }
    }

    // If all devices have common error code ?
    if (!deviceError.empty())
    {
        bool allAreEqual =
            std::find_if(deviceError.begin() + 1, 
            deviceError.end(), 
            std::bind1st(std::not_equal_to<int32>(), deviceError.front())) == deviceError.end();

        if (allAreEqual)
        {
            // Return common error code and set the error message (if present)
            if (!errorMsg.empty())
            {
                retVal = mLastError.SetMsg(deviceResult, errorMsg);
            }
            else
            {
                retVal = deviceResult;
            }
        }
        else
        {
            // All devices do not have common error
            if ((std::find(deviceError.begin(), deviceError.end(), HIDDFU_ERROR_NONE) != deviceError.end()))
            {
                // One or more device has no error, and the rest have an error code set
                // then return 'partial success'
                if ((std::find(deviceError.begin(), deviceError.end(), HIDDFU_ERROR_NO_RESPONSE) != deviceError.end()))
                {
                    // If one of the device is not responding
                    retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE);
                }
                else
                {
                    retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS);
                }


            }
            else
            {
                // If all devices have failed but they have different error codes set
                retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Failure on device(s).");
            }
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::GetUsbHubDevInst(DEVINST aChildDevInst, DEVINST & aParentDevInst, uint16 aVid)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    std::ostringstream vid;      //< USB Vendor Identifier
    vid << std::uppercase << "VID_" << std::hex << std::setfill('0') << std::setw(4) << aVid;

    bool matchFound = true;
    DEVINST parentDevInst;

    // Look up for parents in USB Tree with a matching VID
    while (matchFound)
    {
        // Get parent's Device Instance
        CONFIGRET status = CM_Get_Parent(&parentDevInst, aChildDevInst, 0);
        if (status == CR_SUCCESS)
        {
            //Get parent's Device Instance ID
            char *pDevInstID = new char[MAX_DEVICE_ID_LEN];
            status = CM_Get_Device_IDA(parentDevInst, pDevInstID, MAX_PATH, 0);
            if (status == CR_SUCCESS)
            {
                std::string str(pDevInstID, MAX_DEVICE_ID_LEN);
                if (str.find(vid.str()) != std::string::npos)
                {
                    // Use parent's Device Instance for matching VID
                    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Parent Device: %s", str.c_str());
                    aParentDevInst = parentDevInst;
                }
                else
                {
                    // If parent has not got a matching VID stop searching
                    matchFound = false;
                }
            }
            else
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN,
                    "Failed to get Device Instance ID");
            }
            aChildDevInst = parentDevInst;
        }
        else
        {
            retVal = mLastError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Failed to get parent device");
        }
    }
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::StopOperation(uint16 aWaitForStopMs)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
        itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
    {
        retVal = (*itHidDfuDevices)->DeviceStop(aWaitForStopMs);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::ValidateBinFile(const char *apFile)
{
    assert(apFile != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Check the file extension for ".bin"
    std::string fileStr = apFile;
    std::string::size_type pos = fileStr.rfind('.');
    if (pos != std::string::npos)
    {
        std::string extension = fileStr.substr(pos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension.compare("bin") != 0)
        {
            retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File extension is invalid, expected .bin");
        }
    }
    else
    {
        retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File extension is invalid, expected .bin");
    }

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Open the file and get the file handle
        FILE *pFileHandle = fopen(apFile, "rb");
        if (pFileHandle == NULL)
        {
            std::ostringstream msg;
            msg << "Failed to open file \"" << apFile << "\"";
            retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_OPEN_FAILED, msg.str());
        }
        else
        {
            // Ensure file is not empty
            fseek(pFileHandle, 0, SEEK_END);
            unsigned long sizeOfFile = ftell(pFileHandle);

            if (sizeOfFile == 0)
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File format is invalid, no payload found");
            }
            else
            {
                rewind(pFileHandle);
            }
            // Close file and free buffer
            fclose(pFileHandle);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::ValidateDfuFile(const char *apFile)
{
    assert(apFile != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    FILE *pFile = fopen(apFile, "rb");
    if (pFile == NULL)
    {
        std::ostringstream msg;
        msg << "Failed to open file \"" << apFile << "\"";
        retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_OPEN_FAILED, msg.str());
    }

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Get file length
        fseek(pFile, 0, SEEK_END);
        const size_t fileSize = (size_t)ftell(pFile);

        if (fileSize < sizeof(DfuFileSuffix))
        {
            retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File too short");
        }
        else
        {
            // Move to start of suffix
            fseek(pFile, -(long)sizeof(DfuFileSuffix), SEEK_END);

            // Read suffix
            DfuFileSuffix suffixData;
            if (fread(&suffixData, sizeof(uint8), sizeof(DfuFileSuffix), pFile) == 0)
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_READ_FAILED, "Failed to read file suffix");
            }

            // Validate suffix
            if (retVal == HIDDFU_ERROR_NONE &&
                (suffixData.bLength < sizeof(DfuFileSuffix) ||
                fileSize < suffixData.bLength ||
                memcmp(suffixData.ucDfuSignature, STANDARD_DFU_SUFFIX.ucDfuSignature, sizeof(suffixData.ucDfuSignature)) != 0 ||
                suffixData.bcdDfu != STANDARD_DFU_SUFFIX.bcdDfu))
            {
                retVal = mLastError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File suffix is invalid for the connected device");
            }

            if (retVal == HIDDFU_ERROR_NONE)
            {
                // Allocate buffer to read file
                uint8 *pFileBuffer = new uint8[fileSize];
                memset(pFileBuffer, 0, fileSize);

                // Move to start of file
                fseek(pFile, 0, SEEK_SET);

                const size_t fileSizeWithoutCRC = fileSize - sizeof(suffixData.dwCrc); 

                // Read file information into buffer, excluding final CRC (4 bytes)
                fread(pFileBuffer, sizeof(pFileBuffer[0]), fileSizeWithoutCRC, pFile);

                // Calculate file CRC
                CRC crc;
                crc(pFileBuffer, fileSizeWithoutCRC);
                if (crc() != suffixData.dwCrc)
                {
                    retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_FILE_CRC_INCORRECT);
                }

                // Return to start of file
                fseek(pFile, 0, SEEK_SET);

                delete[] pFileBuffer;
            }
        }
    }

    if (pFile != NULL)
    {
        fclose(pFile);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::ResetDevice()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceLoader>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
            itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
        {
            retVal = (*itHidDfuDevices)->ResetDevice(false);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::WriteToDevice(const uint8* apBuffer, uint32 aLength)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceLoader>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
            itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
        {
            retVal = (*itHidDfuDevices)->WriteToDevice(apBuffer, aLength);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::Backup(const char* apFilename, uint8 aResetAfter)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceLoader>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Check parameters and state
        if (apFilename == NULL || strlen(apFilename) == 0)
        {
            retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_INVALID_PARAMETER);
        }
        else
        {
            // Store the filename
            int32 deviceIndex = 0;
            int32 retValForBackup = HIDDFU_ERROR_NONE;
            for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
                itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
            {
                // Check if the thread is active ?
                if ((*itHidDfuDevices)->IsThreadActive())
                {
                    retValForBackup = mLastError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
                }
                else
                {
                    std::string fileName(apFilename, strlen(apFilename));
                    // Append fileNameSuffix to fileName, if there are more than 1 devices
                    if (mHidDfuDevices.size() > 1)
                    {
                        size_t decPosition = fileName.find_last_of(".");
                        fileName.insert(decPosition, "-");
                        fileName.insert(decPosition + 1,
                            static_cast<std::ostringstream*>(&(std::ostringstream() << deviceIndex))->str());
                        deviceIndex++;
                    }

                    (*itHidDfuDevices)->SetFileName(fileName);
                    retValForBackup = (*itHidDfuDevices)->DeviceBackup(aResetAfter); // Start the backup thread
                }

                // Store any intermediate error for the devices being backed-up
                if (retValForBackup != HIDDFU_ERROR_NONE)
                {
                    retVal = retValForBackup;
                }
            }
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::Upgrade(const char* apFilename, uint8 aResetAfter)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceLoader>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Check state and parameters
        if (apFilename == NULL || strlen(apFilename) == 0)
        {
            retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_INVALID_PARAMETER);
        }
        else
        {
            // Store the filename
            retVal = ValidateDfuFile(apFilename);

            if (retVal == HIDDFU_ERROR_NONE)
            {
                int32 retValForUpgrade = HIDDFU_ERROR_NONE;
                if (mHidDfuDevices.size() == 0)
                {
                    retVal = HIDDFU_ERROR_SEQUENCE;
                }

                for(HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
                    itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
                {
                    // Check if the thread is active ?
                    if ((*itHidDfuDevices)->IsThreadActive())
                    {
                        retValForUpgrade = mLastError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
                    }
                    else
                    {
                        (*itHidDfuDevices)->SetFileName(apFilename);
                        retValForUpgrade = (*itHidDfuDevices)->DeviceUpgrade(aResetAfter); // Start the upgrade thread
                    }

                    // Store any intermediate error for the devices being upgraded
                    if (retValForUpgrade != HIDDFU_ERROR_NONE)
                    {
                        retVal = retValForUpgrade;
                    }
                }
            }
        }
    }
    return retVal;
}


////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuEngine::UpgradeBin(const char* apFilename)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = CreateHidDeviceObjects<CHidDfuDeviceApplication>();

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Check state and parameters
        if (apFilename == NULL || strlen(apFilename) == 0)
        {
            retVal = mLastError.SetDefaultMsg(HIDDFU_ERROR_INVALID_PARAMETER);
        }
        else
        {
            // File validation
            retVal = ValidateBinFile(apFilename);

            if (retVal == HIDDFU_ERROR_NONE)
            {
                int32 retValForUpgrade = HIDDFU_ERROR_NONE;
                for (HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
                    itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
                {
                    // Check if the thread is active ?
                    if ((*itHidDfuDevices)->IsThreadActive())
                    {
                        retValForUpgrade = mLastError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
                    }
                    else
                    {
                        (*itHidDfuDevices)->SetFileName(apFilename);
                        retValForUpgrade = (*itHidDfuDevices)->DeviceUpgrade(false); // Start the upgrade thread
                    }

                    // Store any intermediate error for the devices being upgraded
                    if (retValForUpgrade != HIDDFU_ERROR_NONE)
                    {
                        retVal = retValForUpgrade;
                    }
                }
            }
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

const char* CHidDfuEngine::GetLastError()
{
    FUNCTION_DEBUG_SENTRY;
    return mLastError.Get().c_str();
}

////////////////////////////////////////////////////////////////////////////////

uint8 CHidDfuEngine::GetFailedDevicesCount(void)
{
    uint8 failedDevices = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint8, failedDevices);

    for(HidDfuDeviceVctrItr itHidDfuDevices = mHidDfuDevices.begin();
        itHidDfuDevices != mHidDfuDevices.end(); ++itHidDfuDevices)
    {
        int32 deviceResult = (*itHidDfuDevices)->GetDevResult();
        if ((deviceResult != HIDDFU_ERROR_NONE) && (deviceResult != HIDDFU_ERROR_BUSY))
        {
            failedDevices++;
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Error: %s", (*itHidDfuDevices)->GetLastError());
        }
    }

    return failedDevices;
}

////////////////////////////////////////////////////////////////////////////////