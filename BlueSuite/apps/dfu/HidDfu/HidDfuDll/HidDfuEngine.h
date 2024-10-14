//*******************************************************************************
//
//  HidDfuEngine.h
//
//  Copyright (c) 2012-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Internal functions implementing Device Firmware Update using HID driver.
//
//*******************************************************************************

#ifndef HID_DFU_ENGINE_H
#define HID_DFU_ENGINE_H

#include "common/types.h"
#include "HidDfuErrorMsg.h"
#include "HidDfuDevice.h"

#include <string>
#include <vector>
#include <cfgmgr32.h>

///
/// Class API for HidDfu functions.
/// The public functions are wrapped in a C API for the DLL interface.
///
class CHidDfuEngine
{
public:
    CHidDfuEngine();
    ~CHidDfuEngine();

    ///
    /// Reads image from connected devices and saves to the given file name.
    /// If there are multiple devices, files are suffixed with a "-" and a number
    /// (based on the order in which the devices were enumerated
    /// by the system).
    /// This operation is non-blocking, therefore a success result only indicates
    /// that the operation was started successfully. GetOpProgress should be called 
    /// to wait for completion, and GetResult called to get the result of the operation.
    /// An error is returned if an operation is already in progress.
    /// @param[in] apfilename File name.
    /// @param[in] aResetAfter Reset the device after backup.
    /// @return The result of starting the backup operation.
    /// @see hidDfuBackup.
    ///
    int32 Backup(const char* apfilename, uint8 aResetAfter);

    ///
    /// Looks for devices matching the given parameters and if found,
    /// attempts to open connections to the devices.
    /// @param[in] aVid Vendor ID.
    /// @param[in] aPid Product ID.
    /// @param[in] aUsage USB usage value. If the value is zero, the 
    ///            device usage parameter is not checked for a match.
    /// @param[in] aUsagePage USB usage page value. If the value is 
    ///            zero, the device usagePage parameter is not checked
    ///            for a match.
    /// @param[out] apCount The number of devices found.
    /// @return The result.
    /// @see hidDfuConnect.
    ///
    int32 ConnectDevice(uint16 aVid, uint16 aPid, uint16 aUsage, uint16 aUsagePage, uint16* apCount);

    ///
    /// Disconnects from connected devices.
    /// An error is returned if an operation is in progress.
    /// @return The result.
    /// @see hidDfuDisconnect.
    ///
    int32 DisconnectDevice();

    ///
    /// Gets the count of devices which failed for the last
    /// upgrade or backup operation.
    /// @return The count.
    /// @see hidDfuGetFailedDevicesCount.
    ///
    uint8 GetFailedDevicesCount();

    ///
    /// Gets the version information of the connected devices.
    /// @param[out] apVersionString Pointer to a buffer where the comma/semicolon separated string representing
    ///                             device version information for all the connected devices will be
    ///                             written. The format of the string is :
    ///                             "dev1_ver_major,dev1_ver_minor,dev1_config_ver;dev1_ver_major,..."
    /// @param[in, out] apMaxLength Length of the apVersionString buffer, 
    ///                             returns expected length if the length given is less than is required to
    ///                             store the version string.
    /// @param[in] aCheckMatch Check if all the devices have same version
    /// @return The result.
    ///
    int32 GetFirmwareVersion(char* apVersionString, uint16* apMaxLength, bool aCheckMatch);

    ///
    /// Gets the description of the last error.
    /// @return The last error description.
    /// @see hidDfuGetLastError.
    ///
    const char* GetLastError();

    ///
    /// Gets the progress of the current / last operation.
    /// If there are multiple devices, calculate mean percentage
    /// for all devices
    /// @return The progress value (percentage).
    /// @see hidDfuGetProgress.
    ///
    uint8 GetProgress();

    ///
    /// Gets the result of the last operation from the devices.
    /// An error will be returned if an operation is ongoing, or if 
    /// an operation has not been started.
    /// If all devices have common error same would be returned, if they
    /// have different errors then HIDDFU_ERROR_UNKNOWN will be returned.
    /// @return The result.
    /// @see hidDfuGetResult.
    ///
    int32 GetResult();

    ///
    /// Send command to DFU mode BlueCore to carry out a reset, resulting 
    /// in the devices exiting DFU mode.
    /// @return The result.
    ///
    int32 ResetDevice();

    ///
    /// Stop the upgrade, backup or upgrade binary operation.
    /// @param[in] aWaitForStopMs Wait time (in milliseconds) for operation to stop.
    /// @return The result.
    ///
    int32 StopOperation(uint16 aWaitForStopMs);

    ///
    /// Reads DFU image from file and upgrades the connected devices.
    /// This operation is non-blocking, therefore a success result only indicates
    /// that the operation was started successfully. GetOpProgress should be called 
    /// to wait for completion, and GetResult called to get the result of the operation.
    /// An error is returned if an operation is already in progress.
    /// @param[in] apfilename File name.
    /// @param[in] aResetAfter Reset the devices after upgrade.
    /// @return The result of starting the upgrade operation.
    /// @see hidDfuUpgrade.
    ///
    int32 Upgrade(const char* apfilename, uint8 aResetAfter);

