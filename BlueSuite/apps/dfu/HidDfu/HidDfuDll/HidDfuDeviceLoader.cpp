//*******************************************************************************
//
//  HidDfuDeviceLoader.cpp
//
//  Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDeviceLoader class.
//
//*******************************************************************************

#include "HidDfu.h"
#include "HidDfuDeviceLoader.h"
#include "DFUEngine/CRC.h"
#include "time/hi_res_clock.h"
#include "time/stop_watch.h"

#include <sstream>
#include <assert.h>

extern "C"
{
#include <hidsdi.h>
}

////////////////////////////////////////////////////////////////////////////////

const CHidDfuDeviceLoader::DfuFileSuffix CHidDfuDeviceLoader::STANDARD_DFU_SUFFIX = 
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

CHidDfuDeviceLoader::CHidDfuDeviceLoader()
    : CHidDfuDevice(INVALID_HANDLE_VALUE),
    mFeatureReportLength(0),
    mBwPollTimeout(0)
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDeviceLoader::CHidDfuDeviceLoader(HANDLE aDeviceHandle, USHORT aFeatureReportLength)
    : CHidDfuDevice(aDeviceHandle),
    mFeatureReportLength(aFeatureReportLength),
    mBwPollTimeout(0)
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDeviceLoader::~CHidDfuDeviceLoader()
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

