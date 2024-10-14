//*******************************************************************************
//
//  HidDfuDeviceApplication.cpp
//
//  Copyright (c) 2016-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDeviceApplication class.
//
//*******************************************************************************

#include "HidDfu.h"
#include "HidDfuDeviceApplication.h"

#include <assert.h>
#include <iomanip>
#include <setupapi.h>
#include "time/hi_res_clock.h"
#include "time/stop_watch.h"
#include "unicode/ichar.h"
#include <thread>

extern "C"
{
#include <hidsdi.h>
}

/// Use this to simplify the code populating the map
typedef std::pair<UpgradeProtocolOpCode, std::string> OpCodeStringPair;
typedef std::pair<UpgradeProtocolErrorCode, std::string> ErrCodeStringPair;
typedef std::pair<UpgradeStatus, std::string> ConnectionStatusStringPair;

////////////////////////////////////////////////////////////////////////////////

CHidDfuDeviceApplication::CHidDfuDeviceApplication()
    : CHidDfuDevice(INVALID_HANDLE_VALUE),
    mHostState(STATE_UPGRADE_IDLE),
    mResumePoint(UPGRADE_RESUME_POINT_START),
    mCurrentDataOffset(0),
    mRequestedBytes(0),
    mRequestedDataOffset(0),
    mpFileHandle(NULL)
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDeviceApplication::CHidDfuDeviceApplication(HANDLE aDeviceHandle,
    const char *apDevicePath, DEVINST aUsbHubDevInst,
    uint16 aVid, uint16 aPid, uint16 aUsage, uint16 aUsagePage)
    : CHidDfuDevice(aDeviceHandle),
    mDevicePath(apDevicePath),
    mUsbHubDevInst(aUsbHubDevInst),
    mVid(aVid), mPid(aPid), mUsage(aUsage), mUsagePage(aUsagePage),
    mHostState(STATE_UPGRADE_IDLE),
    mResumePoint(UPGRADE_RESUME_POINT_START),
    mCurrentDataOffset(0),
    mRequestedBytes(0),
    mRequestedDataOffset(0),
    mpFileHandle(NULL)
{
    FUNCTION_DEBUG_SENTRY;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "DeviceHandle: %d", aDeviceHandle);
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "DevicePath: %s", mDevicePath.c_str());
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::Initialise()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Flush transmit buffer
    FlushFileBuffers(mDeviceHandle);

    // Populate the map with opcode and readable string
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_START_REQ, "UPGRADE_START_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_START_CFM, "UPGRADE_START_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_DATA_BYTES_REQ, "UPGRADE_DATA_BYTES_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_DATA, "UPGRADE_DATA"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_ABORT_REQ, "UPGRADE_ABORT_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_ABORT_CFM, "UPGRADE_ABORT_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_TRANSFER_COMPLETE_IND, "UPGRADE_TRANSFER_COMPLETE_IND"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_TRANSFER_COMPLETE_RES, "UPGRADE_TRANSFER_COMPLETE_RES"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_PROCEED_TO_COMMIT, "UPGRADE_PROCEED_TO_COMMIT"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_COMMIT_REQ, "UPGRADE_COMMIT_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_COMMIT_CFM, "UPGRADE_COMMIT_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_ERROR_IND, "UPGRADE_ERROR_IND"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_COMPLETE_IND, "UPGRADE_COMPLETE_IND"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_SYNC_REQ, "UPGRADE_SYNC_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_SYNC_CFM, "UPGRADE_SYNC_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_START_DATA_REQ, "UPGRADE_START_DATA_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_IS_VALIDATION_DONE_REQ, "UPGRADE_IS_VALIDATION_DONE_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_IS_VALIDATION_DONE_CFM, "UPGRADE_IS_VALIDATION_DONE_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_HOST_VERSION_REQ, "UPGRADE_HOST_VERSION_REQ"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_HOST_VERSION_CFM, "UPGRADE_HOST_VERSION_CFM"));
    mOpCodeMap.insert(OpCodeStringPair(UPGRADE_ERROR_RES, "UPGRADE_ERROR_RES"));

    // Populate the map with Error Codes and readable string (for UPGRADE_ERROR_IND)
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_SUCCESS, "UPGRADE_HOST_SUCCESS"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_OEM_VALIDATION_SUCCESS, "UPGRADE_HOST_OEM_VALIDATION_SUCCESS"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_DEPRECATED, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_UNKNOWN_ID, "UPGRADE_HOST_ERROR_UNKNOWN_ID"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_DEPRECATED, "UPGRADE_HOST_ERROR_BAD_LENGTH_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_WRONG_VARIANT, "UPGRADE_HOST_ERROR_WRONG_VARIANT"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_WRONG_PARTITION_NUMBER, "UPGRADE_HOST_ERROR_WRONG_PARTITION_NUMBER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_SIZE_MISMATCH, "UPGRADE_HOST_ERROR_PARTITION_SIZE_MISMATCH"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_FOUND_DEPRECATED, "UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_FOUND_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_OPEN_FAILED, "UPGRADE_HOST_ERROR_PARTITION_OPEN_FAILED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DEPRECATED, "UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_DEPRECATED, "UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_SFS_VALIDATION_FAILED, "UPGRADE_HOST_ERROR_SFS_VALIDATION_FAILED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_DEPRECATED, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_DEPRECATED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_UPDATE_FAILED, "UPGRADE_HOST_ERROR_UPDATE_FAILED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_APP_NOT_READY, "UPGRADE_HOST_ERROR_APP_NOT_READY"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_LOADER_ERROR, "UPGRADE_HOST_ERROR_LOADER_ERROR"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_UNEXPECTED_LOADER_MSG, "UPGRADE_HOST_ERROR_UNEXPECTED_LOADER_MSG"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_MISSING_LOADER_MSG, "UPGRADE_HOST_ERROR_MISSING_LOADER_MSG"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BATTERY_LOW, "UPGRADE_HOST_ERROR_BATTERY_LOW"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INVALID_SYNC_ID, "UPGRADE_HOST_ERROR_INVALID_SYNC_ID"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_IN_ERROR_STATE, "UPGRADE_HOST_ERROR_IN_ERROR_STATE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_NO_MEMORY, "UPGRADE_HOST_ERROR_NO_MEMORY"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_SQIF_ERASE, "UPGRADE_HOST_ERROR_SQIF_ERASE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_PARSE, "UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_PARSE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_TOO_SHORT, "UPGRADE_HOST_ERROR_BAD_LENGTH_TOO_SHORT"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_UPGRADE_HEADER, "UPGRADE_HOST_ERROR_BAD_LENGTH_UPGRADE_HEADER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_HEADER, "UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_HEADER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_SIGNATURE, "UPGRADE_HOST_ERROR_BAD_LENGTH_SIGNATURE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_BAD_LENGTH_DATAHDR_RESUME, "UPGRADE_HOST_ERROR_BAD_LENGTH_DATAHDR_RESUME"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_HEADERS, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_HEADERS"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_UPGRADE_HEADER, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_UPGRADE_HEADER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER1, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER1"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER2, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER2"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_DATA, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_DATA"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_FOOTER, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_FOOTER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_MEMORY, "UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_MEMORY"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED, "UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_HEADER, "UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_HEADER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_PS_SPACE, "UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_PS_SPACE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_MATCHING, "UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_MATCHING"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_TYPE_TWO_DFU, "UPGRADE_HOST_ERROR_PARTITION_TYPE_TWO_DFU"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_HEADER, "UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_HEADER"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DATA, "UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DATA"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_FILE_TOO_SMALL, "UPGRADE_HOST_ERROR_FILE_TOO_SMALL"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_FILE_TOO_BIG, "UPGRADE_HOST_ERROR_FILE_TOO_BIG"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_1, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_1"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_2, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_2"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_3, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_3"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_4, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_4"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_5, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_5"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_6, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_6"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_ERROR_INTERNAL_ERROR_7, "UPGRADE_HOST_ERROR_INTERNAL_ERROR_7"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_WARN_APP_CONFIG_VERSION_INCOMPATIBLE, "UPGRADE_HOST_WARN_APP_CONFIG_VERSION_INCOMPATIBLE"));
    mErrorCodeMap.insert(ErrCodeStringPair(UPGRADE_HOST_WARN_SYNC_ID_IS_DIFFERENT, "UPGRADE_HOST_WARN_SYNC_ID_IS_DIFFERENT"));

    // Populate the map with Connection Status and readable string (for response to HID_CMD_CONNECTION_REQ)
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_SUCCESS, "UPGRADE_STATUS_SUCCESS"));
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_UNEXPECTED_ERROR, "UPGRADE_STATUS_UNEXPECTED_ERROR"));
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_ALREADY_CONNECTED_WARNING, "UPGRADE_STATUS_ALREADY_CONNECTED_WARNING"));
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_IN_PROGRESS, "UPGRADE_STATUS_IN_PROGRESS"));
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_BUSY, "UPGRADE_STATUS_BUSY"));
    mConnectionStatusMap.insert(ConnectionStatusStringPair(UPGRADE_STATUS_INVALID_POWER_STATE, "UPGRADE_STATUS_INVALID_POWER_STATE"));

    retVal = GetSizeOfHidReport();
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDeviceApplication::~CHidDfuDeviceApplication()
{
    FUNCTION_DEBUG_SENTRY;

    // Flush transmit buffer
    if (mLastDevError.GetErrorCode() != HIDDFU_ERROR_NO_RESPONSE)
    {
        FlushFileBuffers(mDeviceHandle);
    }
}