    ///
    /// Reads DFU binary image from file and upgrades the connected devices.
    /// This operation is non-blocking, therefore a success result only indicates
    /// that the operation was started successfully. GetResult should be called 
    /// to get the result of the operation.
    /// An error is returned if an operation is already in progress.
    /// @param[in] apfilename File name.
    /// @return The result of starting the upgrade operation.
    /// @see hidDfuUpgradeBin.
    ///
    int32 UpgradeBin(const char* apfilename);

    ///
    /// Writes to the connected device.
    /// @param[in] apBuffer Write buffer.
    /// @param[in] aLength Length of the write buffer in bytes.
    /// @return The result.
    ///
    int32 WriteToDevice(const uint8* apBuffer, uint32 aLength);

private:

    ///
    /// Contains detail of the matching HID device for creating a HID device object later on
    ///
    class CHidDevInfo
    {
    public:
        HANDLE mDeviceHandle;
        std::string mDevPath;
        USHORT mFeatureReportLength;
        DEVINST mUsbHubDevInst;     //< Device Instance of the top-level device with matchig VID (in USB Tree)
        uint16 mVid;                //< Vendor ID
        uint16 mPid;                //< Product ID
        uint16 mUsage;              //< USB usage value
        uint16 mUsagePage;          //< USB usage page value

        CHidDevInfo(HANDLE aDeviceHandle, const std::string &aDevPath,
            USHORT aFeatureReportLength, DEVINST aUsbHubDevInst,
            uint16 aVid, uint16 aPid, uint16 aUsage, uint16 aUsagePage)
            : mDeviceHandle(aDeviceHandle), mDevPath(aDevPath),
              mFeatureReportLength(aFeatureReportLength),
              mUsbHubDevInst(aUsbHubDevInst),
              mVid(aVid), mPid(aPid), mUsage(aUsage), mUsagePage(aUsagePage) {};
    };

    typedef std::vector <CHidDevInfo *> HidDevInfoVctr;
    HidDevInfoVctr mHidDevInfo;

    /// typedef for CHidDfuDevice Vector and iterator
    typedef std::vector <CHidDfuDevice *> HidDfuDeviceVctr;
    typedef std::vector <CHidDfuDevice *>::iterator HidDfuDeviceVctrItr;

    /// CHidDfuDevice Vector, holds information of matching HidDfu devices
    HidDfuDeviceVctr mHidDfuDevices;

    // File suffix structure.
    // The DFU suffix is defined as offsets from the end of the file, which
    // is why the fields are listed in the opposite of expected order and
    // why the "DFU" string for ucDfuSignature is reversed
    // 
    // uint16 bcdDevice         0x0001      (not used) This can be used to hold the stack software build integer
    // uint16 idProduct         0xFFFE      USB Product ID
    // uint16 idVendor          0x0A12      USB Vendor ID
    // uint16 bcdDfu            0x0100      DFU specification number (1.0)
    // uint8 ucDfuSignature[3]  "UFD"       DFU Signature string
    // uint8 bLength            0x10        DFU Suffix length
    // uint32 dwCRC                         Added in code, not a constant
    struct DfuFileSuffix
    {
        uint16  bcdDevice;
        uint16  idProduct;
        uint16  idVendor;
        uint16  bcdDfu;
        uint8   ucDfuSignature[3];
        uint8   bLength;
        uint32  dwCrc;
    };

    /// Standard DFU file suffix data (that expected from a DFU file)
    static const DfuFileSuffix STANDARD_DFU_SUFFIX;

    /// Length of file suffix without CRC (because the CRC calculation doesn't include itself)
    static const size_t DFU_SUFFIX_WITHOUT_CRC_LEN = sizeof(DfuFileSuffix) - sizeof(uint32);

    /// Last error message
    CHidDfuErrorMsg mLastError;

    /// Operation progress (average for all HidDfu devices)
    uint8 mProgress;

    ///
    /// Queries if all the matching HidDfu devices are connected
    /// @return true if all devices are connected, false otherwise.
    ///
    bool Connected();

    ///
    /// Create HID Device objects - Loader or application based on typename T
    /// @return The result.
    ///
    template<typename T> int32 CreateHidDeviceObjects();

    ///
    /// Delete HID Info Objects
    ///
    void DeleteHidDevInfo();

    ///
    /// Gets the Device Instance of top level parent device (USB Hub in USB tree)
    /// which has same VID as the child device instance.
    /// @param[in] aChildDevInst Device Instance of the HID device.
    /// @param[out] aParentDevInst Device Instance of USB Hub for the device.
    /// @param[in] aVid Vendor Identifier
    /// @return The result.
    ///
    int32 GetUsbHubDevInst(DEVINST aChildDevInst, DEVINST &aParentDevInst, uint16 aVid);

    ///
    /// Validates a DFU binary file.
    /// @param[in] apFile File name.
    /// @return The result.
    ///
    int32 ValidateBinFile(const char *apFile);

    ///
    /// Validates a DFU file by checking its suffix data and CRC.
    /// @param[in] apFile File name.
    /// @return The result.
    ///
    int32 ValidateDfuFile(const char *apFile);
};

#endif // #ifndef HID_DFU_ENGINE_H