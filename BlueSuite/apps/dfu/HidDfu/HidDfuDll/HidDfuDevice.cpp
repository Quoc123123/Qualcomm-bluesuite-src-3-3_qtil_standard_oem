//*******************************************************************************
//
//  HidDfuDevice.cpp
//
//  Copyright (c) 2014-2019 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Implementation for CHidDfuDevice class, an instance of the class represents
//  a single HidDfu device with a matching VID and PID.
//
//*******************************************************************************

#include "HidDfu.h"
#include "HidDfuDevice.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

CHidDfuDevice::CHidDfuDevice()
    :
    mDeviceHandle(INVALID_HANDLE_VALUE),
    mResetAfter(false),
    mProgress(0),
    mpThreadFunc(NULL)
{
    FUNCTION_DEBUG_SENTRY;
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDevice::CHidDfuDevice(HANDLE aDeviceHandle)
    :
    mDeviceHandle(aDeviceHandle),
    mResetAfter(false),
    mProgress(0),
    mpThreadFunc(NULL)
{
    FUNCTION_DEBUG_SENTRY;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mDeviceHandle: %d", mDeviceHandle);
}

////////////////////////////////////////////////////////////////////////////////

CHidDfuDevice::~CHidDfuDevice()
{
    FUNCTION_DEBUG_SENTRY;
    // Close Device Handle
    DisconnectDevice();
}

////////////////////////////////////////////////////////////////////////////////

bool CHidDfuDevice::Connected() const
{
    return (mDeviceHandle != INVALID_HANDLE_VALUE);
}

////////////////////////////////////////////////////////////////////////////////

bool CHidDfuDevice::CheckKeepGoing()
{
    bool retVal = true;
    FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

    if (mpThreadFunc != NULL)
    {
        retVal = KeepGoing();
    }
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
void CHidDfuDevice::ResetThreadFuncPointer()
{
    FUNCTION_DEBUG_SENTRY;
    mpThreadFunc = NULL;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::DeviceBackup(uint8 aResetAfter)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (IsActive())
    {
        // An operation is currently running, can't start another one until it completes
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
    }
    else
    {
        mpThreadFunc = &CHidDfuDevice::DoDeviceBackup;
        SetProgress(0);
        mResetAfter = aResetAfter;

        // Start the backup thread
        if (!Start())
        {
            retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_OPERATION_FAILED_TO_START);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::DeviceStop(uint16 aWaitForStopMs)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (IsActive())
    {
        Stop();

        if (!WaitForStop(aWaitForStopMs))
        {
            retVal = mLastDevError.SetMsg(HIDDFU_ERROR_UNKNOWN, "Failed to stop the operation");
        }
    }
    else
    {
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_NO_OP_IN_PROGRESS);
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::DeviceUpgrade(uint8 aResetAfter)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (IsActive())
    {
        // An operation is currently running, can't start another one until it completes
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
    }
    else
    {
        mpThreadFunc = &CHidDfuDevice::DoDeviceUpgrade;
        SetProgress(0);
        mResetAfter = aResetAfter;

        // Start the upgrade thread
        if (!Start())
        {
            retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_OPERATION_FAILED_TO_START);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::DisconnectDevice()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "mDeviceHandle: %d", mDeviceHandle);
    if (mDeviceHandle != INVALID_HANDLE_VALUE)
    {
        if (mLastDevError.GetErrorCode() != HIDDFU_ERROR_NO_RESPONSE)
        {
            CloseHandle(mDeviceHandle);
        }
        mDeviceHandle = INVALID_HANDLE_VALUE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::GetDevResult()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    if (mpThreadFunc == NULL)
    {
        // No operation has been run
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_SEQUENCE);
    }
    else if (IsActive())
    {
        // Only set the error code to BUSY if there was previously no error,
        // otherwise do not over write any other error code with BUSY
        if (mLastDevError.GetErrorCode() == HIDDFU_ERROR_NONE)
        {
            // Operation still running
            retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
        }
    }
    else
    {
        // Get the exit code of the operation thread
        retVal = GetExitCode();
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

const char* CHidDfuDevice::GetLastError()
{
    return mLastDevError.Get().c_str();
}

////////////////////////////////////////////////////////////////////////////////

uint8 CHidDfuDevice::GetProgress()
{
    CriticalSection::Lock here(mProgressLock);

    // 100% if not running an operation
    return (IsActive() ? mProgress : 100);
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::IsThreadActive()
{
    int32 retVal = HIDDFU_ERROR_NONE;
    if (IsActive())
    {
        // An operation is currently running, don't want to reset while that's happening
        retVal = mLastDevError.SetDefaultMsg(HIDDFU_ERROR_BUSY);
    }
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

int32 CHidDfuDevice::SetLastErrorFromWinError(int32 aHidErrorCode, const std::string &aErrorStr, uint32 aWinErrorCode)
{
    int32 retVal = HIDDFU_ERROR_NONE;
    FUNCTION_DEBUG_SENTRY_RET(int32, retVal);

    char *pError = "";
    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        aWinErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&pError,
        0,
        NULL))
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "%s, error: 0x%x", aErrorStr.c_str(), aWinErrorCode);
    }

    // Remove '\r' from end of string
    char *pos = strchr(pError, '\r');
    if (pos != NULL)
    {
        *pos = '\0';
    }

    std::ostringstream msg;
    msg << aErrorStr << " - " << pError;
    retVal = mLastDevError.SetMsg(aHidErrorCode, msg.str());

    LocalFree(pError);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

void CHidDfuDevice::SetProgress(uint8 aProgress)
{
    CriticalSection::Lock here(mProgressLock);

    assert(aProgress <= 100);

    mProgress = ((aProgress < 100) ? aProgress : 100);
}

////////////////////////////////////////////////////////////////////////////////

int CHidDfuDevice::ThreadFunc()
{
    return (this->*(mpThreadFunc))();
}

////////////////////////////////////////////////////////////////////////////////
