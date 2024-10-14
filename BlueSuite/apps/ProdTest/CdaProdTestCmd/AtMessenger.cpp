//**************************************************************************************************
//
//  AtMessenger.cpp
//
//  Copyright (c) 2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  AT messenger class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "AtMessenger.h"
#include "PtUtil.h"
#include "PtUi.h"
#include <PtSerialPort.h>

#include <sstream>
#include <algorithm>
#include <regex>
#include <chrono>
#include <thread>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

CAtMessenger::CAtMessenger(CPtSerialPort& aSerialPort,
    const std::string& aNewline, bool aExpectCmdEcho)
    : mSerialPort(aSerialPort), mUi(CPtUi::Ref()), mNewline(aNewline),
      mExpectCmdEcho(aExpectCmdEcho),
      mTransReadBlockLen(DEFAULT_TRANS_READ_BLOCK_LEN)
{
    if (mNewline.empty())
    {
        throw CAtMsgrErrorException("Newline string is invalid (empty)");
    }
}

////////////////////////////////////////////////////////////////////////////////

CAtMessenger::~CAtMessenger()
{
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CAtMessenger::Transact(const std::string& aCommand,
    uint32 aTimeoutMs)
{
    const string AT_ERROR_SIG = "ERROR" + mNewline;
    const string AT_OK_SIG = "OK" + mNewline;

    Write(aCommand);

    // Read until we get a response indicating the transaction is complete, or
    // until we hit the transaction timeout.
    bool transactionComplete = false;
    bool transactionOK = false;
    vector<string> allLines;
    string readStr;
    time_point<steady_clock> startTime = steady_clock::now();
    do
    {
        readStr += Read(mTransReadBlockLen, false);

        string::size_type pos;
        if ((pos = readStr.find(AT_OK_SIG)) != string::npos)
        {
            transactionOK = true;
            transactionComplete = true;
            // Discard result
            readStr.erase(pos, AT_OK_SIG.size());
        }
        else if ((pos = readStr.find(AT_ERROR_SIG)) != string::npos)
        {
            transactionComplete = true;
            // Discard result
            readStr.erase(pos, AT_ERROR_SIG.size());
        }

        if (transactionComplete)
        {
            // Retain any trailing characters for the next read
            mReadBuffer += readStr.substr(pos);

            readStr.erase(pos);
        }

        if (!transactionComplete &&
            duration_cast<milliseconds>(steady_clock::now() - startTime) < milliseconds(aTimeoutMs))
        {
            this_thread::sleep_for(milliseconds(100));
        }
    } while (!transactionComplete &&
        duration_cast<milliseconds>(steady_clock::now() - startTime) < milliseconds(aTimeoutMs));

    if (!readStr.empty())
    {
        allLines = PtUtil::SplitString(readStr.c_str(), mNewline);

        // Remove empty lines
        auto strIsEmpty = [](const string& aStr) { return aStr.empty(); };
        allLines.erase(std::remove_if(allLines.begin(), allLines.end(), strIsEmpty), allLines.end());
    }

    if (!transactionComplete && allLines.empty())
    {
        throw CAtMsgrErrorException("No response received from the DUT");
    }
    else if (!transactionComplete)
    {
        throw CAtMsgrErrorException("AT command response incomplete (expecting \"OK\" or \"ERROR\" result)");
    }
    else if (!transactionOK)
    {
        // Using specific exception type for this, to allow trapping in order to provide a more user
        // friendly error message depending on the command sent.
        throw CAtMsgrCmdErrorException("Device response indicates failure (expected \"OK\" result)");
    }

    if (mExpectCmdEcho)
    {
        // Find the line ending with the command echo (could have leading data we don't care about)
        vector<string>::const_iterator iCommandEcho =
            find_if(allLines.begin(), allLines.end(),
                [aCommand](string aStr) -> bool { return aStr.size() >= aCommand.size() && aStr.compare(aStr.size() - aCommand.size(), aCommand.size(), aCommand) == 0; });
        if (iCommandEcho == allLines.end())
        {
            throw CAtMsgrErrorException("DUT response does not contain expected command");
        }
        else
        {
            // Remove any command echo and anything before it
            allLines.erase(allLines.begin(), iCommandEcho + 1);
        }
    }

    return allLines;
}

////////////////////////////////////////////////////////////////////////////////

void CAtMessenger::Write(const std::string& aMessage)
{
    // Terminate the message
    mSerialPort.Write(aMessage + mNewline);

    ostringstream logMsg;
    logMsg << "TX: " << aMessage;
    mUi.Write(logMsg.str(), false);
}

////////////////////////////////////////////////////////////////////////////////

std::string CAtMessenger::Read(size_t aMaxLen, bool aTrimWhitespace)
{
    if (aMaxLen == 0)
    {
        throw CAtMsgrErrorException("Request to read nothing");
    }

    // We have some left overs after a transaction, these should preceed
    // any further data read from the device.
    string msg;
    size_t devReadCount = (aMaxLen > mReadBuffer.size() ? aMaxLen - mReadBuffer.size() : 0);
    if (devReadCount > 0)
    {
        string newMsg = mSerialPort.Read(devReadCount);
        if (!newMsg.empty())
        {
            // Log the message, replacing newline sequence with a single "\n" to
            // make the message more compact and easier to read.
            ostringstream logMsg;
            logMsg << "RX: " << regex_replace(newMsg, regex(mNewline), "\\n");
            mUi.Write(logMsg.str(), false);
        }
        mReadBuffer += newMsg;
    }

    size_t returnCount = min(aMaxLen, mReadBuffer.size());
    msg.assign(mReadBuffer, 0, returnCount);
    mReadBuffer.erase(0, returnCount);
    if (aTrimWhitespace)
    {
        PtUtil::TrimString(msg);
    }

    return msg;
}

////////////////////////////////////////////////////////////////////////////////

void CAtMessenger::SetTransReadBlockLen(size_t aLen)
{
    mTransReadBlockLen = aLen;
}
