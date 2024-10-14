//**************************************************************************************************
//
//  PtSerialPort.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Serial port class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "PtSerialPort.h"
#include "PtUi.h"
#include "..\PtUtil.h"
#include <sstream>

using namespace std;
using namespace QTIL;

////////////////////////////////////////////////////////////////////////////////

CPtSerialPort::CPtSerialPort(const std::string& aPort, uint32 aBaudRate)
  : mUi(CPtUi::Ref()), mPortHandle(INVALID_HANDLE_VALUE), mMaxRxLen(0)
{
    ostringstream portNameStr;
    // Add prefix to "COM<n>" to allow ports enumerated > 9 to be opened,
    // if it looks like it isn't specified already
    if (aPort.at(0) == 'c' || aPort.at(0) == 'C')
    {
        portNameStr << "\\\\.\\";
    }
    portNameStr << aPort;

    // Open the port
    mPortHandle = ::CreateFile(portNameStr.str().c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (mPortHandle == INVALID_HANDLE_VALUE)
    {
        ostringstream msg;
        msg << "Failed to open COM port \"" << portNameStr.str() << "\"";
        throw CPtSerialPortException(msg.str());
    }

    // Configure the port
    DCB dcb = { sizeof(DCB) };
    dcb.BaudRate = aBaudRate;
    dcb.fBinary = 1; // Must be TRUE
    // fDtrControl must be enabled for the charger PCB USB virtual COM port,
    // otherwise we can't read from the DUT. It's harmless for other 3 wire
    // UARTs as the line will not be connected.
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fNull = TRUE; // TRUE to discard null (0x00) characters, which cause string handling problems.
    if (::SetCommState(mPortHandle, &dcb) == FALSE)
    {
        ostringstream msg;
        msg << "Failed to configure COM port ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }

    // Set the timeouts
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (::SetCommTimeouts(mPortHandle, &timeouts) == FALSE)
    {
        ostringstream msg;
        msg << "Failed to set COM port timeouts ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }

    // Get the RX buffer size
    COMMPROP commProp = {};
    if (::GetCommProperties(mPortHandle, &commProp) == FALSE)
    {
        ostringstream msg;
        msg << "Failed to get COM port properties ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }
    mMaxRxLen = commProp.dwCurrentRxQueue;

    ostringstream msg;
    msg << "Opened serial port \"" << portNameStr.str() << "\"";
    mUi.Write(msg.str(), false);
}

////////////////////////////////////////////////////////////////////////////////

CPtSerialPort::~CPtSerialPort()
{
    if (mPortHandle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mPortHandle);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CPtSerialPort::Write(const string& aMessage)
{
    if (aMessage.empty())
    {
        throw CPtSerialPortException("Message to write is empty");
    }

    DWORD written;
    if (::WriteFile(mPortHandle, &aMessage[0],
        static_cast<DWORD>(aMessage.size()), &written, NULL) == FALSE)
    {
        ostringstream msg;
        msg << "Failure writing to COM port ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }
    else if (written != aMessage.size())
    {
        throw CPtSerialPortException("Number of characters written to COM port does not equal the number requested");
    }
}

////////////////////////////////////////////////////////////////////////////////

string CPtSerialPort::Read(size_t aMaxLen)
{
    if (aMaxLen == 0)
    {
        throw CPtSerialPortException("Asked to read nothing");
    }
    else if (aMaxLen > mMaxRxLen)
    {
        ostringstream msg;
        msg << "Maximum length given (" << aMaxLen 
            << ") exceeds port RX buffer size (" << mMaxRxLen << ")";
        throw CPtSerialPortException(msg.str());
    }

    DWORD read;
    char* pInBuff = new char[aMaxLen];
    if (pInBuff == NULL)
    {
        throw CPtSerialPortException("Failed to allocate buffer for read");
    }

    if (::ReadFile(mPortHandle, pInBuff, static_cast<DWORD>(aMaxLen),
        &read, NULL) == FALSE)
    {
        delete[] pInBuff;

        ostringstream msg;
        msg << "Failed to read from COM port (" << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }

    string readStr(pInBuff, read);
    
    delete[] pInBuff;

    return readStr;
}

////////////////////////////////////////////////////////////////////////////////

void CPtSerialPort::Purge()
{
    if (::PurgeComm(mPortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR) == FALSE)
    {
        ostringstream msg;
        msg << "Failed to purge COM port (" << PtUtil::GetLastWinErrorMessage() << ")";
        throw CPtSerialPortException(msg.str());
    }
}