void CHidDfuDeviceLoader::AddWriteHeader(uint8 *apBuffer, uint16 aWValue, uint16 aWLength)
{
    FUNCTION_DEBUG_SENTRY;

    assert(apBuffer != NULL);

    // Add DFU write header information to buffer.
    // Header information has the following format:
    // ReportID             1 byte
    // bRequest             1 byte
    // wValue               2 bytes             holds block number for DFU download
    // wLength              2 bytes             total amount of bytes to be written
    apBuffer[0] = DFU_HID_REPORTID_UPGRADE;
    apBuffer[1] = DFU_B_REQUEST;
    apBuffer[2] = LOBYTE(aWValue);
    apBuffer[3] = HIBYTE(aWValue);
    apBuffer[4] = LOBYTE(aWLength);
    apBuffer[5] = HIBYTE(aWLength);
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::CheckUpgradeComplete(bool aWaitForFullTimeout)
{
    int32 retVal = HIDDFU_ERROR_BUSY;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Start timer - used to check that mBwPollTimeout hasn't been exceeded
    // while waiting for the status response.
    StopWatch pollTimer;

    // If the status is returned successfully, and the device is busy, 
    // one more attempt is allowed (after waiting for the full mBwPollTimeout period).
    static const uint32 MAX_BUSY_STATUS_ATTEMPTS = 2;

    // If we're waiting for the full poll timeout, we only get the status once
    const uint32 attempts = (aWaitForFullTimeout ? 1 : MAX_BUSY_STATUS_ATTEMPTS);

    if (aWaitForFullTimeout)
    {
        // Wait for full poll timeout before status fetch
        HiResClockSleepMilliSec(mBwPollTimeout);
    }

    // Get the current status from the device
    for(uint32 i = 0; i < attempts && retVal == HIDDFU_ERROR_BUSY; ++i)
    {
        // GetStatus can fail because of a timeout in the HID interface, because 
        // the chip may not respond while it is busy writing to flash. If the failure 
        // is for that reason, retry so long as mBwPollTimeout has not been exceeded.
        uint8 status, state;
        do
        {
            retVal = GetStatus(&status, &state);
        }
        while(retVal == HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE && pollTimer.duration() < mBwPollTimeout && 
            !aWaitForFullTimeout);

        if (retVal == HIDDFU_ERROR_NONE)
        {
            // Check if busy or idle (finished)
            if (status == DFU_STATUS_OK && state == DFU_STATE_DFU_DNBUSY && i < (attempts - 1))
            {
                // Device busy, sleep for the remainder of bwPollTimeout then retry
                const uint32 duration = pollTimer.duration();
                if (duration < mBwPollTimeout)
                {
                    HiResClockSleepMilliSec(mBwPollTimeout - duration);
                }

                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
            }
            else if(status == DFU_STATUS_ERR_FIRMWARE)
            {
                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_DEVICE_FIRMWARE);
            }
            else if((status != DFU_STATUS_OK || 
                (state != DFU_STATE_APP_IDLE && state != DFU_STATE_DFU_IDLE && state != DFU_STATE_DFU_DNLOAD_IDLE)))
            {
                // If status is not DFU_STATUS_OK or the state isn't Idle, the upgrade has failed
                std::ostringstream msg;
                msg << "Upgrade failed (DFU status code = " << static_cast<uint16>(status) << ")";
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED, msg.str());
            }
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::ClearDeviceStatus()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    uint8 buffer[DFU_HID_REPORTID_STATE_SIZE];
    buffer[0] = DFU_HID_REPORTID_STATE;
    buffer[1] = DFU_CMD_CLRSTATUS;

    if(!HidD_SetFeature(mDeviceHandle, buffer, DFU_HID_REPORTID_STATE_SIZE))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_CLEAR_STATUS_FAILED, "Failed to clear device status", ::GetLastError());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::DoDeviceBackup()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (!Connected())
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if (mFeatureReportLength == 0)
    {
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, "Feature report length is invalid (0)");
    }
    else
    {
        mBwPollTimeout = 0;

        uint8 *pBuffer = new uint8[mFeatureReportLength];
        memset(pBuffer, 0, mFeatureReportLength);

        uint32 dataLeftToWrite = 0;
        uint32 dataLength = 0;
        uint16 replyLength;

        // Open output file
        FILE *pFile = fopen(mFileName.c_str(), "wb");
        if (pFile == NULL)
        {
            std::ostringstream msg;
            msg << "Failed to open file \"" << mFileName << "\". File must be writable.";
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_FILE_OPEN_FAILED, msg.str());
        }
        else
        {
            // Read first chunk of DFU file, this contains the file length
            retVal = HidReportIdBackup(pBuffer, mFeatureReportLength, &replyLength);
            if (retVal == HIDDFU_ERROR_NONE)
            {
                // Get DFU file length from first block
                dataLength = GetDfuFileLength(pBuffer);
                dataLeftToWrite = dataLength;
                if (dataLength == 0)
                {
                    retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Device reports DFU length of zero");
                }
            }
        }

        CRC crc;

        if (retVal == HIDDFU_ERROR_NONE)
        {
            // Update CRC
            crc(pBuffer, replyLength);

            // Write first chunk of data to output file
            if (fwrite(pBuffer, sizeof(pBuffer[0]), replyLength, pFile) != replyLength)
            {
                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_WRITE_FAILED);
            }
            else
            {
                // Decrement the data left to write to the output file 
                dataLeftToWrite -= replyLength;
            }
        }

        // Read data from device and copy into output file
        while ((retVal == HIDDFU_ERROR_NONE && dataLeftToWrite > 0) && KeepGoing())
        {
            // Update progress - this should never get to 100%
            SetProgress(static_cast<uint8>(100 - ((static_cast<float64>(dataLeftToWrite) / dataLength) * 100)));

            retVal = HidReportIdBackup(pBuffer, mFeatureReportLength, &replyLength);
            if (retVal == HIDDFU_ERROR_NONE)
            {
                // Update CRC
                crc(pBuffer, replyLength);

                if (fwrite(pBuffer, sizeof(pBuffer[0]), replyLength, pFile) != replyLength)
                {
                    retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_WRITE_FAILED);
                }
                else
                {
                    dataLeftToWrite -= replyLength;
                }
            }
        }

        if ((retVal == HIDDFU_ERROR_NONE) && KeepGoing())
        {
            // Update CRC with DFU file suffix data
            DfuFileSuffix dfuSuffix = STANDARD_DFU_SUFFIX;
            crc(&dfuSuffix, DFU_SUFFIX_WITHOUT_CRC_LEN);
            dfuSuffix.dwCrc = crc();

            // Write DFU suffix data to file
            if (fwrite(&dfuSuffix, sizeof(uint8), sizeof(DfuFileSuffix), pFile) != sizeof(DfuFileSuffix))
            {
                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_WRITE_FAILED);
            }
        }

        // Close file and free buffer
        if (pFile != NULL)
        {
            fclose(pFile);
        }
        delete[] pBuffer;

        // Reset the device
        if ((retVal == HIDDFU_ERROR_NONE) && (mResetAfter) && KeepGoing())
        {
            retVal = ResetDevice(true);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::DoDeviceUpgrade()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    mBwPollTimeout = 0;

    FILE *pFile = fopen(mFileName.c_str(), "rb");
    if (pFile == NULL)
    {
        std::ostringstream msg;
        msg << "Failed to open file \"" << mFileName << "\"";
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_FILE_OPEN_FAILED, msg.str());
    }

    if (!Connected() && (retVal == HIDDFU_ERROR_NONE))
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if (retVal == HIDDFU_ERROR_NONE)
    {
        const int32 payloadLength = mFeatureReportLength - DFU_HID_FROM_HOST_HEADER_SIZE;

        if (payloadLength <= 0)
        {
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File format is invalid, no payload found");
        }
        else
        {
            // Allocate memory buffer to read data from file
            uint8 *pBuffer = new uint8[mFeatureReportLength];
            memset(pBuffer, 0, mFeatureReportLength);

            // Read first block of data from file leaving space at start of buffer for header
            fread(pBuffer + DFU_HID_FROM_HOST_HEADER_SIZE, sizeof(pBuffer[0]), payloadLength, pFile);

            int64 fileDataLength = 0;
            int64 dataLeftToWrite = 0;

            if (ferror(pFile))
            {
                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_READ_FAILED);
            }
            else
            {
                // Get file length from buffer.
                // File length does not include DFU suffix which is not written to device
                fileDataLength = (int64)GetDfuFileLength(pBuffer + DFU_HID_FROM_HOST_HEADER_SIZE);
                dataLeftToWrite = fileDataLength;

                // Check status - should be in same state as when complete
                retVal = CheckUpgradeComplete();
                if (retVal == HIDDFU_ERROR_DEVICE_FIRMWARE)
                {
                    // Device has been left in a bad state, perhaps a previous DFU 
                    // attempt was interrupted part way through. Clear the device 
                    // status first, then continue as normal.
                    retVal = ClearDeviceStatus();
                    if (retVal == HIDDFU_ERROR_NONE)
                    {
                        // Check the status is now ok
                        retVal = CheckUpgradeComplete();
                    }
                }
            }

            uint16 blockNumber = 0;
            if (retVal == HIDDFU_ERROR_NONE)
            {
                // Carry out hid_SetFeature to write block to device
                retVal = HidSetFeatureWithHeader(payloadLength, pBuffer, &dataLeftToWrite, &blockNumber);
            }

            // Loop to read data from file and send to device
            while ((retVal == HIDDFU_ERROR_NONE && dataLeftToWrite > 0) && KeepGoing())
            {
                // Check status
                retVal = CheckUpgradeComplete();
                if (retVal == HIDDFU_ERROR_NONE)
                {
                    // Update progress - this should never get to 100%
                    SetProgress(static_cast<uint8>(100 - ((static_cast<float64>(dataLeftToWrite) / fileDataLength) * 100)));

                    // Read first block of data from file leaving space at start of buffer for header
                    memset(pBuffer, 0, mFeatureReportLength);
                    fread(pBuffer + DFU_HID_FROM_HOST_HEADER_SIZE, sizeof(pBuffer[0]), payloadLength, pFile);

                    // Check for file read error
                    if (ferror(pFile))
                    {
                        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_READ_FAILED);
                    }
                    else
                    {
                        // Carry out hid_SetFeature to write block to device
                        retVal = HidSetFeatureWithHeader(payloadLength, pBuffer, &dataLeftToWrite, &blockNumber);
                    }
                }
            }

            // Check status and complete the upgrade
            if ((retVal == HIDDFU_ERROR_NONE) && KeepGoing())
            {
                retVal = CheckUpgradeComplete();

                if (retVal == HIDDFU_ERROR_NONE)
                {
                    // Send a download request with Length field set to zero.
                    // This is covered in the USB Device Firmware Upgrade Specification
                    retVal = EndUpgrade(pBuffer, blockNumber);
                }
            }

            delete[] pBuffer;
        }
    }

    if (pFile != NULL)
    {
        fclose(pFile);
    }

    // Reset the device
    if ((retVal == HIDDFU_ERROR_NONE) && (mResetAfter) && KeepGoing())
    {
        retVal = ResetDevice(true);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::EndUpgrade(uint8 *apBuffer, uint16 aBlockNumber)
{
    assert(apBuffer != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    memset(apBuffer, 0, mFeatureReportLength);

    AddWriteHeader(apBuffer, aBlockNumber, 0);

    if(!HidD_SetFeature(mDeviceHandle, apBuffer, mFeatureReportLength))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
                "Write to device failed attempting to complete upgrade", ::GetLastError());
    }
    else
    {
        retVal = CheckUpgradeComplete(true);
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////

uint32 CHidDfuDeviceLoader::GetDfuFileLength(const uint8 *apBuffer) const
{
    assert(apBuffer != NULL);

    // DFU file has the following format
    // char     file_id[8]
    // uint16   file_version
    // uint32   file_len
    // uint16   file_hdr_len
    // char     file_desc[64]

    return (apBuffer[10] + 
        apBuffer[11] * DFU_HID_BYTE_MULTIPLIER + 
        apBuffer[12] * DFU_HID_WORD_MULTIPLIER + 
        apBuffer[13] * DFU_HID_BYTE_MULTIPLIER * DFU_HID_WORD_MULTIPLIER); 
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::GetDevFirmwareVersion(
    uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion)
{
    int32 retVal;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Not supported
    retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_UNSUPPORTED);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::GetStatus(uint8* apStatus, uint8* apState)
{
    assert(apStatus != NULL);
    assert(apState != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    uint8 *pBuffer = new uint8[DFU_HID_REPORTID_STATUS_SIZE];
    memset(pBuffer, 0, DFU_HID_REPORTID_STATUS_SIZE);

    // Initialise buffer
    pBuffer[0] = DFU_HID_REPORTID_STATUS;

    // Read status from device
    if(!HidD_GetFeature(mDeviceHandle, pBuffer, DFU_HID_REPORTID_STATUS_SIZE))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
                "Failed to get feature information (GetStatus)", ::GetLastError());
    }
    else
    {
        // Read status, state, and bwPollTimeout fields from reply
        *apStatus = pBuffer[1];
        *apState = pBuffer[5];
        mBwPollTimeout = (pBuffer[2] +
            pBuffer[3] * DFU_HID_BYTE_MULTIPLIER +
            pBuffer[4] * DFU_HID_WORD_MULTIPLIER);
    }

    delete[] pBuffer;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::HidReportIdBackup(uint8* apBuffer, uint16 aBufferLength, uint16 *apReplyLength)
{
    assert(apBuffer != NULL);
    assert(aBufferLength > 0);
    assert(apReplyLength != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (aBufferLength > 0)
    {
        uint8 *pFeatureBuffer = new uint8[aBufferLength];
        memset(pFeatureBuffer, 0, aBufferLength);
        pFeatureBuffer[0] = DFU_HID_REPORTID_UPGRADE;

        if (HidD_GetFeature(mDeviceHandle, pFeatureBuffer, aBufferLength)) 
        {
            // The format of the response is:
            // * 1 byte  - Report ID
            // 2 bytes - number of valid bytes returned
            // 3 bytes - header padding
            // x bytes - data
            //

            *apReplyLength = pFeatureBuffer[1] + DFU_HID_BYTE_MULTIPLIER * pFeatureBuffer[2];

            if (*apReplyLength <= aBufferLength) 
            {
                memcpy(apBuffer, pFeatureBuffer + DFU_HID_TO_HOST_HEADER_SIZE, *apReplyLength);
            }
        }
        else
        {
            // Since HidD_GetFeature is the first HID API call for backup operation, and fails if the device type is not BlueCore
            // and there is no way to detect device type, so display a generic message (by rewriting), and specific details only in the log.
            SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
                "Failed to get feature information (HidReportIdBackup)", ::GetLastError());
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, "Failed to run backup operation on the connected device.");
        }

        delete[] pFeatureBuffer;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::HidSetFeatureWithHeader(int32 aPayloadLength, uint8* apBuffer, 
                                             int64* apDataLeftToWrite, 
                                             uint16* apBlockNumber)
{
    assert(apBuffer != NULL);
    assert(apDataLeftToWrite != NULL);
    assert(apBlockNumber != NULL);

    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Check to see if amount read from file is shorter than gFeatureLengthReport - DFU_HID_FROM_HOST_HEADER_SIZE
    if ((int32)*apDataLeftToWrite < aPayloadLength)
    {
        aPayloadLength = (int32)*apDataLeftToWrite; 
    }

    // Add header for write operation
    AddWriteHeader(apBuffer, *apBlockNumber, (uint16)aPayloadLength);

    if(!HidD_SetFeature(mDeviceHandle, apBuffer, mFeatureReportLength))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
                "Failed to write to device", ::GetLastError());
    }
    else
    {
        *apDataLeftToWrite -= aPayloadLength;
        *apBlockNumber += 1;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::Initialise()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Add any initialization code here, if needed

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::ResetDevice(const bool aFromOperationThread)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Check state
    if (!Connected())
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if ((IsActive()) && (aFromOperationThread == false))
    {
        // An operation is currently running, don't want to reset while that's happening
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
    }
    else
    {
        uint8 *pBuffer = new uint8[DFU_HID_REPORTID_STATE_SIZE];
        pBuffer[0] = DFU_HID_REPORTID_STATE;
        pBuffer[1] = DFU_CMD_RESET;

        if(!HidD_SetFeature(mDeviceHandle, pBuffer, DFU_HID_REPORTID_STATE_SIZE))
        {
            retVal = SetLastErrorFromWinError(HIDDFU_ERROR_RESET_FAILED, "Device reset failed", ::GetLastError());
        }
        else
        {
            // Reset causes device to reboot normally, i.e. not in DFU mode,
            // so it's safest to disconnect after reset to avoid problems if 
            // DFU operations are attempted after reset.
            retVal = DisconnectDevice();
        }

        delete[] pBuffer;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceLoader::WriteToDevice(const uint8* apBuffer, uint32 aLength)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Check state
    if (!Connected())
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if (IsActive())
    {
        // An operation is currently running, don't want to allow this while that's happening
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
    }
    else if(!HidD_SetFeature(mDeviceHandle, (PVOID)apBuffer, aLength))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, "Write to device failed", ::GetLastError());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
