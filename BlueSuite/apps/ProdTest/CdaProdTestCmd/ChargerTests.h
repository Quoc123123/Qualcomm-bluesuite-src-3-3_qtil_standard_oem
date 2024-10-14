//**************************************************************************************************
//
//  ChargerTests.h
//
//  Copyright (c) 2020-2023 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Charger ProdTest test class declarations, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CHARGER_TESTS_H
#define CHARGER_TESTS_H

#include "Test.h"
#include "common\types.h"
#include "PtSetup.h"
#include "ChargerDevice.h"
#include <string>
#include <vector>

///
/// Charger test factory
///
class CChargerTestFactory
{
public:

    ///
    /// Gets whether a named test is supported or not.
    /// @param[in] aName Name of the test.
    /// @return true if the test is supported, false otherwise.
    ///
    static bool TestSupported(const std::string& aName);

    ///
    /// Creates supported test objects based on the station configuration.
    /// @param aStation The test station object.
    /// @return Test objects.
    /// @throws CPtException.
    ///
    static std::vector<CTest*> CreateTests(const CStation& aStation);

private:

    ///
    /// Constructor.
    ///
    CChargerTestFactory();

    ///
    /// The supported test names.
    ///
    static const std::vector<std::string> mSupportedTests;
};


///
/// Charger device test base class
///
class CChargerTest : public CTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CChargerTest() {};

    ///
    /// Creates supported test objects based on the station configuration.
    /// @param aStation The test station object.
    /// @return Test objects.
    /// @throws CPtException.
    ///
    static std::vector<CTest*> CreateTests(const CStation& aStation);

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerTest(const std::string& aName, const CStation& aStation, CChargerDevice& aDut);

    ///
    /// The charger device object.
    ///
    CChargerDevice& mChargerDevice;
};


///
/// LED test class
///
class CChargerLedTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerLedTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerLedTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    typedef std::pair<std::string, CChargerDevice::LedMode> LedModePair;
    typedef std::map<LedModePair::first_type, LedModePair::second_type> LedModeMap;

    ///
    /// LED modes map, mapping mode strings to enumerated mode values.
    ///
    static const LedModeMap mSupportedLedModeMap;

    ///
    /// The LED modes to test
    ///
    std::vector<LedModePair> mLedModes;
};


///
/// Set device serial number test class
///
class CChargerSetSnTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerSetSnTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerSetSnTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Check device serial number test class
///
class CChargerCheckSnTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerCheckSnTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerCheckSnTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// VBAT test class
///
class CChargerVbattTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerVbattTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerVbattTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Voltage to test at in milli-volts.
    ///
    uint16 mTestMv;

    ///
    /// Test margin in milli-volts.
    ///
    uint16 mTestMarginMv;

    ///
    /// Whether the user will be asked to set the VBAT voltage.
    ///
    bool mUserSet;
};


///
/// Earbud comms test class
///
class CChargerBudCommsTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerBudCommsTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerBudCommsTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// VBUS test class
///
class CChargerVbusTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerVbusTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerVbusTest() {};

    ///
    /// Perform post-connection, pre-execution checks (those which require a DUT
    /// connection).
    /// 
    void PostConnectionCheck() override;

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    typedef std::pair<std::string, CChargerDevice::RegLevel> RegLevelPair;
    typedef std::map<RegLevelPair::first_type, RegLevelPair::second_type> RegLevelMap;

    ///
    /// Regulator levels map, mapping strings to enumerated values.
    ///
    static const RegLevelMap mSupportedRegLevelMap;

    ///
    /// VBUS test data
    ///
    struct VbusTest
    {
        VbusTest() : enable(false), level(CChargerDevice::RegLevel::HIGH),
                     lowLimitMv(0), highLimitMv(0), cCommsPullUpEnable(false) {};

        std::string levelStr; //!< The level
        bool enable; //!< Whether the regulator should be enabled or not
        CChargerDevice::RegLevel level; //!< The regulator level
        uint16 lowLimitMv; //!< The lower voltage limit in mV
        uint16 highLimitMv; //!< The upper voltage limit in mV
        bool cCommsPullUpEnable; //!< Whether the case comms pull-up should be enabled or not
    };

    ///
    /// Gets the VBUS tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<level>:<low_limit>:<high_limit>"
    /// level is a string specifying the regulator level, one of: "OFF", "RESET", "LOW", or "HIGH".
    /// low_limit specifies the lower pass limit for the VBUS voltage in mV.
    /// high_limit specifies the upper pass limit for the VBUS voltage in mV.
    /// For example "OFF:0:100, RESET:400:600, LOW:3900:4100, HIGH:4900:5100".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// VBUS tests
    ///
    std::vector<VbusTest> mTests;
};


///
/// Current sense test class
///
class CChargerCurrentSenseTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerCurrentSenseTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerCurrentSenseTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Low limit for the ADC readings.
    ///
    uint16 mLowLimit;

    ///
    /// High limit for the ADC readings.
    ///
    uint16 mHighLimit;
};


///
/// Lid sensor test class
///
class CChargerLidSensorTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerLidSensorTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerLidSensorTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Battery discharge current test class
///
class CChargerBattDischargeITest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerBattDischargeITest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerBattDischargeITest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Low current limit.
    ///
    uint16 mLowLimitMa;

    ///
    /// Low current limit.
    ///
    uint16 mHighLimitMa;
};