////////////////////////////////////////////////////////////////////////////////

uint8 CHidDfuDeviceApplication::CalculateWriteProgress()
{
    uint8 writeProgress = 0;
    FUNCTION_DEBUG_SENTRY_RET(uint8, writeProgress);

    // Get current position
    long currentPos = ftell(mpFileHandle);

    // Go to end, and get end position
    int seekEnd = fseek(mpFileHandle, 0, SEEK_END);
    long endPos = ftell(mpFileHandle);

    // Rewind
    fseek(mpFileHandle, currentPos, SEEK_SET);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "currentPos = %u endPos = %u seekEnd = %d",
            currentPos, endPos, seekEnd);

    // Calculate Write progress
    writeProgress = static_cast<uint8>((currentPos * 100) / endPos);
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Progress = %d%%", writeProgress);

    // Calculate overall progress and then set
    uint8 progress = ReCalculateProgress(writeProgress);
    SetProgress(progress);

    return writeProgress;
}

//////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::DoDeviceBackup()
{
    int32 retVal;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Not supported
    retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_UNSUPPORTED);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::DoDeviceUpgrade()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    retVal = RunInitialUpgradeSequence();

    // Set reboot stopped flag
    // Do not send 'Disconnect Request' if operation was stopped while reboot was in progress,
    // because device will not be ready to receive the 'Disconnect Request'
    bool rebootStopped = false;

    if ((retVal == HIDDFU_ERROR_NONE) && (mResumePoint == UPGRADE_RESUME_POINT_START))
    {
        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_DATA_READY))
        {
            retVal = SendUpgradeStartDataReq();
        }

        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_DATA_TRANSFER))
        {
            retVal = ReadDataBytesReq();
        }

        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_DATA_VALIDATION))
        {
            // Time duration during which host can send a data validation request(s)
            const uint32 maxTimeForValidationMs = GetEnvVariable("HID_VALIDATION_TIME_MS", 60000);

            StopWatch timeSinceStart;
            retVal = SendUpgradeIsImageValidDoneReq();

            while ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_DATA_VALIDATING)
                && (timeSinceStart.duration() < maxTimeForValidationMs)
                && CheckKeepGoing())
            {
                // Send the data validation request again
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Wait(%d milliseconds) for validation", mDelayInValidation);
                HiResClockSleepMilliSec(mDelayInValidation);
                retVal = SendUpgradeIsImageValidDoneReq();
            }

            if ((retVal == HIDDFU_ERROR_NONE) && (mHostState != STATE_UPGRADE_DATA_VALIDATED))
            {
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED, "Device failed to validate data");
            }
        }

        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_DATA_VALIDATED))
        {
            retVal = SendUpgradeTransferCompleteResp();
        }

        // Wait for reboot
        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_IDLE))
        {
            // Close Device Handle
            retVal = DisconnectDevice();

            // Wait (in milliseconds) time after data validation has completed and device is rebooting
            // before attempting to reconnect
            HiResClockMilliSec hidRestartDelayMs = (GetEnvVariable("HID_RESTART_DELAY_SEC", RESTART_DELAY_SEC)) * 1000;

            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Wait(%d seconds) for device to reboot", hidRestartDelayMs/1000);
            StopWatch restartTime;
            while ((restartTime.duration() < hidRestartDelayMs)
                && CheckKeepGoing())
            {
                // Wait for a second
                HiResClockSleepMilliSec(1000);
            }

            if (!CheckKeepGoing())
            {
                // Thread is stopped, and device is rebooting, set flag to true
                rebootStopped = true;
            }
        }
    }

    // Gets the sub nodes after restart, and sets Device Path
    if (retVal == HIDDFU_ERROR_NONE)
    {
        // USB Hub - Top Level enumeration for the USB Device to connect
        // Check if it is available
        retVal = GetDeviceInstanceId((mUsbHubDevInst));

        // Get the child (USB Composite Device)
        DEVINST childDevInst;
        if (retVal == HIDDFU_ERROR_NONE)
        {
            CONFIGRET status = CM_Get_Child(&childDevInst, mUsbHubDevInst, 0);
            if (status == CR_SUCCESS)
            {
                // Check if the child is available
                retVal = GetDeviceInstanceId((childDevInst));
            }
            else
            {
                std::ostringstream errorStr;
                errorStr << "Failed to get Child Device Instance for " << mUsbHubDevInst << "(status = " << status << ")";
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED, errorStr.str());
            }
        }

        std::string oldDevicePath = mDevicePath;
        mDevicePath.clear();

        // Search the sub tree to get the new Device Path
        if (retVal == HIDDFU_ERROR_NONE)
        {
            retVal = GetSubDevNodesAndDevPath(childDevInst);
        }

        if (retVal == HIDDFU_ERROR_NONE)
        {
            if (oldDevicePath.compare(mDevicePath))
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Device Path changed after restart");
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Old Device Path: %s", oldDevicePath.c_str());
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "New Device Path: %s", mDevicePath.c_str());
            }

            if (mDevicePath.empty())
            {
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Failed to find device after restart");
            }
        }
    }

    // After reboot, connect to use new image
    if ((retVal == HIDDFU_ERROR_NONE) && CheckKeepGoing())
    {
        uint16 attempt = 1;
        const uint16 MAX_ATTEMPTS = 10;

        do {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "[Attempt: %d], DevicePath: %s", attempt,
                mDevicePath.c_str());
            retVal = HidConnect();
            if (retVal != HIDDFU_ERROR_NONE)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Wait(2 seconds)");
                HiResClockSleepMilliSec(2000);
            }
            attempt++;
        } while (attempt <= MAX_ATTEMPTS && (retVal != HIDDFU_ERROR_NONE));
    }

    if ((retVal == HIDDFU_ERROR_NONE) && CheckKeepGoing())
    {
        retVal = RunInitialUpgradeSequence();
    }

    if ((retVal == HIDDFU_ERROR_NONE) && (mResumePoint == UPGRADE_RESUME_POINT_POST_REBOOT) && CheckKeepGoing())
    {
        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_COMMIT_HOST_CONTINUE))
        {
            retVal = SendUpgradeProceedToCommit();
        }

        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_COMMIT_VERIFICATION))
        {
            retVal = SendUpgradeCommitCfm();
        }

        if ((retVal == HIDDFU_ERROR_NONE))
        {
            SendDisconnectionReq();
        }
    }

    // If thread is stopped and device is not rebooting then send a 'Disconnect Request'
    if (!CheckKeepGoing() && (!rebootStopped))
    {
        SendDisconnectionReq();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::GetDeviceInstanceId(DEVINST aDevInst, bool aCheck)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(uint32, retVal);
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "DevInst: %d", aDevInst);

    //Get Device Instance ID
    char *pDevInstID = new char[MAX_DEVICE_ID_LEN];
    CONFIGRET status = CM_Get_Device_IDA(aDevInst, pDevInstID, MAX_PATH, 0);
    if (status == CR_SUCCESS)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Device: %s", pDevInstID);
        if (aCheck)
        {
            // Test for matching VID, PID with USB Interface (MI_)

            std::string str(pDevInstID, MAX_DEVICE_ID_LEN);

            // USB Vendor and Product Identifier with USB Interface (MI_)
            // e.g. VID_xxxx&PID_xxxx&MI_
            std::ostringstream vidPidMi;
            vidPidMi << std::uppercase << "VID_" << std::hex << std::setfill('0') << std::setw(4) << mVid 
                << "&PID_" << std::setw(4) << mPid << "&MI_";

            if (str.find(vidPidMi.str()) != std::string::npos)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Found matching device");
            }
            else
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Match failed for VID, PID and I/F");
                retVal = HIDDFU_ERROR_UNKNOWN;
            }
        }
    }
    else
    {
        std::ostringstream errorStr;
        errorStr << "Failed to get Device Instance ID for " << aDevInst << "(status = " << status << ")";
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED, errorStr.str());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

