//*******************************************************************************
//
//  HidDfuDevice.h
//
//  Copyright (c) 2014-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDevice class, an instance of the class represents
//  a single HidDfu device with a matching VID and PID.
//
//*******************************************************************************

#ifndef HID_DFU_DEVICE_H
#define HID_DFU_DEVICE_H

#include "HidDfuErrorMsg.h"
#include "common/types.h"
#include "thread/thread.h"
#include "thread/critical_section.h"
#include "engine/enginefw_interface.h"

#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_HID_DFU_LIB

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <string>

///
/// Class API for HidDfu Device functions.
///
class CHidDfuDevice : public Threadable
{
public:
    CHidDfuDevice();
    CHidDfuDevice(HANDLE aDeviceHandle);
    ~CHidDfuDevice();

    ///
    /// Returns whether a device is connected or not.
    /// @return true if a device is connected, false otherwise.
    ///
    bool Connected() const;

    ///
    /// Upgrade the device, conditional reset
    /// @param[in] aResetAfter Reset after upgrade if set to true.
    ///
    int32 DeviceUpgrade(uint8 aResetAfter);

    ///
    /// Backup the device, conditional reset
    /// @param[in] aResetAfter Reset after backup if set to true.
    ///
    int32 DeviceBackup(uint8 aResetAfter);

    ///
    /// Backup the device (method to be threaded)
    /// @return The result.
    ///
    virtual int32 DoDeviceBackup() = 0;

    ///
    /// Stop the ongoing operation
    /// @param[in] aWaitForStopMs Wait time (in milliseconds) for operation to stop.
    /// @return The result.
    ///
    int32 DeviceStop(uint16 aWaitForStopMs);

    ///
    /// Upgrade the device (method to be threaded)
    /// @return The result.
    ///
    virtual int32 DoDeviceUpgrade() = 0;

    ///
    /// Gets the result of the last operation non-blocking operation.
    /// An error will be returned if an operation is ongoing, or if 
    /// an operation has not been started.
    /// @return The result.
    /// @see hidDfuGetResult.
    ///
    int32 GetDevResult();

    ///
    /// Get the firmware version from the device
    /// @param[out] aVersionMajor Version Major
    /// @param[out] aVersionMinor Version Minor
    /// @param[out] aConfigVersion Config Version
    /// @return The result.
    ///
    virtual int32 GetDevFirmwareVersion(
        uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion) = 0;

    ///
    /// Get the Last Error description for this device.
    /// @return Error string.
    ///
    const char* GetLastError();

    ///
    /// Get the progress of current operation for this device.
    /// @return Progress in percentage.
    ///
    uint8 GetProgress();

    ///
    /// Initialise.
    /// @return The result.
    ///
    virtual int32 Initialise() = 0;

    ///
    /// Check if the thread for upgrade/backup is running.
    /// @return true if the thread is active, false otherwise.
    ///
    int32 IsThreadActive();

    ///
    /// Send command to DFU mode BlueCore to carry out a reset, resulting 
    /// in the device exiting DFU mode.
    /// @param[in] aFromOperationThread Indicates whether the call is from
    /// an operation thread or another source. If set to false, the function
    /// will fail if an operation thread is active, therefore it must be set
    /// to true if called from an operation thread.
    /// @return The result.
    ///
    virtual int32 ResetDevice(bool aFromOperationThread) = 0;

    ///
    /// Store the File Name for upgrade/backup.
    /// @param[in] fileName File Name.
    ///
    void SetFileName(std::string fileName) { mFileName = fileName; }

    ///
    /// Writes to the connected device.
    /// @param[in] apBuffer Write buffer.
    /// @param[in] aLength Length of the write buffer in bytes.
    /// @return The result.
    ///
    virtual int32 WriteToDevice(const uint8* apBuffer, uint32 aLength) = 0;

private:

    /// Operation progress (for non-blocking operations)
    uint8 mProgress;

    /// Function pointer typedef for non-blocking operations
    typedef int32(CHidDfuDevice::* THREAD_FUNC)();

    /// Function pointer for non-blocking operations
    THREAD_FUNC mpThreadFunc;

    ///
    /// Runs non-blocking operations
    /// @return The result
    /// @see Threadable
    ///
    virtual int ThreadFunc();

protected:

    /// Device Handle
    HANDLE mDeviceHandle;

    /// DFU filename
    std::string mFileName;

    /// Last error message for this device
    CHidDfuErrorMsg mLastDevError;

    /// Thread lock for progress get & set
    CriticalSection mProgressLock;

    /// Flag for  - Reset after backup/upgrade operation
    uint8 mResetAfter;

    ///
    /// Gets the thread state.
    /// If the operation is threaded check if thread is running, else return true(default).
    /// @return The result.
    ///
    bool CheckKeepGoing();

    ///
    /// Close the device handle
    /// @return The result.
    ///
    int32 DisconnectDevice();

    ///
    /// Sets the thread function pointer to NULL, a derived class operation which does not use thread
    /// can then reset the thread function pointer to NULL.
    ///
    void ResetThreadFuncPointer();

    ///
    /// Get details of OS error message.
    /// @param[in] aHidErrorCode HidDfu application error code.
    /// @param[in] Caller details to add details to OS Error String
    /// @param[in] Error Code as received from OS
    /// @return The result.
    ///
    int32 SetLastErrorFromWinError(int32 aHidErrorCode, const std::string &aErrorStr, uint32 aWinErrorCode);

    ///
    /// Sets the progress value for the current non-blocking operation
    /// @param[in] aProgress The progress value (percentage). If the value 
    /// given is greater than 100, the value will be set to 100.
    ///
    void SetProgress(uint8 aProgress);
};

#endif // #ifndef HID_DFU_DEVICE_H