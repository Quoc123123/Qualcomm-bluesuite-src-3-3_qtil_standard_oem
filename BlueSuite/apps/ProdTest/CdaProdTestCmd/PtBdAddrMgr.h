//**************************************************************************************************
//
//  PtBdAddrMgr.h
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Bluetooth address manager class declarations, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_BD_ADDR_MGR_H
#define PT_BD_ADDR_MGR_H

#include "common\types.h"
#include <string>
#include <fstream>
#include <map>

///
/// Production test Bluetooth address record class.
///
class CPtBdAddrRecord
{
public:
    ///
    /// Constructor.
    /// @param[in] aRecordPath Path to the record file.
    /// @param[in] aReadOnly true to open the record for read only, false otherwise.
    /// @throws CPtException
    ///
    CPtBdAddrRecord(const std::string& aRecordPath, bool aReadOnly);

    ///
    /// Destructor.
    ///
    ~CPtBdAddrRecord();

    ///
    /// Gets the record file path.
    /// @return Record file path.
    ///
    std::string GetRecordFile() const { return mRecordPath; }

    ///
    /// Updates the record file with the serial number to Bluetooth address
    /// mapping.
    /// Can only be used when not in read-only mode.
    /// @param[in] aSerialNum Device serial number.
    /// @param[in] aBdAddr Bluetooth address.
    /// @throws CPtException
    ///
    void Update(const std::string& aSerialNum, const std::string& aBdAddr);

    ///
    /// Gets the Bluetooth address for a serial number from the record.
    /// @param[in] aSerialNum Device serial number.
    /// @return The Bluetooth address if found, otherwise an empty string.
    /// @throws CPtException
    ///
    std::string GetAddress(const std::string& aSerialNum) const;

private:
    ///
    /// Copy constructor.
    ///
    CPtBdAddrRecord(const CPtBdAddrRecord&);

    ///
    /// Assignment operator.
    ///
    CPtBdAddrRecord& operator=(const CPtBdAddrRecord&);

    ///
    /// Reads the record file.
    /// @throws CPtException
    ///
    void Read();

    ///
    /// Path of the Bluetooth address record file.
    ///
    std::string mRecordPath;

    ///
    /// Whether the record is read only or writable.
    ///
    bool mReadOnly;

    ///
    /// Record file stream.
    ///
    std::fstream* mpRecFile;

    ///
    /// Map of serial numbers to Bluetooth addresses
    ///
    std::map<std::string, std::string> mAddrMap;
};


///
/// Production test Bluetooth address manager class.
///
class CPtBdAddrMgr
{
public:
    ///
    /// Checks that a Bluetooth address is in the defined format
    /// of 12 contiguous hexadecimal digits.
    /// @param[in] aBdAddr The address string to check
    /// @return Valid Bluetooth address without "0x" prefix
    /// @throws CPtException.
    ///
    static std::string CheckBdAddr(const std::string& aBdAddr);

    ///
    /// Constructor.
    /// @param[in] aDbPath Path to the Bluetooth address database.
    ///   Can be blank if a database file isn't to be used (just using to record address).
    /// @param[in] aIncrement Increment to apply to the next address
    ///   when an address has been used.
    /// @param[in] aRecordPath Path to the record file.
    /// @throws CPtException.
    ///
    CPtBdAddrMgr(const std::string& aDbPath, uint16 aIncrement, const std::string& aRecordPath);

    ///
    /// Destructor.
    ///
    ~CPtBdAddrMgr() {};

    ///
    /// Get the next Bluetooth address.
    /// Does not update the database, i.e. repeated calls will
    /// return the same address.
    /// @return Bluetooth address as a string of 12 hex digits.
    /// @throws CPtException.
    ///
    std::string GetNext();

    ///
    /// Increments the database with the Bluetooth address to be used next.
    /// Must follow a call to GetNext.
    /// @throws CPtException.
    ///
    void MarkUsed() const;

    ///
    /// Updates the record file with the serial number to Bluetooth address
    /// mapping.
    /// @param[in] aSerialNum Device serial number.
    /// @param[in] aBdAddr Bluetooth address.
    /// @throws CPtException
    ///
    void UpdateRecord(const std::string& aSerialNum, const std::string& aBdAddr);

private:
    ///
    /// Gets the LAP part of a Bluetooth address as a number.
    /// @param[in] aBdAddr A Bluetooth address string (12 hex digits).
    /// @return The LAP value.
    ///
    uint32 GetLap(const std::string& aBdAddr) const;

    ///
    /// Path of the Bluetooth address database.
    ///
    std::string mDbPath;

    ///
    /// Increment to use when updating for the next address.
    ///
    uint16 mIncrement;

    ///
    /// Record object.
    ///
    CPtBdAddrRecord mRecord;

    ///
    /// The next BT address
    ///
    std::string mNextAddr;

    ///
    /// The last BT address
    ///
    std::string mLastAddr;
};

#endif // PT_BD_ADDR_MGR_H
