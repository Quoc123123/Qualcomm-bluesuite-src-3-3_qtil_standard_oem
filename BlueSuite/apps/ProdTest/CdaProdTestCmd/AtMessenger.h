//**************************************************************************************************
//
//  AtMessenger.h
//
//  Copyright (c) 2021-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  AT messenger class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef AT_MESSENGER_H
#define AT_MESSENGER_H

#include "common/types.h"
#include "PtException.h"
#include <vector>

class CPtSerialPort;
class CPtUi;

///
/// AT messenger error exception class.
///
class CAtMsgrErrorException : public CPtException
{
public:
    ///
    /// Constructor.
    /// @param[in] aMessage The exception message.
    ///
    explicit CAtMsgrErrorException(const std::string& aMessage) : CPtException(aMessage) {};
};

///
/// AT messenger command error exception class.
///
class CAtMsgrCmdErrorException : public CAtMsgrErrorException
{
public:
    ///
    /// Constructor.
    /// @param[in] aMessage The exception message.
    ///
    explicit CAtMsgrCmdErrorException(const std::string& aMessage) : CAtMsgrErrorException(aMessage) {};
};

///
/// AT messenger class
///
class CAtMessenger
{
public:
    ///
    /// Default read block length used for transaction processing.
    ///
    static const size_t DEFAULT_TRANS_READ_BLOCK_LEN = 64;

    ///
    /// Constructor.
    /// @param[in] aSerialPort Serial port object.
    /// @param[in] aNewline Newline sequence for message termination (cannot be empty).
    /// @param[in] aExpectCmdEcho Whether a command echo line is expected in the response.
    /// @throws CAtMsgrErrorException.
    ///
    CAtMessenger(CPtSerialPort& aSerialPort, const std::string& aNewline,
        bool aExpectCmdEcho);

    ///
    /// Destructor.
    ///
    virtual ~CAtMessenger();

    ///
    /// Performs an AT command transaction, checking the
    /// response for result, and returning any other response data.
    /// @param[in] aCommand The command to write.
    /// @param[in] aTimeoutMs The total read timeout in milliseconds (if set to 0 only one read is performed).
    /// @return Any additional lines in the response preceeding the result (result line and any command echo removed).
    /// @throws CAtMsgrErrorException, CAtMsgrCmdErrorException.
    ///
    std::vector<std::string> Transact(const std::string& aCommand,
        uint32 aTimeoutMs);

    ///
    /// Writes to the device, terminating the message with the newline characters.
    /// @param[in] aMessage The message to write.
    ///
    void Write(const std::string& aMessage);

    ///
    /// Reads from the device.
    /// @param[in] aMaxLen Maximum characters to read (before trimming).
    /// @param[in] aTrimWhitespace Whether leading and trailing whitespace is trimmed.
    /// @return The characters read from the device.
    /// @throws CAtMsgrErrorException.
    ///
    std::string Read(size_t aMaxLen, bool aTrimWhitespace);

    ///
    /// Sets the read block length used for transaction handling. Only needs to be
    /// set if something specific is required, otherwise DEFAULT_TRANS_READ_BLOCK_LEN
    /// will be used.
    /// @param[in] aLen Read block length (number of characters).
    ///
    void SetTransReadBlockLen(size_t aLen);

private:

    ///
    /// The serial port object.
    ///
    CPtSerialPort& mSerialPort;

    ///
    /// User interfacing object.
    ///
    CPtUi& mUi;

    ///
    /// Newline sequence used for message termination.
    ///
    std::string mNewline;

    ///
    /// Whether a command echo line is expected in the response.
    ///
    bool mExpectCmdEcho;

    ///
    /// Read block length used when processing transactions.
    ///
    size_t mTransReadBlockLen;

    ///
    /// Data buffer used to hold any data read from the device but not yet
    /// read by the client (e.g. start of a line captured during transaction
    /// processing).
    ///
    std::string mReadBuffer;
};

#endif // AT_MESSENGER_H