///
/// Battery charging test class
///
class CChargerBattChargingTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerBattChargingTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerBattChargingTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    typedef std::pair<std::string, CChargerDevice::ChargerMode> ChargeModePair;
    typedef std::map<ChargeModePair::first_type, ChargeModePair::second_type> ChargeModeMap;

    ///
    /// Charge modes map, mapping mode strings to enumerated mode values.
    ///
    static const ChargeModeMap mSupportedChargeModeMap;

    ///
    /// Charging test data
    ///
    struct BattChargeTest
    {
        BattChargeTest() : mode(CChargerDevice::ChargerMode::MODE_100_MA), lowLimitMa(0), highLimitMa(0) {};

        CChargerDevice::ChargerMode mode; //!< The charger mode
        uint16 lowLimitMa; //!< The lower current limit in mA
        uint16 highLimitMa; //!< The upper current limit in mA
    };

    ///
    /// Gets the charging tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<mode>:<low_limit>:<high_limit>"
    /// mode is a string specifying the charging mode, one of: "100MA", "500MA", "ILIM", or "STANDBY".
    /// low_limit specifies the lower pass limit for the charging current in mA.
    /// high_limit specifies the upper pass limit for the charging current in mA.
    /// For example "500MA:300:500, 100MA:60:100, ILIM:400:545, STANDBY:0:30".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// Charging tests
    ///
    std::vector<BattChargeTest> mTests;
};


///
/// Standby mode test class
///
class CChargerStandbyTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerStandbyTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerStandbyTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Low current limit (uA).
    ///
    uint16 mLowLimitUa;

    ///
    /// Low current limit (uA).
    ///
    uint16 mHighLimitUa;
};


///
/// Thermistor test class
///
class CChargerThermistorTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerThermistorTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerThermistorTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Low limit in milli-volts.
    ///
    uint16 mLowLimitMv;

    ///
    /// High limit in milli-volts.
    ///
    uint16 mHighLimitMv;
};


///
/// Set shipping mode test class
///
class CChargerSetShipModeTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerSetShipModeTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerSetShipModeTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Fetches and logs an earbud's BT device address.
    /// @param[in] aEarbud Earbud selection.
    /// @throws CPtException.
    ///
    void LogEarbudAddr(CChargerDevice::EarbudSel aEarbud);

    ///
    /// Whether the Bluetooth device addresses will be logged.
    ///
    bool mLogBdAddrs;
};


///
/// Configuration set test class
///
class CChargerCfgSetTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerCfgSetTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerCfgSetTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    typedef std::pair<std::string, std::string> CfgKeyValuePair;

    ///
    /// The configuration settings to write.
    ///
    std::vector<CfgKeyValuePair> mCfgSettings;
};


///
/// Case-bud comms TX-RX loop test class
///
class CChargerBudCommsTxRxLoopTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerBudCommsTxRxLoopTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerBudCommsTxRxLoopTest() {};

    ///
    /// Perform post-connection, pre-execution checks (those which require a DUT
    /// connection).
    /// 
    void PostConnectionCheck() override;

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// BUD-DETECT test class
///
class CChargerBudDetectTest : public CChargerTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CChargerBudDetectTest(const CStation& aStation, CChargerDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CChargerBudDetectTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Name of the setting used to define the GPIO configuration
    /// 
    static const char* const GPIOS_SETTING;

    ///
    /// Bud detection GPIO settings
    /// 
    struct DetectGpio
    {
        explicit DetectGpio(const std::string& aBudName) : earbud(aBudName), inState(CChargerDevice::GpioState::UNKNOWN) {}
        std::string earbud; //!< earbud description (e.g. LEFT or RIGHT).
        std::string gpio;   //!< GPIO specifier, e.g. "A3", meaning port A, pin index 3.
        CChargerDevice::GpioState inState;  //!< Expected GPIO state when earbud is in case.
    };

    ///
    /// GPIO settings for left bud
    /// 
    DetectGpio mGpioLeftBud;

    ///
    /// GPIO settings for right bud
    /// 
    DetectGpio mGpioRightBud;

    ///
    /// Bud detection tests
    /// 
    enum class BudsTest
    {
        None,
        LeftOnly,
        RightOnly,
        Both
    };

    typedef std::pair<std::string, BudsTest> BudTestsPair;
    typedef std::map<BudTestsPair::first_type, BudTestsPair::second_type> BudTestsMap;

    ///
    /// Bud tests map, mapping test strings to enumerated test values.
    ///
    static const BudTestsMap mSupportedBudTestsMap;

    ///
    /// The bud tests to run
    ///
    std::vector<BudsTest> mBudTests;

    ///
    /// Set the GPIO settings for an earbud.
    /// @param[in] aGpioCfgStr GPIO configuration string in the format <port><index>, e.g., "A3".
    ///   Supported ports are 'A','B' and 'C'.The index must be between 0 and 15.
    /// @param[in] aGpio The GPIO settings object to set.
    /// @throws CPtException.
    ///
    void SetGpio(const std::string& aGpioCfgStr, DetectGpio& aGpio);

    ///
    /// Check that the GPIO state is as expected for the GPIO configuration.
    /// @param[in] aGpio The GPIO settings object to check against.
    /// @param[in] aState The GPIO state.
    /// @param[in] aBudIn true if the test is with the earbud in, false if out.
    /// @return true if the state is as expected, false otherwise.
    ///
    bool GpioStateAsExpected(const DetectGpio& aGpio, CChargerDevice::GpioState aState, bool aBudIn);

    ///
    /// Get the name for the given buds test.
    /// @param[in] aTest The buds test.
    /// @return Test name.
    ///
    std::string CChargerBudDetectTest::GetTestName(BudsTest aTest);
};


#endif // CHARGER_TESTS_H
