//*******************************************************************************
//
//  HidDfuErrorMsg.cpp
//
//  Copyright (c) 2013-2018 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CHidDfuErrorMsg utility class implementation.
//
//*******************************************************************************

#include "HidDfuErrorMsg.h"
#include "HidDfu.h"

#include <assert.h>

#include "engine/enginefw_interface.h"

#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_HID_DFU_LIB

// Use this to simplify the code populating the map of default messages
typedef std::pair<int32, std::string> IntStringPair;

/////////////////////////////////////////////////////////////////////////////

CHidDfuErrorMsg::CHidDfuErrorMsg() 
    : 
    mErrorCode(HIDDFU_ERROR_NONE)
{ 
    FUNCTION_DEBUG_SENTRY;

    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_NONE, "")); // No error, no message
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_SEQUENCE, "Invalid sequence (hidDfuConnect must be called first)"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_CONNECTION, "Failed to connect to device"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_FILE_OPEN_FAILED, "Failed to open file"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_FILE_WRITE_FAILED, "Failed to write to file"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_FILE_INVALID_FORMAT, "File format is invalid"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_FILE_CRC_INCORRECT, "File CRC is incorrect"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_FILE_READ_FAILED, "File read failed"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_UPGRADE_FAILED, "Upgrade failed"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_RESET_FAILED, "Device reset failed"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_OUT_OF_MEM, "Out of memory"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_INVALID_PARAMETER, "Invalid parameter"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_DRIVER_INTERFACE_FAILURE, "Driver operation failed"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_OPERATION_FAILED_TO_START, "Operation failed to start"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_BUSY, "Busy, operation in progress"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_CLEAR_STATUS_FAILED, "Failed to clear device status"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_DEVICE_FIRMWARE, "Device firmware is corrupt"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_UNSUPPORTED, "Unsupported API"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_OPERATION_PARTIAL_SUCCESS, "Failure on one or more device(s)"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_UNKNOWN, "Unknown error"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_NO_OP_IN_PROGRESS, "No operation in progress"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_NO_RESPONSE, "Device is not responding (reset the device and retry)"));
    mDefaultMessages.insert(IntStringPair(HIDDFU_ERROR_OP_PARTIAL_SUCCESS_NO_RESPONSE, "One of the devices is not responding (reset the devices and retry)"));
}

/////////////////////////////////////////////////////////////////////////////

int32 CHidDfuErrorMsg::SetDefaultMsg(int32 aErrorCode)
{
    CriticalSection::Lock here(mMessageLock);

    // Default to empty string in case we don't find a mapping
    mMessage.clear();

    // Set the error
    mErrorCode = aErrorCode;

    // Set from the mapping
    std::map<int32, std::string>::const_iterator iPair = mDefaultMessages.find(aErrorCode);
    if(iPair != mDefaultMessages.end())
    {
        mMessage = iPair->second;
    }
    else
    {
        assert(false);

        mMessage = "Unknown error code";
    }

    // Must return the passed in error code unchanged. Only returning it to 
    // simplify the calling code a little.
    return aErrorCode;
}

/////////////////////////////////////////////////////////////////////////////

int32 CHidDfuErrorMsg::SetMsg(int32 aErrorCode, std::string aMessage)
{
    CriticalSection::Lock here(mMessageLock);

    assert(aMessage.length() > 0);

    // Set the error
    mErrorCode = aErrorCode;
    mMessage = aMessage;
    MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "ERROR[%d]: %s", mErrorCode, mMessage.c_str());

    // Must return the passed in error code unchanged. Only returning it to 
    // simplify the calling code a little.
    return aErrorCode;
}

/////////////////////////////////////////////////////////////////////////////

std::string& CHidDfuErrorMsg::Get()
{
    CriticalSection::Lock here(mMessageLock);

    return mMessage;
}

/////////////////////////////////////////////////////////////////////////////

int32 CHidDfuErrorMsg::GetErrorCode()
{
    CriticalSection::Lock here(mMessageLock);

    return mErrorCode;
}

/////////////////////////////////////////////////////////////////////////////
