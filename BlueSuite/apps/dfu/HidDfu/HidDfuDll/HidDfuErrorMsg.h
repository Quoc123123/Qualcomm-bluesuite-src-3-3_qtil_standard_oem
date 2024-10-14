//*******************************************************************************
//
//  HidDfuErrorMsg.h
//
//  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CHidDfuErrorMsg utility class (used to provide default error descriptions for 
//  HidDfu error types from any HidDfu class).
//
//*******************************************************************************

#ifndef HIDDFU_ERROR_MSG_H
#define HIDDFU_ERROR_MSG_H

#include "common/types.h"
#include "thread/critical_section.h"
#include <string>
#include <map>

///
/// Class used to deal with error messages for HidDfu error types. 
///
class CHidDfuErrorMsg
{
public:
    CHidDfuErrorMsg();
    ~CHidDfuErrorMsg() {};

    ///
    /// Sets the error message to the default for the error code. 
    /// This function should only be used for error types or situations 
    /// where a more detailed message is unnecessary. Otherwise use 
    /// the SetMsg function to provide a detailed message containing any 
    /// relevant values and parameters.
    /// Returns the passed in error code to simplify settings of 
    /// return codes and last error messages in user code (1 line).
    /// @param[in] aErrorCode Error code.
    /// @return aErrorCode parameter value.
    ///
    int32 SetDefaultMsg(int32 aErrorCode);

    ///
    /// Returns the passed in error code to simplify settings of 
    /// return codes and last error messages in user code (1 line).
    /// @param[in] aErrorCode Error code.
    /// @param[in] aMessage The message to set.
    /// @return aErrorCode parameter value.
    ///
    int32 SetMsg(int32 aErrorCode, std::string aErrorMessage);

    ///
    /// Gets the error message string.
    /// @return Error message string.
    ///
    std::string& Get();

    ///
    /// Gets the error code.
    /// @return Error code.
    ///
    int32 GetErrorCode();

private:
    CHidDfuErrorMsg& operator=(const CHidDfuErrorMsg& aRHS);
    CHidDfuErrorMsg(const CHidDfuErrorMsg& aRHS);

    /// Map of error codes to default error messages
    std::map<int32, std::string> mDefaultMessages;

    /// The error code
    int32 mErrorCode;

    /// The error message
    std::string mMessage;

    /// Lock for multi-thread use
    CriticalSection mMessageLock;
};

#endif // #ifndef HIDDFU_ERROR_MSG_H