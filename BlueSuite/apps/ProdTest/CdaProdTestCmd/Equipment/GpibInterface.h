//**************************************************************************************************
//
//  GpibInterface.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Class declaration for a General Purpose Interface Bus (GPIB) interface, part of an example
//  application for production test.
//
//**************************************************************************************************

#ifndef GPIB_INTERFACE_H
#define GPIB_INTERFACE_H

#include "common/types.h"
#include "PtException.h"
#include "PtUi.h"
#include <string>

///
/// GPIB interface exception class
///
class CGpibException : public CPtException
{
public:
    ///
    /// Constructor
    /// @param[in] aMessage The exception message.
    ///
    explicit CGpibException(const std::string& aMessage) : CPtException(aMessage) {};
};

///
/// GPIB interface class
///
class CGpibInterface
{
public:
    ///
    /// Constructor
    ///
    CGpibInterface();

    ///
    /// Destructor
    ///
    ~CGpibInterface();

    ///
    /// Opens a connection.
    /// @param[in] aPrimaryAddress Primary GPIB address.
    /// @throws CGpibException.
    ///
    void Open(int32 aPrimaryAddress);

    ///
    /// Sends a command to a connected device.
    /// @param[in] aCommand GPIB command to write.
    /// @throws CGpibException.
    ///
    void Write(const std::string& aCommand);

    ///
    /// Reads from a connected device.
    /// @return The string read from the device.
    /// @throws CGpibException.
    ///
    std::string Read();

private:

    ///
    /// Size of the buffer used for reads.
    ///
    static const size_t READ_BUFFER_SIZE = 1024;

    ///
    /// Lookup table mapping GPIB error codes to error strings.
    ///
    static const char mErrorMnemonicLookup[29][5];

    ///
    /// Buffer for reads (+1 for termination).
    ///
    char mReadBuffer[READ_BUFFER_SIZE + 1];

    ///
    /// Handle to an instrument.
    ///
    uint32 mDevHandle;

    ///
    /// Test interface object.
    ///
    CPtUi& mUi;

#ifndef NOGPIB
    ///
    /// Cleans up the connection in the event of error and throws.
    /// @param[in] apErrorMsg Error message.
    /// @throws CGpibException.
    ///
    void ErrorCleanup(const char* apErrorMsg);
#endif
};

#endif // GPIB_INTERFACE_H
