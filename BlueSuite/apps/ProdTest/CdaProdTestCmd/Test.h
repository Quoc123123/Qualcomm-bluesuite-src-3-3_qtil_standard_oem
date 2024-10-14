//**************************************************************************************************
//
//  Test.h
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  ProdTest test class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef TEST_H
#define TEST_H

#include "common\types.h"
#include "PtUi.h"
#include <string>

class CStation;
class CPtSetup;


///
/// Test base class
///
class CTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CTest() {};

    ///
    /// Allows post-connection, pre-execution checks (those which require a DUT
    /// connection).
    /// NOTE: Configuration checks not requiring a connection should be performed
    /// during construction.
    /// 
    virtual void PostConnectionCheck() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    virtual bool Execute() = 0;

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    ///
    CTest(const std::string& aName, const CStation& aStation);

    ///
    /// Writes a log message.
    /// @param[in] aMessage Message to write.
    /// @param[in] aMandatory true for a mandatory message (shown even when in quiet mode), false otherwise.
    ///
    void WriteLog(const std::string& aMessage, bool aMandatory);

    ///
    /// Writes a log message without the test name prefix.
    /// @param[in] aMessage Message to write.
    /// @param[in] aMandatory true for a mandatory message (shown even when in quiet mode), false otherwise.
    ///
    void WriteLogNoPrefix(const std::string& aMessage, bool aMandatory);

    ///
    /// Writes an updatable message to the user interface (does not terminate with a newline).
    /// Useful for progress reporting where the value changes within the same message.
    /// @param[in] aMessage The message.
    ///
    void WriteUpdateMsg(const std::string& aMessage);

    ///
    /// Raises an error (throws an exception).
    /// @param[in] aMessage The error message.
    /// @throws CPtException.
    ///
    void ThrowError(const std::string& aMessage);

    ///
    /// Asks user to do something and hit a key
    /// @param[in] aMessage The message.
    ///
    void AskUserHitKey(const std::string& aMessage);

    ///
    /// Asks user to check something and report pass/fail
    /// @param[in] aMessage The message.
    /// @return true if the user indicates check ok (pass), otherwise false.
    ///
    bool AskUserCheck(const std::string& aMessage);

    ///
    /// Reports the result of the test.
    /// @param[in] aPass true if the test passed, false otherwise.
    /// @return The value of aPass.
    ///
    bool ReportResult(bool aPass);

    ///
    /// Get the station object.
    /// @return Station object.
    ///
    const CStation& GetStation() const;

    ///
    /// Get the station setup object.
    /// @return Station setup object.
    ///
    const CPtSetup& GetSetup() const;

    ///
    /// Gets and validates limit setting values.
    /// Used for settings which are comma-separated lists of 2 values:
    ///   <lowLimit>,<highLimit>.
    /// @param[in] aName The name of the setting providing the limits.
    /// @param[out] aLowLimit The low limit.
    /// @param[out] aHighLimit The low limit.
    /// @throws CPtException.
    ///
    template<typename T>
    void GetLimits(const std::string& aName, T& aLowLimit, T& aHighLimit);

    ///
    /// The test name;
    ///
    const std::string mName;

    ///
    /// The production test station object.
    ///
    const CStation& mStation;

private:
    ///
    /// Copy constructor.
    ///
    CTest(const CTest&);

    ///
    /// Assignment operator.
    ///
    CTest& operator=(const CTest&);

    ///
    /// The user interface object.
    /// Private as derived classes should use the protected functions for UI interaction.
    ///
    CPtUi& mUi;
};

#endif // TEST_H
