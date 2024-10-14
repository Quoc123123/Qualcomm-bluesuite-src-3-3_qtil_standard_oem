//**************************************************************************************************
//
//  PtSerialNum.cpp
//
//  Copyright (c) 2021-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Serial number class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "PtSerialNum.h"
#include "PtException.h"
#include "PtSetup.h"
#include "PtUtil.h"
#include <sstream>
#include <algorithm>

using namespace QTIL;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

const CPtSerialNum::SnTypeMap CPtSerialNum::mSupportedTypesMap = {
    { "DEC", SnType::DEC },
    { "HEX", SnType::HEX },
    { "ALNUM", SnType::ALNUM }
};

////////////////////////////////////////////////////////////////////////////////

CPtSerialNum::CPtSerialNum(const std::string& aSerNum, const CPtSetup& aSetup)
    : mType(SnType::DEC), mSerNum(aSerNum)
{
    // Validate the serial number against the requirements in the setup
    CheckLength(aSetup);
    CheckFormat(aSetup);
}

////////////////////////////////////////////////////////////////////////////////

void CPtSerialNum::CheckLength(const CPtSetup& aSetup)
{
    static const string SN_LEN_SETTING = "SerNumLen";

    // Optional setting, if not specified, any non-zero length allowed
    string snLenStr = aSetup.GetValue(SN_LEN_SETTING, false);
    if (!snLenStr.empty())
    {
        uint16 snReqLen = aSetup.GetValueNum<uint16>(SN_LEN_SETTING);
        if (snReqLen == 0)
        {
            ostringstream msg;
            msg << "\"" << SN_LEN_SETTING << "\" value must be greater than zero";
            throw CPtException(msg.str());
        }
        else if (mSerNum.length() != snReqLen)
        {
            ostringstream msg;
            msg << "Serial number length (" << mSerNum.length()
                << ") does not match required length (" << snReqLen
                << ") specified by the \"" << SN_LEN_SETTING << "\" setting";
            throw CPtException(msg.str());
        }
    }
    else if (mSerNum.empty())
    {
        throw CPtException("Serial number cannot be zero length");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtSerialNum::CheckFormat(const CPtSetup& aSetup)
{
    static const string SN_FORMAT_SETTING = "SerNumFormat";

    // Optional setting, if not specified the default type (DEC) is used.
    string snFormatStr = aSetup.GetValue(SN_FORMAT_SETTING, false);
    if (!snFormatStr.empty())
    {
        // Check the given format is valid
        PtUtil::ToUpper(snFormatStr);
        SnTypeMap::const_iterator it = mSupportedTypesMap.find(snFormatStr);
        if (it == mSupportedTypesMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << SN_FORMAT_SETTING
                << "\" contains invalid format value \"" << snFormatStr << "\"";
            throw CPtException(msg.str());
        }
        else
        {
            mType = it->second;
        }
    }

    // Check the serial number format against the type
    bool valid = false;
    if (mType == SnType::DEC)
    {
        valid = all_of(mSerNum.begin(), mSerNum.end(), ::isdigit);
    }
    else if (mType == SnType::HEX)
    {
        valid = all_of(mSerNum.begin(), mSerNum.end(), ::isxdigit);
    }
    else if (mType == SnType::ALNUM)
    {
        valid = all_of(mSerNum.begin(), mSerNum.end(), ::isalnum);
    }

    if (!valid)
    {
        ostringstream msg;
        msg << "Serial number \"" << mSerNum
            << "\" does not match the required format (" << snFormatStr
            << ") specified by the \"" << SN_FORMAT_SETTING << "\" configuration setting";
        throw CPtException(msg.str());
    }
}