//**************************************************************************************************
//
//  Dut.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  DUT base class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef DUT_H
#define DUT_H

#include "PtUi.h"
#include "PtException.h"
#include <string>

class CPtSerialNum;
class CPtSetup;

///
/// DUT exception class.
///
class CDutException : public CPtException
{
public:
    ///
    /// Constructor.
    /// @param[in] aMessage The exception message.
    ///
    explicit CDutException(const std::string& aMessage) : CPtException(aMessage) {};
};


///
/// DUT base class
///
class CDut
{
public:

    ///
    /// Supported reset modes.
    ///
    enum class ResetMode
    {
        WAIT,     //!< Wait for device to come out of reset
        NOWAIT    //!< Do not wait for device to come out of reset
    };

    ///
    /// Destructor.
    ///
    virtual ~CDut() {};

    ///
    /// Connects to the device.
    /// @throws CDutException.
    ///
    virtual void Connect() = 0;

    ///
    /// Disconnects from the device.
    ///
    virtual void Disconnect() = 0;

    ///
    /// Resets the device.
    /// @param[in] aMode The reset mode.
    /// @throws CDutException.
    ///
    virtual void Reset(ResetMode aMode) = 0;

    ///
    /// Performs any necessary actions prior to running tests.
    /// @throws CDutException.
    ///
    virtual void PreTestActions() = 0;

    ///
    /// Performs any necessary actions after all tests have been run.
    /// @param[in] aTestsPassed true if all tests passed, false otherwise.
    /// @throws CDutException.
    ///
    virtual void PostTestActions(bool aTestsPassed) = 0;

    ///
    /// Sets the serial number.
    /// @param[in] apSerialNum The serial number.
    ///
    virtual void SetSerialNum(const CPtSerialNum* apSerialNum) { mpSerialNumber = apSerialNum; };

    ///
    /// Gets the serial number.
    /// @return The serial number.
    ///
    const CPtSerialNum* GetSerialNum() const { return mpSerialNumber; };

    ///
    /// Sets the device address (e.g. BT address).
    /// @param[in] aDevAddress The device address.
    ///
    virtual void SetAddress(const std::string& aDevAddress) { mDeviceAddress = aDevAddress; };

    ///
    /// Gets the device address (e.g. BT address).
    /// @return The serial number as a string.
    ///
    const std::string& GetAddress() const { return mDeviceAddress; };

protected:

    ///
    /// Constructor.
    /// @param[in] aSetup Production test setup object.
    ///
    explicit CDut(const CPtSetup& aSetup) : mSetup(aSetup), mUi(CPtUi::Ref()), mpSerialNumber(NULL) {};

    ///
    /// Setup object.
    ///
    const CPtSetup& mSetup;

    ///
    /// User interfacing object.
    ///
    CPtUi& mUi;

    ///
    /// DUT serial number
    ///
    const CPtSerialNum* mpSerialNumber;

    ///
    /// DUT address (e.g. BT address)
    ///
    std::string mDeviceAddress;
};

#endif // DUT_H
