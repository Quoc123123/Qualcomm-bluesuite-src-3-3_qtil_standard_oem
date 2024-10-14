//**************************************************************************************************
//
//  GpibInterface.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class definition for a General Purpose Interface Bus (GPIB) interface, part of an example
//  application for production test.
//
//**************************************************************************************************

#include "GpibInterface.h"
#ifndef NOGPIB
#include "ni4882.h"
#endif
#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const char CGpibInterface::mErrorMnemonicLookup[29][5] =
    {
        "EDVR", "ECIC", "ENOL", "EADR", "EARG",
        "ESAC", "EABO", "ENEB", "EDMA", "",
        "EOIP", "ECAP", "EFSO", "",     "EBUS",
        "ESTB", "ESRQ", "",     "",      "",
        "ETAB", "ELCK", "EARM", "EHDL",  "",
        "",     "EWIP", "ERST", "EPWR"
    };

////////////////////////////////////////////////////////////////////////////////

CGpibInterface::CGpibInterface()
: mDevHandle(0), mUi(CPtUi::Ref())
{
    mReadBuffer[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////////

CGpibInterface::~CGpibInterface()
{
    if (mDevHandle != 0)
    {
#ifndef NOGPIB
        // Close device
        ibonl(mDevHandle, 0);
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef NOGPIB
// Disable C4100 (Unreferenced formal parameter) in this configuration
#pragma warning( disable : 4100 )
#endif

void CGpibInterface::Open(int32 aPrimaryAddress)
{
#ifndef NOGPIB
    // Open a handle to the device
    #define BDINDEX               0     // Board Index
    #define NO_SECONDARY_ADDR     0     // Secondary address of device
    #define TIMEOUT               T10s  // Timeout value = 10 seconds
    #define EOTMODE               1     // Enable the END message
    #define EOSMODE               0     // Disable the EOS mode

    mDevHandle = ibdev(BDINDEX, aPrimaryAddress, NO_SECONDARY_ADDR,
                       TIMEOUT, EOTMODE, EOSMODE);
    if (Ibsta() & ERR)
    {
        ostringstream msg;
        msg << "Unable to open GPIB device with primary address " << aPrimaryAddress;
        ErrorCleanup(msg.str().c_str());
    }

    // Clear the internal or device functions of the device
    ibclr(mDevHandle);
    if (Ibsta() & ERR)
    {
        ErrorCleanup("Unable to clear GPIB device");
    }
#else
    throw CGpibException("GPIB not implemented");
#endif
}

////////////////////////////////////////////////////////////////////////////////

void CGpibInterface::Write(const string& aCommand)
{
#ifndef NOGPIB
    // Write the command
    ibwrt(mDevHandle, aCommand.c_str(), aCommand.size());
    if (Ibsta() & ERR)
    {
        ostringstream msg;
        msg << "Failed to write GPIB command \"" << aCommand << "\"" << endl;
        ErrorCleanup(msg.str().c_str());
    }
#else
    throw CGpibException("GPIB not implemented");
#endif
}

////////////////////////////////////////////////////////////////////////////////

std::string CGpibInterface::Read()
{
#ifndef NOGPIB
    // Read from device
    ibrd(mDevHandle, mReadBuffer, READ_BUFFER_SIZE);
    if (Ibsta() & ERR)
    {
        ErrorCleanup("Failed to read data from GPIB device");
    }

    // Terminate the response string
    mReadBuffer[Ibcnt()] = '\0';

    return string(mReadBuffer);
#else
    throw CGpibException("GPIB not implemented");
#endif
}

////////////////////////////////////////////////////////////////////////////////

#ifndef NOGPIB
void CGpibInterface::ErrorCleanup(const char* apErrorMsg)
{
    // Add the interface status code and error mnemonic to the message
    ostringstream msg;
    msg << apErrorMsg << " (nibsta = " << Ibsta()
        << " iberr = " << Iberr() << mErrorMnemonicLookup[Iberr()] << ")";

    if (mDevHandle != -1)
    {
        mUi.Write("GPIB cleanup: Taking device offline", false);
        // Close device
        ibonl(mDevHandle, 0);
    }

    throw CGpibException(msg.str());
}
#endif
