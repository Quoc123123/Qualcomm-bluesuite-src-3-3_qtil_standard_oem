//**************************************************************************************************
//
//  PtException.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Exception base class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_EXCEPTION_H
#define PT_EXCEPTION_H

#include <string>

///
/// Production test exception base class
///
class CPtException : public std::exception
{
public:
    ///
    /// Constructor
    /// @param[in] aMessage The exception message.
    ///
    explicit CPtException(const std::string& aMessage) : exception(aMessage.c_str()) {};
};

#endif // PT_EXCEPTION_H
