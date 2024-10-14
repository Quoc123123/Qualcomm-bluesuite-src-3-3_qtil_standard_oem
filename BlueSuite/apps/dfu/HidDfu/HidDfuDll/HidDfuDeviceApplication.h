//*******************************************************************************
//
//  HidDfuDeviceApplication.h
//
//  Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDeviceApplication class.
//
//*******************************************************************************

#ifndef HID_DFU_DEVICE_APPLICATION_H
#define HID_DFU_DEVICE_APPLICATION_H

#include "HidDfuDevice.h"
#include "UpgradeProtocol.h"

#include <cfgmgr32.h>

///
/// Class API for HidDfu Device (Application specific functions).
///
class CHidDfuDeviceApplication : public CHidDfuDevice
{
public:
    CHidDfuDeviceApplication();
    CHidDfuDeviceApplication(HANDLE aDeviceHandle,
        const char *apDevicePath, DEVINST aUsbHubDevInst,
        uint16 aVid, uint16 aPid, uint16 aUsage, uint16 aUsagePage);
    ~CHidDfuDeviceApplication();

    ///
    /// Upgrade the device
    /// @return The result.
    ///
    virtual int32 DoDeviceUpgrade();

    ///
    /// Backup the device
    /// @return The result.
    ///
    virtual int32 DoDeviceBackup();

    ///
    /// Get the firmware version from the device.
    /// @param[out] aVersionMajor Version Major
    /// @param[out] aVersionMinor Version Minor
    /// @param[out] aConfigVersion Config Version
    /// @return The result.
    ///
    virtual int32 GetDevFirmwareVersion(
        uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion);

    ///
    /// Initialise.
    /// @return The result.
    ///
    virtual int32 Initialise();

    ///
    /// Send command to DFU mode device to carry out a reset.
    /// Note: The command is NOT supported at present, but might be in future
    /// @param[in] aFromOperationThread Indicates whether the call is from
    /// an operation thread or another source. If set to false, the function
    /// will fail if an operation thread is active, therefore it must be set
    /// to true if called from an operation thread.
    /// @return The result.
    ///
    virtual int32 ResetDevice(bool aFromOperationThread);

    ///
    /// Writes to the connected device.
    /// Note: The command is NOT supported at present, but might be in future
    /// @param[in] apBuffer Write buffer.
    /// @param[in] aLength Length of the write buffer in bytes.
    /// @return The result.
    ///
    virtual int32 WriteToDevice(const uint8* apBuffer, uint32 aLength);

private:

    DEVINST mUsbHubDevInst;     //< Device Instance of the top-level device with matchig VID (in USB Tree)
    uint16 mVid;                //< Vendor ID
    uint16 mPid;                //< Product ID
    uint16 mUsage;              //< USB usage value
    uint16 mUsagePage;          //< USB usage page value

    // Device Path of the connected device
    std::string mDevicePath;

    /// Host State Machine
    typedef enum
    {
        STATE_UPGRADE_IDLE,
        STATE_UPGRADE_CONNECT,
        STATE_UPGRADE_SYNC,
        STATE_UPGRADE_READY,
        STATE_UPGRADE_DATA_READY,
        STATE_UPGRADE_DATA_TRANSFER,
        STATE_UPGRADE_DATA_TRANSFERRING,
        STATE_UPGRADE_DATA_VALIDATION,
        STATE_UPGRADE_DATA_VALIDATING,
        STATE_UPGRADE_DATA_VALIDATED,
        STATE_UPGRADE_COMMIT_HOST_CONTINUE,
        STATE_UPGRADE_COMMIT_VERIFICATION,
        STATE_UPGRADE_COMMIT,
    } HostUpgradeState;

    /// Present state of upgrade on host
    HostUpgradeState mHostState;

    /// Readable Op Codes
    std::map<UpgradeProtocolOpCode, std::string> mOpCodeMap;

    /// Readable Error Codes
    std::map<UpgradeProtocolErrorCode, std::string> mErrorCodeMap;

    /// Readable Connection Status
    std::map<UpgradeStatus, std::string> mConnectionStatusMap;

    /// Upgrade resume point
    UpgradeResumePoint mResumePoint;

    /// Number of bytes device has requested in UPGRADE_DATA_BYTES_REQ
    size_t mRequestedBytes;

