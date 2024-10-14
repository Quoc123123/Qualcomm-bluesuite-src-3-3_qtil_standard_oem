//**************************************************************************************************
//
//  PtBdAddrMgr.cpp
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Bluetooth address manager class definitions, part of an example application for production test.
//
//**************************************************************************************************

#include "PtBdAddrMgr.h"
#include "PtException.h"
#include "PtUtil.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <assert.h>
#include <regex>

using namespace QTIL;
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// CPtBdAddrRecord
////////////////////////////////////////////////////////////////////////////////

CPtBdAddrRecord::CPtBdAddrRecord(const string& aRecordPath, bool aReadOnly)
    : mRecordPath(aRecordPath), mReadOnly(aReadOnly)
{
    if (mReadOnly)
    {
        // Open file for read
        mpRecFile = new fstream(mRecordPath, ios_base::in);
        if (mpRecFile == NULL || !mpRecFile->good())
        {
            ostringstream msg;
            msg << "Failed to open Bluetooth address record file \"" << mRecordPath << "\" for read";
            throw CPtException(msg.str());
        }
    }
    else
    {
        // Open file for read/write(append)
        mpRecFile = new fstream(mRecordPath, ios_base::in | ios_base::out | ios_base::app);
        if (mpRecFile == NULL || !mpRecFile->good())
        {
            ostringstream msg;
            msg << "Failed to open Bluetooth address record file \"" << mRecordPath << "\" for read/write";
            throw CPtException(msg.str());
        }
    }

    Read();
}

////////////////////////////////////////////////////////////////////////////////

CPtBdAddrRecord::~CPtBdAddrRecord()
{
    delete mpRecFile;
}

////////////////////////////////////////////////////////////////////////////////

void CPtBdAddrRecord::Update(const string& aSerialNum, const string& aBdAddr)
{
    if (mReadOnly)
    {
        throw CPtException("Record file was opened as read only, can't update");
    }
    else if (aSerialNum.empty())
    {
        throw CPtException("Invalid (empty) serial number");
    }

    string bdAddress;
    try
    {
        bdAddress = CPtBdAddrMgr::CheckBdAddr(aBdAddr);
    }
    catch (CPtException &ex)
    {
        ostringstream msg;
        msg << "Invalid BT address \"" << aBdAddr << "\" (" << ex.what() << ")";
        throw CPtException(msg.str());
    }

    // On first write, write a header explaining the contents
    mpRecFile->seekp(0, ios::end);
    if (mpRecFile->tellp() == streampos(0))
    {
        *mpRecFile << "# Record of Bluetooth address assigned to each device" << endl
            << "# <SN> = <BDADDR>" << endl << endl;
    }

    *mpRecFile << aSerialNum << " = " << bdAddress << endl;

    // Keep the internal record in sync
    mAddrMap[aSerialNum] = bdAddress;
}

////////////////////////////////////////////////////////////////////////////////

std::string CPtBdAddrRecord::GetAddress(const std::string& aSerialNum) const
{
    if (aSerialNum.empty())
    {
        throw CPtException("Invalid (empty) serial number");
    }

    string addr;

    map<string, string>::const_iterator iRec = mAddrMap.find(aSerialNum);
    if (iRec != mAddrMap.end())
    {
        addr = iRec->second;
    }

    return addr;
}

////////////////////////////////////////////////////////////////////////////////

void CPtBdAddrRecord::Read()
{
    static const regex REGEXP_RECORD_LINE("^\\s*(\\S+)\\s*=\\s*(\\S+)\\s*$");

    string line;

    while (!mpRecFile->eof())
    {
        getline(*mpRecFile, line);

        if (!line.empty() && line.at(0) != '#') // Skip blank and comment lines
        {
            cmatch match;
            if (regex_match(line.c_str(), match, REGEXP_RECORD_LINE))
            {
                try
                {
                    mAddrMap[match.str(1)] = CPtBdAddrMgr::CheckBdAddr(match.str(2));
                }
                catch(CPtException &ex)
                {
                    ostringstream msg;
                    msg << "Error in record \"" << line << "\" in \""
                        << mRecordPath << "\" (" << ex.what() << ")";
                    throw CPtException(msg.str());
                }
            }
            else
            {
                ostringstream msg;
                msg << "Invalid BT address record \"" << line << "\" in \""
                    << mRecordPath << "\"";
                throw CPtException(msg.str());
            }
        }
    }

    // Clear EOF flag otherwise subsequent file operations will fail
    mpRecFile->clear();

    if (mReadOnly && mAddrMap.empty())
    {
        ostringstream msg;
        msg << "No BT address records found in \"" << mRecordPath << "\"";
        throw CPtException(msg.str());
    }
}


////////////////////////////////////////////////////////////////////////////////
// CPtBdAddrMgr
////////////////////////////////////////////////////////////////////////////////

string CPtBdAddrMgr::CheckBdAddr(const string& aBdAddr)
{
    // Remove optional "0x" hex prefix if present.
    string bdAddress = aBdAddr;
    string::size_type pos = bdAddress.find("0x");
    if (pos == 0)
    {
        bdAddress.erase(0, 2);
    }

    // Check format, 12 hex digits
    if (bdAddress.size() != 12 || !all_of(bdAddress.begin(), bdAddress.end(), isxdigit))
    {
        throw CPtException("Bluetooth address must be 12 hexadecimal digits");
    }

    return bdAddress;
}