uint32 CHidDfuDeviceApplication::GetEnvVariable(const char *apEnvVar, uint32 aValue) const
{
    uint32 retVal = aValue;
    FUNCTION_DEBUG_SENTRY_RET(uint32, retVal);

    const char *pEnvVar = getenv(apEnvVar);
    if (pEnvVar != NULL)
    {
        long lVal = strtol(pEnvVar, NULL, 0);

        // If no valid conversion could be performed, a zero value is returned
        // then, set to default
        if (lVal == 0)
        {
            retVal = aValue;
        }
        else
        {
            retVal = static_cast<uint32>(lVal);
        }
    }

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "%s = %u", apEnvVar, retVal);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

std::string CHidDfuDeviceApplication::GetConnectionStatusStr(const UpgradeStatus aConnectionStatus) const
{
    std::string connectionStatusStr;
    FUNCTION_DEBUG_SENTRY_RET(std::string, connectionStatusStr);

    std::map<UpgradeStatus, std::string>::const_iterator it = mConnectionStatusMap.find(aConnectionStatus);

    if (it != mConnectionStatusMap.end())
    {
        connectionStatusStr = it->second;
    }
    else
    {
        connectionStatusStr = "Unknown Connection Status";
    }

    return connectionStatusStr;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::GetDevicePath(DEVNODE aDevNode)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    char *pDevInstID = new char[MAX_DEVICE_ID_LEN];
    CONFIGRET status = CM_Get_Device_IDA(aDevNode, pDevInstID, MAX_PATH, 0);
    if (status == CR_SUCCESS)
    {
        GUID guid;
        // Get device intereface GUID for HID Class devices.
        HidD_GetHidGuid(&guid);
        // Get a handle to the device information set (for HID Class devices)
        // that contains requested device information elements
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&guid, pDevInstID, NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (deviceInfoSet == INVALID_HANDLE_VALUE)
        {
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN,
                "Failed to get Device Information Set");
        }
        else
        {
            // Iterate through the devices found - look for one that matches
            // the VID, PID, usage and usage page
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

                deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                // Get details about the device interface
                if(!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &interfaceDeviceData,
                    deviceInterfaceDetailData, requiredSize, NULL, NULL))
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

                        if (attributes.VendorID == mVid && attributes.ProductID == mPid)
                        {
                            PHIDP_PREPARSED_DATA preparsedData;
                            HidD_GetPreparsedData(deviceHandle, &preparsedData);

                            HIDP_CAPS capabilities;
                            HidP_GetCaps(preparsedData, &capabilities);

                            // Check Usage and Usage Page match.
                            // Usage and Usage Page check is optional (zero is reserved
                            // value in both cases, so using zero as "don't check").
                            if ((mUsage == 0 || capabilities.Usage == mUsage) &&
                                (mUsagePage == 0 || capabilities.UsagePage == mUsagePage))
                            {
                                // Set the Device Path
                                mDevicePath = deviceInterfaceDetailData->DevicePath;
                                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Setting DevicePath: %s", mDevicePath.c_str());
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
    else
    {
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN,
            "Failed to get Device Instance ID");
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

std::string CHidDfuDeviceApplication::GetErrCodeStr(const UpgradeProtocolErrorCode aErrCode) const
{
    std::string errCodeStr;
    FUNCTION_DEBUG_SENTRY_RET(std::string, errCodeStr);

    std::map<UpgradeProtocolErrorCode, std::string>::const_iterator it = mErrorCodeMap.find(aErrCode);

    if (it != mErrorCodeMap.end())
    {
        errCodeStr = it->second;
    }
    else
    {
        errCodeStr = "Unknown Error Code";
    }

    return errCodeStr;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::GetSubDevNodesAndDevPath(DEVNODE aDevNode)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // If the Device Node has got same VID, PID with USB Interface,
    // search for HID Device under it's sub nodes
    if (GetDeviceInstanceId(aDevNode, true) == HIDDFU_ERROR_NONE)
    {
        retVal = GetDevicePath(aDevNode);
    }

    if (retVal == HIDDFU_ERROR_NONE)
    {
        // Traverse the USB Tree
        do
        {
            // Get Device Instance handle to the first child node
            DEVNODE devNodeChild;
            CONFIGRET status = CM_Get_Child(&devNodeChild, aDevNode, 0);

            // Get Device Instance ID for logging, ignore the return value
            (void)GetDeviceInstanceId(devNodeChild);

            if (status == CR_SUCCESS)
            {
                // Recurse (Ignore the return value and check for the sibling)
                (void)GetSubDevNodesAndDevPath(devNodeChild);
            }
            else
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Get first child for DevNode %d failed, status = %d",
                    aDevNode, status);
            }

            // Get Device Instance Handle to the next sibling node
            DEVNODE devNodeSibling;
            status = CM_Get_Sibling(&devNodeSibling, aDevNode, 0);

            // Get Device Instance ID for logging, ignore the return value
            (void)GetDeviceInstanceId(devNodeSibling);

            if (status != CR_SUCCESS)
            {
                devNodeSibling = NULL;
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Get Sibling for DevNode %d failed, status = %d",
                    aDevNode, status);
            }

            aDevNode = devNodeSibling;
        } while (aDevNode != NULL && mDevicePath.empty());
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::GetDevFirmwareVersion(
    uint16 &aVersionMajor, uint16 &aVersionMinor, uint16 &aConfigVersion)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    ResetThreadFuncPointer();

    if (mHostState == STATE_UPGRADE_IDLE)
    {
        retVal = SendConnectionReq();
    }

    if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_CONNECT))
    {
        retVal = SendUpgradeHostVersionReq(aVersionMajor, aVersionMinor, aConfigVersion);
    }

    // Send Disconnect
    SendDisconnectionReq();

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

