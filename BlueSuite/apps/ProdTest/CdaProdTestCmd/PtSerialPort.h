//**************************************************************************************************
//
//  PtSerialPort.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Serial Port class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_SERIAL_PORT_H
#define PT_SERIAL_PORT_H

#include "common\types.h"
#include "PtException.h"
#include <string>

class CPtUi;

///
/// Serial port exception class.
///
class CPtSerialPortException : public CPtException
{
public:
    ///
    /// Constructor.
    /// @param[in] aMessage The exception message.
    ///
    explicit CPtSerialPortException(const std::string& aMessage) : CPtException(aMessage) {};
};

///
/// Production test serial port class.
///
class CPtSerialPort
{
public:
    ///
    /// Constructor.
    /// @param[in] aPort Serial port identifier, e.g. "COM15".
    /// @param[in] aBaudRate Baud rate for the port, e.g. 115200.
    /// @throws CPtException
    ///
    CPtSerialPort(const std::string& aPort, uint32 aBaudRate);

    ///
    /// Destructor.
    ///
    ~CPtSerialPort();

    ///
    /// Writes to the port.
    /// @param[in] aMessage The characters to write.
    /// @throws CPtException
    ///
    void Write(const std::string& aMessage);

    ///
    /// Reads from the port.
    /// @param[in] aMaxLen Maximum number of characters to read.
    /// @return The characters read. If the read times out, an
    /// empty string will be returned.
    /// @throws CPtException
    ///
    std::string Read(size_t aMaxLen);

    ///
    /// Purges the TX and RX buffers.
    /// @throws CPtException.
    ///
    void Purge();

private:
    ///
    /// User interfacing object.
    ///
    CPtUi& mUi;

    ///
    /// The port (file) handle.
    ///
    HANDLE mPortHandle;

    ///
    /// The maximum read length.
    ///
    size_t mMaxRxLen;
};

#endif // PT_SERIAL_PORT_H