    /// Start offset device has communicated in device UPGRADE_DATA_BYTES_REQ
    size_t mRequestedDataOffset;

    /// Current file offset
    size_t mCurrentDataOffset;

    /// Time in ms the host should wait before sending the next UPGRADE_IS_VALIDATION_DONE_REQ message
    uint8 mDelayInValidation;

    /// File handle for binary file
    FILE *mpFileHandle;

    /// Unique upgrade file identifier
    static const uint32 FILE_ID = 0x01020304;

    // Size of data blobs that are sent from HID device to the application
    uint8 mInputReportLenBytes;
    // Size of data blobs that are sent from application to the HID device
    uint8 mOutputReportLenBytes;
    // Size of data blobs (for feature reports) that can be read and/or written (typically related to config info)
    uint8 mFeatureReportLenBytes;

    ///
    /// Calculate the percentage of data read from file for writing to device.
    /// @return Progress in percentage.
    ///
    uint8 CalculateWriteProgress();

    ///
    /// Get the Device Instance ID
    /// @param[in] aDevInst Device Instance for the Device to be tested.
    /// @param[in] aCheck If True, test if the Device has matching VID, PID with a USB Interface.
    /// @return The result.
    ///
    int32 GetDeviceInstanceId(DEVINST aDevInst, bool aCheck = false);

    ///
    /// Searches for matching HID device in Device Information Set
    /// and sets the Device Path for connection.
    /// @param[in] aDevNode Device Node of the device found.
    /// @return The result.
    ///
    int32 GetDevicePath(DEVNODE aDevNode);

    ///
    /// Gets the DFU file length from a file data buffer.
    /// @param[out] apBuffer Data buffer.
    /// @return The file length.
    ///
    uint32 GetDfuFileLength(const uint8 *apBuffer) const;

    ///
    /// Get Connection Status as a readable string for logging and error messages.
    /// @param[in] Connection Status as received from device.
    /// @return Connection Status String.
    ///
    std::string GetConnectionStatusStr(const UpgradeStatus aConnectionStatus) const;

    ///
    /// Get the value assigned to environment variable (if available).
    /// @param[in] apEnvVar Environment Variable.
    /// @parama[in] aValue Value to be used if enironment variable is not set.
    /// @return Value assigned to variable(if available) else aValue(input parameter).
    ///
    uint32 GetEnvVariable(const char *apEnvVar, uint32 aValue) const;

    ///
    /// Get Error Code as a readable string for logging and error messages.
    /// @param[in] Error Code as received from device.
    /// @return Error Code String.
    ///
    std::string GetErrCodeStr(const UpgradeProtocolErrorCode aErrCode) const;

    ///
    /// Get Op Code as a readable string for logging and error messages.
    /// @param[in] Upgrade Protocol Op Code.
    /// @return OpCode String.
    ///
    std::string GetOpCodeStr(const UpgradeProtocolOpCode aOpCode) const;

    ///
    /// Get the size of the HID report as declared by the device.
    /// @return The result.
    ///
    int32 GetSizeOfHidReport();

    ///
    /// Recursive function to search USB tree for matching HID Device.
    /// @param[in] aDevNode Device Node of the parent device.
    /// @return The result.
    ///
    int32 GetSubDevNodesAndDevPath(DEVNODE aDevNode);

    ///
    /// Handles the message received from device, checks against expected opcode and sends error response if necessary.
    /// @param[in] Pointer to message buffer.
    /// @param[in] Length of buffer
    /// @param[in] Expected Op Code.
    /// @param[in] Host State after the message received is analysed.
    /// @return The result.
    ///
    int32 HandleUpgradeProtocolMsgReceived(const uint8 *apBuffer, uint8 aLength, uint8 aExpectedOpCode, HostUpgradeState aNewHostState);

    ///
    /// Establish a HID connection after device restart.
    /// @return The result.
    ///
    int32 HidConnect();

    ///
    /// Wrapper function for call to ReadFile.
    /// @param[in] apBuffer Read buffer.
    /// @return The result.
    ///
    int32 HidDfuReceiveMsg(uint8 *apBuffer);

