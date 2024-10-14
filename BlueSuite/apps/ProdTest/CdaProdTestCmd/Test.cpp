//**************************************************************************************************
//
//  Test.cpp
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  ProdTest test class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "Test.h"
#include "PtSetup.h"
#include "PtStation.h"
#include "..\PtUtil.h"
#include <sstream>

using namespace std;
using namespace QTIL;

////////////////////////////////////////////////////////////////////////////////
// CTest
////////////////////////////////////////////////////////////////////////////////

CTest::CTest(const std::string& aName, const CStation& aStation)
    : mName(aName), mStation(aStation), mUi(CPtUi::Ref())
{
}

////////////////////////////////////////////////////////////////////////////////

void CTest::WriteLog(const std::string& aMessage, bool aMandatory)
{
    ostringstream msg;
    msg << mName << ": " << aMessage;
    mUi.Write(msg.str(), aMandatory);
}

////////////////////////////////////////////////////////////////////////////////

void CTest::WriteLogNoPrefix(const std::string& aMessage, bool aMandatory)
{
    mUi.Write(aMessage, aMandatory);
}

////////////////////////////////////////////////////////////////////////////////

void CTest::WriteUpdateMsg(const std::string& aMessage)
{
    ostringstream msg;
    msg << mName << ": " << aMessage;
    mUi.UpdateLine(msg.str());

}

////////////////////////////////////////////////////////////////////////////////

void CTest::ThrowError(const std::string& aMessage)
{
    ostringstream msg;
    msg << mName << ": " << aMessage;
    throw CPtException(msg.str());
}

////////////////////////////////////////////////////////////////////////////////

void CTest::AskUserHitKey(const std::string& aMessage)
{
    ostringstream msg;
    msg << mName << ": " << aMessage;
    mUi.AskHitKey(msg.str());
}

////////////////////////////////////////////////////////////////////////////////

bool CTest::AskUserCheck(const std::string& aMessage)
{
    ostringstream msg;
    msg << mName << ": " << aMessage;
    return mUi.AskForCheck(msg.str());
}

////////////////////////////////////////////////////////////////////////////////

bool CTest::ReportResult(bool aPass)
{
    ostringstream msg;
    msg << mName;
    if (aPass)
    {
        msg << " PASSED";
    }
    else
    {
        msg << " FAILED";
    }

    mUi.WriteStatus(msg.str(), aPass);

    return aPass;
}

////////////////////////////////////////////////////////////////////////////////

const CStation & CTest::GetStation() const
{
    return mStation;
}

////////////////////////////////////////////////////////////////////////////////

const CPtSetup& CTest::GetSetup() const
{
    return mStation.GetSetup();
}

////////////////////////////////////////////////////////////////////////////////

template void CTest::GetLimits(const std::string& aName, uint16& aLowLimit, uint16& aHighLimit);
template void CTest::GetLimits(const std::string& aName, int16& aLowLimit, int16& aHighLimit);

template<typename T>
void CTest::GetLimits(const std::string& aName, T& aLowLimit, T& aHighLimit)
{
    vector<string> limits = GetSetup().GetValueList(aName, true);
    if (limits.size() != 2)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aName
            << "\" should be a list of 2 values";
        ThrowError(msg.str());
    }

    PtUtil::TrimString(limits.at(0));
    PtUtil::TrimString(limits.at(1));

    istringstream lowStr(limits.at(0));
    istringstream highStr(limits.at(1));
    if ((numeric_limits<T>::min() == 0 && (limits.at(0).at(0) == '-' || limits.at(1).at(0) == '-')) ||
        !(lowStr >> aLowLimit) || !lowStr.eof() ||
        !(highStr >> aHighLimit) || !highStr.eof())
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aName
            << "\" contains invalid values";
        ThrowError(msg.str());
    }

    if (aLowLimit > aHighLimit)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aName
            << "\" contains invalid values (low limit cannot be greater than the high limit)";
        ThrowError(msg.str());
    }
}