////////////////////////////////////////////////////////////////////////////////

CPtBdAddrMgr::CPtBdAddrMgr(const string& aDbPath, uint16 aIncrement,
                           const string& aRecordPath)
    : mDbPath(aDbPath), mIncrement(aIncrement), mRecord(aRecordPath, false)
{
    if (aIncrement == 0)
    {
        throw CPtException("Address increment must be > 0");
    }
}

////////////////////////////////////////////////////////////////////////////////

string CPtBdAddrMgr::GetNext()
{
    if (mDbPath.empty())
    {
        throw CPtException("Bluetooth address file has not been specified");
    }

    mNextAddr.clear();
    mLastAddr.clear();

    ifstream dbFile(mDbPath);
    if (!dbFile.good())
    {
        ostringstream msg;
        msg << "Failed to open Bluetooth address file \"" << mDbPath << "\" for read";
        throw CPtException(msg.str());
    }

    while (!dbFile.eof())
    {
        //
        // Expected format of file is:
        // 
        // NEXT=<bdaddr>
        // LAST=<bdaddr>
        //
        // Where:
        //      bdaddr is a string of 12 hex digits, optional "0x" prefix allowed.
        //      The first 6 digits (the NAP and UAP parts) for NEXT and LAST must match.
        //      For the last 6 digits (the LAP), the LAST value must be greater than
        //      the next value, otherwise it means all addresses have been used.
        //      Whitespace is tollerated in the lines.
        //      Comment lines can be present, starting with '#'.
        //      Blank lines are ignored.
        //

        string line;
        getline(dbFile, line);
        if (!line.empty() && line.at(0) != '#') // Skip blank and comment lines
        {
            // Remove any spaces
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

            // Line could be empty after space removal
            if (!line.empty())
            {
                vector<string> strings = PtUtil::SplitString(line.c_str(), "=");

                bool invalidContent = false;
                if (strings.size() != 2)
                {
                    invalidContent = true;
                }
                else
                {
                    if (strings.at(0) == "NEXT")
                    {
                        mNextAddr = CheckBdAddr(strings.at(1));
                    }
                    else if (strings.at(0) == "LAST")
                    {
                        mLastAddr = CheckBdAddr(strings.at(1));
                    }
                    else
                    {
                        invalidContent = true;
                    }
                }

                if (invalidContent)
                {
                    ostringstream msg;
                    msg << "Invalid content (\"" << line << "\") in Bluetooth address file \""
                        << mDbPath << "\"";
                    throw CPtException(msg.str());
                }
            }
        }
    }

    if (mNextAddr.empty() || mLastAddr.empty())
    {
        ostringstream msg;
        msg << "Missing \"NEXT\" or \"LAST\" entries in Bluetooth address file \""
            << mDbPath << "\"";
        throw CPtException(msg.str());
    }

    // Check NAP and UAP parts match
    if (mNextAddr.substr(0, 6) != mLastAddr.substr(0, 6))
    {
        ostringstream msg;
        msg << "NAP and/or UAP (first 6 digits) for LAST and NEXT must match in Bluetooth address file \""
            << mDbPath << "\"";
        throw CPtException(msg.str());
    }

    // Check LAPs, next <= last
    if (GetLap(mNextAddr) > GetLap(mLastAddr))
    {
        ostringstream msg;
        msg << "No available addresses in Bluetooth address file \""
            << mDbPath << "\"";
        throw CPtException(msg.str());
    }

    return mNextAddr;
}

////////////////////////////////////////////////////////////////////////////////

void CPtBdAddrMgr::MarkUsed() const
{
    if (mDbPath.empty())
    {
        throw CPtException("Bluetooth address file has not been specified");
    }
    else if (mNextAddr.empty())
    {
        throw CPtException("GetNext must be called first");
    }

    // Open for overwrite
    ofstream dbFile(mDbPath);
    if (!dbFile.good())
    {
        ostringstream msg;
        msg << "Failed to open Bluetooth address file \"" << mDbPath << "\" for write";
        throw CPtException(msg.str());
    }

    // Increment the LAP
    uint32 nextLap = GetLap(mNextAddr);
    nextLap += mIncrement;

    dbFile << "NEXT=" << mNextAddr.substr(0, 6)
           << hex << setw(6) << setfill('0') << nextLap << endl;
    dbFile << "LAST=" << mLastAddr << endl;
}

////////////////////////////////////////////////////////////////////////////////

void CPtBdAddrMgr::UpdateRecord(const string& aSerialNum, const string& aBdAddr)
{
    mRecord.Update(aSerialNum, aBdAddr);
}

////////////////////////////////////////////////////////////////////////////////

uint32 CPtBdAddrMgr::GetLap(const string& aBdAddr) const
{
    uint32 lap;
    istringstream lapStr(aBdAddr.substr(6));
    lapStr >> hex >> lap;

    return lap;
}