std::string CHidDfuDeviceApplication::GetOpCodeStr(const UpgradeProtocolOpCode aOpCode) const
{
    std::string opCodeStr;
    FUNCTION_DEBUG_SENTRY_RET(std::string, opCodeStr);

    std::map<UpgradeProtocolOpCode, std::string>::const_iterator it = mOpCodeMap.find(aOpCode);

    if (it != mOpCodeMap.end())
    {
        opCodeStr = it->second;
    }
    else
    {
        opCodeStr = "Unknown OpCode";
    }

    return opCodeStr;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::HandleUpgradeProtocolMsgReceived(
        const uint8 *apBuffer, const uint8 aLength, const uint8 aExpectedOpCode, const HostUpgradeState aNewHostState)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    assert(apBuffer);

    CUpgradeProtocolMsg msg(apBuffer, aLength);

    if(msg.GetUpgradeMsgSize() != 0)
    {
        uint8 opCodeReceived = msg.GetOpCode();

        if(opCodeReceived == aExpectedOpCode)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC,
                "Received Message with OpCode(0x%x): %s",
                aExpectedOpCode, GetOpCodeStr(static_cast<UpgradeProtocolOpCode>(aExpectedOpCode)).c_str());

            SetHostState(aNewHostState);
            retVal = HIDDFU_ERROR_NONE;
        }
        else
        {
            std::ostringstream errorStr;

            // If the device has sent UPGRADE_ERROR_IND in response to any host message.
            if(opCodeReceived == UPGRADE_ERROR_IND)
            {
                CUpgradeErrorInd errorIndMsg(apBuffer, aLength);
                uint16 errorCode = errorIndMsg.GetErrorCode();
                errorStr << "Received Error Indication from chip, error code: 0x" << std::hex << errorCode
                        << "(" << GetErrCodeStr(static_cast<UpgradeProtocolErrorCode>(errorCode)).c_str() << ")";

                // Send UPGRADE_ERROR_RES
                SendUpgradeErrorResp(errorCode);
            }
            else
            {
                errorStr << "Unexpected message (" 
                        << GetOpCodeStr(static_cast<UpgradeProtocolOpCode>(opCodeReceived)).c_str() << ")"
                        << " received from the chip";
            }

            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED, errorStr.str());

            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC,
                "Expected OpCode(0x%x): %s Received OpCode(0x%x): %s ",
                aExpectedOpCode, GetOpCodeStr(static_cast<UpgradeProtocolOpCode>(aExpectedOpCode)).c_str(),
                opCodeReceived, GetOpCodeStr(static_cast<UpgradeProtocolOpCode>(opCodeReceived)).c_str());

        }
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Received HID message with length: %d", msg.GetUpgradeMsgSize());
    }

    if (mHostState != aNewHostState)
    {
        HiResClockSleepMilliSec(10);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::HidConnect()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "DevicePath: %s", mDevicePath.c_str());

    // Open the device using device path
    HANDLE deviceHandle = CreateFile(mDevicePath.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);

    if (deviceHandle == INVALID_HANDLE_VALUE)
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_CONNECTION, "Unable to establish a HID connection", ::GetLastError());
    }
    else
    {
        mDeviceHandle = deviceHandle;
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mDeviceHandle: %d", mDeviceHandle);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

void CHidDfuDeviceApplication::Read(uint8 *apBuffer, uint32 &aBytesRead, int32 &aReadRetVal)
{
    FUNCTION_DEBUG_SENTRY;

    if (!ReadFile(mDeviceHandle, apBuffer, mInputReportLenBytes, &aBytesRead, NULL))
    {
        aReadRetVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
            "Failed to read from device", ::GetLastError());
    }
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::HidDfuReceiveMsg(uint8 *apBuffer)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Use a temporary buffer to pass to thread
    uint8 *pBuffer = new uint8[mInputReportLenBytes];
    memset(pBuffer, 0, mInputReportLenBytes);

    // Read from device in a seperate thread, so that if the read hangs
    // then the timed thread will ensure that there is a mechanism to report the failure
    uint32 bytesRead = 0;
    std::thread t(&CHidDfuDeviceApplication::Read, this, pBuffer, std::ref(bytesRead), std::ref(retVal));

    StopWatch timeSinceStart;
    // Time duration for which host will wait for a response
    const uint32 maxWaitTimeMs = GetEnvVariable("HID_WAIT_TIME_MS", 70000);

    while ((timeSinceStart.duration() < maxWaitTimeMs) && (bytesRead != mInputReportLenBytes)
            && (retVal == HIDDFU_ERROR_NONE) && CheckKeepGoing())
    {
        HiResClockSleepMilliSec(5);
    }

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "timeSinceStart.duration()=%d bytesRead=%d retVal=%d",
            timeSinceStart.duration(), bytesRead, retVal);

    if ((retVal == HIDDFU_ERROR_NONE) && CheckKeepGoing())
    {
        if (bytesRead == mInputReportLenBytes)
        {
            MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "ReadFile[...]", pBuffer, mInputReportLenBytes);
            t.join();

            memcpy(apBuffer, pBuffer, mInputReportLenBytes);

        }
        else if (bytesRead != 0 && bytesRead != mInputReportLenBytes)
        {
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Not enough bytes received from the device.");
            t.detach();
        }
        else
        {
            retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_NO_RESPONSE);
            t.detach();
        }
    }
    else
    {
        t.detach();

        if (bytesRead == mInputReportLenBytes)
        {
            MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "ReadFile[...]", pBuffer, mInputReportLenBytes);
            memcpy(apBuffer, pBuffer, mInputReportLenBytes);
        }
    }

    delete[] pBuffer;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::HidDfuSendMsg(const CUpgradeProtocolMsg& aSendToChip)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Sending %s",
        GetOpCodeStr(static_cast<UpgradeProtocolOpCode>(aSendToChip.GetOpCode())).c_str());

    // Send as raw buffer
    uint8 *pBuffer = new uint8[mOutputReportLenBytes];
    memset(pBuffer, 0, mOutputReportLenBytes);
    aSendToChip.Serialise(pBuffer, mOutputReportLenBytes);

    uint32 bytesWritten = 0;
    if (!WriteFile(mDeviceHandle, pBuffer, mOutputReportLenBytes, &bytesWritten, NULL))
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, 
                "Failed to write to device", ::GetLastError());
    }

    delete[] pBuffer;

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////