    ///
    /// Wrapper function for call to WriteFile.
    /// @param[in] aSendToChip Message for device.
    /// @return The result.
    ///
    int32 HidDfuSendMsg(const CUpgradeProtocolMsg& aSendToChip);

    /////
    ///// Read in a timed thread
    ///// @param[out] apBuffer Read buffer.
    ///// @param[out] aBytesRead Number of bytes read.
    ///// @param[out] aReadRetVal The result.
    /////
    void Read(uint8 *apBuffer, uint32 &aBytesRead, int32 &aReadRetVal);

    ///
    /// Reads UPGRADE_DATA_BYTES_REQ and runs a loop to send data to device.
    /// @return The result.
    ///
    int32 ReadDataBytesReq();

    ///
    /// Read response from the device.
    /// @param[in] apBuffer Read buffer.
    /// @param[in] aExpectedOpCode Expected Op Code.
    /// @param[in] aNewHostState New Host State to set after response is analysed.
    /// @return The result.
    ///
    int32 ReadResponse(uint8 *apBuffer, uint8 aExpectedOpCode, HostUpgradeState aNewHostState);

    ///
    /// Re-calculate progress using write progress and host state.
    /// This is done to reset "progress" value taking into consideration reboot delay.
    /// @param[in] aWriteProgress Write progress in percentage.
    /// @return The progress in percentage.
    ///
    uint8 ReCalculateProgress(uint8 aWriteProgress);

    ///
    /// Send HID disconnection followed by connection request (not part of Upgrade Protocol messages).
    /// @return The result.
    ///
    int32 ReConnect();

    ///
    /// Handles the Upgrade Protocol messages send/receive after every connection establishment.
    /// @return The result.
    ///
    int32 RunInitialUpgradeSequence();

    ///
    /// Send HID connection request (not part of Upgrade Protocol messages).
    /// @param[in] aReconnecting Set to true if the call is for a reconnection attempt, otherwise false
    /// @return The result.
    ///
    int32 SendConnectionReq(bool aReconnecting = false);

    ///
    /// Send HID disconnection request (not part of Upgrade Protocol messages).
    /// @return The result.
    ///
    int32 SendDisconnectionReq();

    ///
    /// Send Upgrade Protocol Abort Request.
    /// @return The result.
    ///
    int32 SendUpgradeAbortReq();

    ///
    /// Send Upgrade Protocol Commit Confirm.
    /// @return The result.
    ///
    int32 SendUpgradeCommitCfm();

    ///
    /// Send Upgrade Protocol Data to device, after reading from binary file.
    /// @return The result.
    ///
    int32 SendUpgradeData();

    ///
    /// Send Upgrade Protocol Error Response.
    /// @param[in]
    /// @return The result.
    ///
    int32 SendUpgradeErrorResp(uint16 aErrorCode);

    ///
    /// Send Upgrade Protocol Host Version Request.
    /// @param[out] aVersionMajor Version Major
    /// @param[out] aVersionMinor Version Minor
    /// @param[out] aConfigVersion Config Version
    /// @return The result.
    ///
    int32 SendUpgradeHostVersionReq(uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion);

    ///
    /// Send Upgrade Protocol Image Validation Request to check for completion of the validation process.
    /// @return The result.
    ///
    int32 SendUpgradeIsImageValidDoneReq();

    ///
    /// Send Upgrade Protocol Start Data Request to initiate data transfer.
    /// @return The result.
    ///
    int32 SendUpgradeStartDataReq();

    ///
    /// Send Upgrade Protocol Start Request.
    /// @return The result.
    ///
    int32 SendUpgradeStartReq();

    ///
    /// Send Upgrade Protocol Start Request.to restart the upgrade process.
    /// @return The result.
    ///
    int32 SendUpgradeSyncReq();

    ///
    /// Send Upgrade Protocol Proceed To Commit message.
    /// @return The result.
    ///
    int32 SendUpgradeProceedToCommit();

    ///
   /// Send Upgrade Protocol Transfer Complete Response.
    /// @return The result.
    ///
    int32 SendUpgradeTransferCompleteResp();

    ///
    /// Change the host state.
    /// @param[in] aNewHostState New Host State.
    ///
    void SetHostState(HostUpgradeState aNewHostState);
};

#endif // #ifndef HID_DFU_DEVICE_APPLICATION_H