uint8 CHidDfuDeviceApplication::ReCalculateProgress(uint8 aWriteProgress)
{
    uint8 progress = aWriteProgress;
    FUNCTION_DEBUG_SENTRY_RET(uint8, progress);

    if ((mResumePoint != UPGRADE_RESUME_POINT_POST_REBOOT) && (mResumePoint != UPGRADE_RESUME_POINT_POST_COMMIT))
    {   // Progress should not get to 100% unless device has rebooted and commit has completed
        if (aWriteProgress >= PROGRESS_REBOOT_VALUE)
        {
            progress = PROGRESS_REBOOT_VALUE;
        }
    }
    else
    {   // Once file has been written set the progress based on Host State
        switch (mHostState)
        {
        case STATE_UPGRADE_COMMIT_HOST_CONTINUE:
            progress = 98;
            break;
        case STATE_UPGRADE_COMMIT_VERIFICATION:
            progress = 99;
            break;
        case STATE_UPGRADE_COMMIT:
            progress = 100;
            break;
        }
    }

    return progress;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::ReadDataBytesReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Open the file and get the file handle
    mpFileHandle = fopen(mFileName.c_str(), "rb");
    if (mpFileHandle == NULL)
    {
        std::ostringstream msg;
        msg << "Failed to open file \"" << mFileName << "\"";
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_FILE_OPEN_FAILED, msg.str());
    }

    while((mResumePoint != UPGRADE_RESUME_POINT_PRE_VALIDATE)
            && (retVal == HIDDFU_ERROR_NONE)
            && CheckKeepGoing())
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_DATA_BYTES_REQ, STATE_UPGRADE_DATA_TRANSFERRING);

        CUpgradeDataBytesReq msg(pResponseBuf, mInputReportLenBytes);

        if((msg.GetUpgradeMsgSize() != 0) && (retVal == HIDDFU_ERROR_NONE))
        {
            mRequestedBytes = msg.GetNumberOfBytes();
            mRequestedDataOffset = msg.GetStartOffset();
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC,
                    "mRequestedBytes: 0x%x mRequestedDataOffset: 0x%x",
                    mRequestedBytes, mRequestedDataOffset);
        }

        delete[] pResponseBuf;

        if (retVal == HIDDFU_ERROR_NONE && mHostState == STATE_UPGRADE_DATA_TRANSFERRING && CheckKeepGoing())
        {
            retVal = SendUpgradeData();
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::ReadResponse(
        uint8 *apBuffer, uint8 aExpectedOpCode, HostUpgradeState aNewHostState)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    memset(apBuffer, 0, mInputReportLenBytes);
    apBuffer[0] = HID_REPORTID_RESPONSE;

    retVal = HidDfuReceiveMsg(apBuffer);
    if(retVal == HIDDFU_ERROR_NONE)
    {
        retVal = HandleUpgradeProtocolMsgReceived(apBuffer, mInputReportLenBytes, aExpectedOpCode, aNewHostState);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::ResetDevice(const bool aFromOperationThread)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Not supported
    retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_UNSUPPORTED);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::GetSizeOfHidReport()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    PHIDP_PREPARSED_DATA pPreParsedData = NULL; // The opaque parser info describing this device
    HIDP_CAPS hidCapability;                    // The capabilities of the HID device

    if (!HidD_GetPreparsedData(mDeviceHandle, &pPreParsedData))
    {
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Error: HidD_GetPreparsedData failed.");
    }

    if (retVal == HIDDFU_ERROR_NONE)
    {
        if (!HidP_GetCaps(pPreParsedData, &hidCapability))
        {
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Error: HidP_GetCaps failed.");
        }
        else
        {
            // Size of data field is 8-bit in HID message, so perform a range check
            if ((hidCapability.InputReportByteLength > std::numeric_limits<uint8>::max()) 
                    || (hidCapability.OutputReportByteLength > std::numeric_limits<uint8>::max())
                    || (hidCapability.FeatureReportByteLength > std::numeric_limits<uint8>::max()))
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "hidCapability.InputReportByteLength = %d", hidCapability.InputReportByteLength);
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "hidCapability.OutputReportByteLength = %d", hidCapability.OutputReportByteLength);
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "hidCapability.FeatureReportByteLength = %d", hidCapability.FeatureReportByteLength);
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Report length out of range.");
            }
            else
            {
                // Size of data blobs

                // that are sent from HID device to the application
                mInputReportLenBytes = static_cast<uint8>(hidCapability.InputReportByteLength);
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mInputReportLenByte = %d", mInputReportLenBytes);

                // that are sent from application to the HID device
                mOutputReportLenBytes = static_cast<uint8>(hidCapability.OutputReportByteLength);
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mOutputReportLenByte = %d", mOutputReportLenBytes);

                // that can be manually read and/or written (typically related to config info)
                mFeatureReportLenBytes = static_cast<uint8>(hidCapability.FeatureReportByteLength);
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mFeatureReportLenByte = %d", mFeatureReportLenBytes);
            }
        }
    }

    HidD_FreePreparsedData(pPreParsedData);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::RunInitialUpgradeSequence()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_IDLE))
    {
        retVal = SendConnectionReq();
    }

    if (mResumePoint == UPGRADE_RESUME_POINT_START)
    {
        // Reset state
        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_CONNECT))
        {
            retVal = SendUpgradeAbortReq();
        }

        // Reset State, because we just forced it to abort
        if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_IDLE))
        {
            SetHostState(STATE_UPGRADE_CONNECT);
        }
    }

    if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_CONNECT))
    {
        retVal = SendUpgradeSyncReq();
    }

    if ((retVal == HIDDFU_ERROR_NONE) && (mHostState == STATE_UPGRADE_READY))
    {
        retVal = SendUpgradeStartReq();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::ReConnect()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Send Disconnect
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Attempt to reconnect");
    retVal = SendDisconnectionReq();

    //Send Connect
    if (retVal == HIDDFU_ERROR_NONE)
    {
        retVal = SendConnectionReq(true);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendConnectionReq(bool aReconnecting)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    uint8 *pBuffer = new uint8[mFeatureReportLenBytes];
    memset(pBuffer, 0, mFeatureReportLenBytes);

    // Report ID, Size, Command Log, Command High
    pBuffer[0] = HID_REPORTID_COMMAND;
    pBuffer[1] = 1;
    pBuffer[2] = HID_CMD_CONNECTION_REQ;

    MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "SetFeature[...]", pBuffer, mFeatureReportLenBytes);

    // Check state, if idle
    if (mHostState != STATE_UPGRADE_IDLE)
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if(HidD_SetFeature(mDeviceHandle, (PVOID)pBuffer, mFeatureReportLenBytes))
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];
        memset(pResponseBuf, 0, mInputReportLenBytes);

        retVal = HidDfuReceiveMsg(pResponseBuf);
        if(retVal == HIDDFU_ERROR_NONE)
        {
            // Status is in third byte of the HID Connection Response
            UpgradeStatus status = static_cast<UpgradeStatus>(pResponseBuf[2]);

            if (status == UPGRADE_STATUS_SUCCESS)
            {
                SetHostState(STATE_UPGRADE_CONNECT);
            }
            else if (status == UPGRADE_STATUS_ALREADY_CONNECTED_WARNING)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC,
                    "Warning: Already connected, received connection status from chip (0x%x): %s",
                    status, GetConnectionStatusStr(status).c_str());

                if (aReconnecting)
                {
                    std::ostringstream errorStr;
                    errorStr << "Reconnect failure, status received from chip : 0x" << std::hex << status
                        << " (" << GetConnectionStatusStr(status) << ")";
                    retVal = mLastDevError.SetMsg(HIDDFU_ERROR_CONNECTION, errorStr.str());
                }
                else
                {
                    retVal = ReConnect();
                }
            }
            else
            {
                std::ostringstream errorStr;
                if (static_cast<UpgradeStatus>(pResponseBuf[1]) == 1)
                {
                    errorStr << "Connection failure, status received from chip : 0x" << std::hex << status
                        << " (" << GetConnectionStatusStr(status) << ")";
                }
                else if (static_cast<UpgradeStatus>(pResponseBuf[1]) > 1)
                {
                    UpgradeProtocolOpCode responseMsg = static_cast<UpgradeProtocolOpCode>(pResponseBuf[2]);
                    errorStr << "Connection failure, received unexpected response: 0x" << std::hex << status
                        << " (" << GetOpCodeStr(responseMsg).c_str() << ")";
                }
                else
                {
                    errorStr << "Connection failure, unknown response";
                }
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_CONNECTION, errorStr.str());
            }
        }

        delete[] pResponseBuf;
    }
    else
    {
        retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, 
                "Failed to send HID_CMD_CONNECTION_REQ", ::GetLastError());
    }

    delete[] pBuffer;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendDisconnectionReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if ((mLastDevError.GetErrorCode() != HIDDFU_ERROR_NO_RESPONSE))
    {
        uint8 *pBuffer = new uint8[mFeatureReportLenBytes];
        memset(pBuffer, 0, mFeatureReportLenBytes);

        // Report ID, Size, Command Log, Command High
        pBuffer[0] = HID_REPORTID_COMMAND;
        pBuffer[1] = 1;
        pBuffer[2] = HID_CMD_DISCONNECT_REQ;

        MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "SetFeature[...]", pBuffer, mFeatureReportLenBytes);

        if (!HidD_SetFeature(mDeviceHandle, (PVOID)pBuffer, mFeatureReportLenBytes))
        {
            retVal = SetLastErrorFromWinError(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE,
                "Failed to send HID_CMD_DISCONNECT_REQ", ::GetLastError());
        }

        mHostState = STATE_UPGRADE_IDLE;

        delete[] pBuffer;
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Device is not responsive, skip sending Disconnect request");
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeAbortReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeAbortReq abortReq(mOutputReportLenBytes);

    retVal = HidDfuSendMsg(abortReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mOutputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_ABORT_CFM, STATE_UPGRADE_IDLE);
        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeCommitCfm()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeCommitCfm commitCfm(mOutputReportLenBytes);

    // Action
    // 0x00 – Commit upgrade    0x01 – Rollback upgrade
    uint8 action = 0x00;
    commitCfm.SetAction(action);

    retVal = HidDfuSendMsg(commitCfm);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mOutputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_COMPLETE_IND, STATE_UPGRADE_COMMIT);

        if (retVal == HIDDFU_ERROR_NONE)
        {
            // Upgrade process is complete
            mResumePoint = UPGRADE_RESUME_POINT_POST_COMMIT;
        }
        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeData()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);
    size_t maxDataSize = mOutputReportLenBytes - (HEADER_SIZE + 1); // Max size of data excluding header and "More Data" field  of 1 byte

    if (mRequestedBytes == 0)
    {
        retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Requested data size is 0, unhandled.");
    }
    else
    {
        uint8 *pBuffer = new uint8[maxDataSize];

        // Move to the 'requested data offset' received from device in UPGRADE_DATA_BYTES_REQ message
        fseek(mpFileHandle, static_cast<long>(mRequestedDataOffset), SEEK_CUR);

        size_t dataSendLength = 0;
        size_t octetsToSend = mRequestedBytes;
        size_t readLength;

        const uint32 maxTimeBetweenDataSends = GetEnvVariable("HID_DATA_SEND_WAIT_TIME_MS", 2);

        // If the requested length is greater than maxDataSize, it will be necessary to send the data read
        // from file in more than one UPGRADE_DATA message. The 'keep going' check is present as the
        // requested data size may be very large and we don't want to prevent the user from stopping
        // the upgrade in this case.
        while ((retVal == HIDDFU_ERROR_NONE) && (octetsToSend > 0) && CheckKeepGoing())
        {
            memset(pBuffer, 0, maxDataSize);
            readLength = fread(pBuffer, sizeof(pBuffer[0]), octetsToSend > maxDataSize ? maxDataSize : octetsToSend, mpFileHandle);

            if (ferror(mpFileHandle))
            {
                retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_FILE_READ_FAILED);
            }

            if (readLength <= 0)
            {
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File format is invalid, no payload found");
            }

            if (retVal == HIDDFU_ERROR_NONE)
            {
                MSG_HANDLER_NOTIFY_DEBUG_BUFFER(DEBUG_BASIC, "ImageData[...]", static_cast<uint8*>(pBuffer), static_cast<uint32>(readLength));
                CUpgradeDataReq dataReq(mOutputReportLenBytes);
                dataReq.SetUpgradeMsgSize(static_cast<uint8>(readLength + 4));
                dataReq.SetLength(static_cast<uint16>(readLength + 1));
                dataReq.SetImageData(pBuffer, static_cast<uint8>(readLength));

                mCurrentDataOffset += readLength;
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mCurrentDataOffset: %d", mCurrentDataOffset);

                if (CalculateWriteProgress() == 100)
                {
                    dataReq.SetMoreData(1); // More data 1 – Last packet of data from the upgrade file

                    // If we are done with all data then changed state, and proceed for validation
                    mResumePoint = UPGRADE_RESUME_POINT_PRE_VALIDATE;
                    SetHostState(STATE_UPGRADE_DATA_VALIDATION);
                }
                else
                {
                    dataReq.SetMoreData(0); // More data 0 – There is more data in the upgrade file
                }

                retVal = HidDfuSendMsg(dataReq);
                octetsToSend -= readLength;

                if (octetsToSend > 0)
                {
                    HiResClockSleepMilliSec(maxTimeBetweenDataSends);
                }
            }
        }

        delete[] pBuffer;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeErrorResp(uint16 aErrorCode)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeErrorRes errorResp(mOutputReportLenBytes);

    errorResp.SetErrorCode(aErrorCode);

    // We are not handling return values here and
    // are doing a best attempt to exit as cleanly as possible
    // without modifying the last error code

    // Write UPGRADE_ERROR_RES
    HidDfuSendMsg(errorResp);

    // Reset state to Idle
    SetHostState(STATE_UPGRADE_IDLE);

    // Send Disconnect
    SendDisconnectionReq();

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeHostVersionReq(uint16 &aVersionMajor, uint16 &aVersionMinor,
    uint16 &aConfigVersion)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeHostVersionReq hostVersionReq(mOutputReportLenBytes);

    retVal = HidDfuSendMsg(hostVersionReq);

    if (retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_HOST_VERSION_CFM, mHostState);

        if (retVal == HIDDFU_ERROR_NONE)
        {
            CUpgradeHostVersionCfm msg(pResponseBuf, mInputReportLenBytes);
            aVersionMajor = msg.GetVersionMajor();
            aVersionMinor = msg.GetVersionMinor();
            aConfigVersion = msg.GetConfigVersion();
        }

        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeIsImageValidDoneReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeIsValidationDoneReq validationReq(mOutputReportLenBytes);

    HidDfuSendMsg(validationReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];

        // Response can be UPGRADE_IS_VALIDATION_DONE_CFM or UPGRADE_TRANSFER_COMPLETE_IND,
        retVal = ReadResponse(pResponseBuf, UPGRADE_TRANSFER_COMPLETE_IND, STATE_UPGRADE_DATA_VALIDATED);

        if ((retVal == HIDDFU_ERROR_NONE) && (pResponseBuf[2] == UPGRADE_TRANSFER_COMPLETE_IND))
        {
            mResumePoint = UPGRADE_RESUME_POINT_PRE_REBOOT;
        }
        else
        {
            // If it is UPGRADE_IS_VALIDATION_DONE_CFM we need to send UPGRADE_IS_VALIDATION_DONE_REQ again
            if (mHostState != STATE_UPGRADE_DATA_VALIDATED)
            {
                if (pResponseBuf[2] == UPGRADE_IS_VALIDATION_DONE_CFM)
                {
                    CUpgradeIsValidationDoneCfm isValidationDoneCfm(pResponseBuf, mInputReportLenBytes);
                    mDelayInValidation = isValidationDoneCfm.GetDelayTime();
                    mHostState = STATE_UPGRADE_DATA_VALIDATING;
                }
            }
        }
        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeProceedToCommit()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeProceedToCommit commitReq(mOutputReportLenBytes);

    // Action
    // 0x00 – Proceed  0x01 – Do not proceed 
    uint8 action = 0x00;
    commitReq.SetAction(action);

    retVal = HidDfuSendMsg(commitReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_COMMIT_REQ, STATE_UPGRADE_COMMIT_VERIFICATION);
        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeStartDataReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeStartDataReq startDataReq(mOutputReportLenBytes);

    retVal = HidDfuSendMsg(startDataReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        SetHostState(STATE_UPGRADE_DATA_TRANSFER);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeStartReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeStartReq startReq(mOutputReportLenBytes);

    retVal = HidDfuSendMsg(startReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];
        retVal = ReadResponse(pResponseBuf, UPGRADE_START_CFM, STATE_UPGRADE_DATA_READY);

        if ((retVal == HIDDFU_ERROR_NONE) && (mResumePoint == UPGRADE_RESUME_POINT_POST_REBOOT))
        {
            SetHostState(STATE_UPGRADE_COMMIT_HOST_CONTINUE);
        }

        CUpgradeStartCfm msg(pResponseBuf, mInputReportLenBytes);
        if (msg.GetUpgradeMsgSize() != 0)
        {
            if (msg.GetStatus() != 0x00)    // 0x00 – Success  0x01 – Failure
            {
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED,
                        "Status failure in UPGRADE_START_CFM");
            }

            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "UPGRADE_START_CFM status = 0x%0x", msg.GetStatus());
        }

        delete[] pResponseBuf;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeSyncReq()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeSyncReq syncReq(mOutputReportLenBytes);

    syncReq.SetFileIdentifier(FILE_ID);

    retVal = HidDfuSendMsg(syncReq);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        uint8 *pResponseBuf = new uint8[mInputReportLenBytes];

        retVal = ReadResponse(pResponseBuf, UPGRADE_SYNC_CFM, STATE_UPGRADE_READY);

        CUpgradeSyncCfm msg(pResponseBuf, mInputReportLenBytes);
        if(msg.GetUpgradeMsgSize() != 0)
        {
            mResumePoint = static_cast<UpgradeResumePoint>(msg.GetResumePoint());

            // Compare file identifier
            if (FILE_ID != msg.GetFileIdentifier())
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "UPGRADE_START_CFM Expected File Id = 0x%0x, received File Id =0x%x", FILE_ID, msg.GetFileIdentifier());

                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED,
                    "Unexpected File Identifier in UPGRADE_SYNC_CFM");
            }

            if (((mResumePoint == UPGRADE_RESUME_POINT_START)
                    || (mResumePoint == UPGRADE_RESUME_POINT_POST_REBOOT))
                    && (retVal == HIDDFU_ERROR_NONE))
            {
                SetHostState(STATE_UPGRADE_READY);
            }
            else
            {
                SetHostState(STATE_UPGRADE_IDLE);
                retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UPGRADE_FAILED,
                        "Unexpected resume point in UPGRADE_SYNC_CFM");
            }

            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "UPGRADE_START_CFM mResumePoint = 0x%0x", mResumePoint);
        }

        delete[] pResponseBuf;

    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::SendUpgradeTransferCompleteResp()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    CUpgradeTransferCompleteRes transferCmpltRsp(mOutputReportLenBytes);

    // Action
    // 0x00 – Proceed  0x01 – Do not proceed 
    uint8 action = 0x00;
    transferCmpltRsp.SetAction(action);

    retVal = HidDfuSendMsg(transferCmpltRsp);

    if(retVal == HIDDFU_ERROR_NONE)
    {
        SetHostState(STATE_UPGRADE_IDLE);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

void CHidDfuDeviceApplication::SetHostState(HostUpgradeState aNewHostState)
{
    FUNCTION_DEBUG_SENTRY;

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, 
            "New HostState: 0x%x  Old HostState: 0x%x ", aNewHostState, mHostState);
    mHostState = aNewHostState;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDeviceApplication::WriteToDevice(const uint8* apBuffer, uint32 aLength)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    // Not supported
    retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_UNSUPPORTED);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
