//**************************************************************************************************
//
//  ChargerTests.cpp
//
//  Copyright (c) 2020-2023 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Charger ProdTest test class definitions, part of an example application for production test.
//
//**************************************************************************************************

#include "ChargerTests.h"
#include "PtTimer.h"
#include "PtStation.h"
#include "PtSerialNum.h"
#include "..\PtUtil.h"
#include <sstream>
#include <iomanip>
#include <regex>

using namespace QTIL;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// CCdaDevTestFactory
////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> CChargerTestFactory::mSupportedTests =
{
    CChargerLedTest::NAME,
    CChargerSetSnTest::NAME,
    CChargerCheckSnTest::NAME,
    CChargerVbattTest::NAME,
    CChargerBudCommsTest::NAME,
    CChargerVbusTest::NAME,
    CChargerCurrentSenseTest::NAME,
    CChargerLidSensorTest::NAME,
    CChargerBattDischargeITest::NAME,
    CChargerBattChargingTest::NAME,
    CChargerStandbyTest::NAME,
    CChargerThermistorTest::NAME,
    CChargerSetShipModeTest::NAME,
    CChargerCfgSetTest::NAME,
    CChargerBudCommsTxRxLoopTest::NAME,
    CChargerBudDetectTest::NAME
};

////////////////////////////////////////////////////////////////////////////////

bool CChargerTestFactory::TestSupported(const std::string& aName)
{
    return (find(mSupportedTests.begin(), mSupportedTests.end(), aName) != mSupportedTests.end());
}

////////////////////////////////////////////////////////////////////////////////

std::vector<CTest*> CChargerTestFactory::CreateTests(const CStation& aStation)
{
    // These tests are for Charger DUTs only
    CChargerDevice* pDev = dynamic_cast<CChargerDevice*>(aStation.GetDut());
    if (pDev == NULL)
    {
        throw CPtException("DUT is not supported by charger tests");
    }

    const vector<string> testNames = aStation.GetTestNames();

    vector<CTest*> tests;
    for (string testName : testNames)
    {
        PtUtil::ToUpper(testName);

        if (!TestSupported(testName))
        {
            ostringstream msg;
            msg << "Test \"" << testName << "\" is unsupported." << endl;
            msg << "Supported tests for the device type are: ";

            for (vector<string>::const_iterator iName = mSupportedTests.begin();
                iName != mSupportedTests.end();
                ++iName)
            {
                msg << *iName;
                if (iName != mSupportedTests.end() - 1)
                {
                    msg << ", ";
                }
            }

            throw CPtException(msg.str());
        }

        if (testName == CChargerLedTest::NAME)
        {
            tests.push_back(new CChargerLedTest(aStation, *pDev));
        }
        else if (testName == CChargerSetSnTest::NAME)
        {
            tests.push_back(new CChargerSetSnTest(aStation, *pDev));
        }
        else if (testName == CChargerCheckSnTest::NAME)
        {
            tests.push_back(new CChargerCheckSnTest(aStation, *pDev));
        }
        else if (testName == CChargerVbattTest::NAME)
        {
            tests.push_back(new CChargerVbattTest(aStation, *pDev));
        }
        else if (testName == CChargerBudCommsTest::NAME)
        {
            tests.push_back(new CChargerBudCommsTest(aStation, *pDev));
        }
        else if (testName == CChargerVbusTest::NAME)
        {
            tests.push_back(new CChargerVbusTest(aStation, *pDev));
        }
        else if (testName == CChargerCurrentSenseTest::NAME)
        {
            tests.push_back(new CChargerCurrentSenseTest(aStation, *pDev));
        }
        else if (testName == CChargerLidSensorTest::NAME)
        {
            tests.push_back(new CChargerLidSensorTest(aStation, *pDev));
        }
        else if (testName == CChargerBattDischargeITest::NAME)
        {
            tests.push_back(new CChargerBattDischargeITest(aStation, *pDev));
        }
        else if (testName == CChargerBattChargingTest::NAME)
        {
            tests.push_back(new CChargerBattChargingTest(aStation, *pDev));
        }
        else if (testName == CChargerStandbyTest::NAME)
        {
            tests.push_back(new CChargerStandbyTest(aStation, *pDev));
        }
        else if (testName == CChargerThermistorTest::NAME)
        {
            tests.push_back(new CChargerThermistorTest(aStation, *pDev));
        }
        else if (testName == CChargerSetShipModeTest::NAME)
        {
            tests.push_back(new CChargerSetShipModeTest(aStation, *pDev));
        }
        else if (testName == CChargerCfgSetTest::NAME)
        {
            tests.push_back(new CChargerCfgSetTest(aStation, *pDev));
        }
        else if (testName == CChargerBudCommsTxRxLoopTest::NAME)
        {
            tests.push_back(new CChargerBudCommsTxRxLoopTest(aStation, *pDev));
        }
        else if (testName == CChargerBudDetectTest::NAME)
        {
            tests.push_back(new CChargerBudDetectTest(aStation, *pDev));
        }
        else
        {
            // Shouldn't get here, already checked above if supported,
            // but if we get here we have a mismatch between the list of
            // supported tests and the test name matching cases above.
            ostringstream msg;
            msg << "Test \"" << testName << "\" is unhandled";
            throw CPtException(msg.str());
        }
    }

    return tests;
}


////////////////////////////////////////////////////////////////////////////////
// CChargerTest
////////////////////////////////////////////////////////////////////////////////

CChargerTest::CChargerTest(const std::string& aName, const CStation& aStation,
    CChargerDevice& aDut)
    : CTest(aName, aStation), mChargerDevice(aDut)
{
}


////////////////////////////////////////////////////////////////////////////////
// CChargerLedTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerLedTest::NAME = "CHG:LED";

const CChargerLedTest::LedModeMap CChargerLedTest::mSupportedLedModeMap = {
    {"OFF", CChargerDevice::LedMode::OFF},
    {"RED", CChargerDevice::LedMode::RED},
    {"GREEN", CChargerDevice::LedMode::GREEN},
    {"AMBER", CChargerDevice::LedMode::AMBER},
    {"BLUE", CChargerDevice::LedMode::BLUE},
    {"MAGENTA", CChargerDevice::LedMode::MAGENTA},
    {"CYAN", CChargerDevice::LedMode::CYAN},
    {"WHITE", CChargerDevice::LedMode::WHITE}
};

CChargerLedTest::CChargerLedTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
    static const string CHARGER_LED_MODES_SETTING = "ChgrLedModes";
    vector<string> ledModes = GetSetup().GetValueList(CHARGER_LED_MODES_SETTING, true);

    for (string mode : ledModes)
    {
        PtUtil::ToUpper(mode);
        LedModeMap::const_iterator it = mSupportedLedModeMap.find(mode);
        if (it == mSupportedLedModeMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << CHARGER_LED_MODES_SETTING
                << "\" contains invalid LED mode \"" << mode << "\"";
            ThrowError(msg.str());
        }
        else
        {
            mLedModes.push_back(*it);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerLedTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;
    for (LedModePair ledMode : mLedModes)
    {
        // Set the mode and ask for a check
        mChargerDevice.LedSet(ledMode.second);

        ostringstream msg;
        msg << "Check LED mode is " << ledMode.first;
        pass = AskUserCheck(msg.str()) && pass;

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerSetSnTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerSetSnTest::NAME = "CHG:SET-SN";

CChargerSetSnTest::CChargerSetSnTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerSetSnTest::Execute()
{
    CPtTimer testTimer(mName);

    uint64 sn = mChargerDevice.GetSerialNumber();
    if (sn == 0)
    {
        ThrowError("Serial number must be provided");
    }

    // Must be written as hex
    ostringstream value;
    value << hex << setfill('0') << setw(16) << sn;
    mChargerDevice.ConfigWriteValue("SERIAL", value.str());

    ostringstream msg;
    // Match the reporting format to the input serial number format
    if (mChargerDevice.GetSerialNum()->GetType() == CPtSerialNum::SnType::HEX)
    {
        msg << hex;
    }
    msg << "Serial number set to " << sn;
    WriteLog(msg.str(), false);

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerCheckSnTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerCheckSnTest::NAME = "CHG:CHECK-SN";

CChargerCheckSnTest::CChargerCheckSnTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerCheckSnTest::Execute()
{
    CPtTimer testTimer(mName);

    // Get the serial number provided by the user
    uint64 sn = mChargerDevice.GetSerialNumber();
    if (sn == 0)
    {
        ThrowError("Serial number must be provided");
    }

    // Read the serial number from the DUT
    string dutSnStr = mChargerDevice.ConfigReadValue("SERIAL");
    // Convert to uint64
    uint64 dutSn;
    istringstream istr(dutSnStr);
    if (!(istr >> hex >> dutSn) || !istr.eof())
    {
        ostringstream msg;
        msg << "Serial number read from DUT \"" << dutSnStr
            << "\" is invalid. Must be hex and convertable to a uint64 value";
        throw CDutException(msg.str());
    }

    bool pass = true;
    if (sn == dutSn)
    {
        ostringstream msg;
        // Match the reporting format to the input serial number format
        if (mChargerDevice.GetSerialNum()->GetType() == CPtSerialNum::SnType::HEX)
        {
            msg << hex;
        }
        msg << "DUT SN is " << dutSn;
        WriteLog(msg.str(), false);
    }
    else
    {
        pass = false;

        ostringstream msg;
        // Match the reporting format to the input serial number format
        if (mChargerDevice.GetSerialNum()->GetType() == CPtSerialNum::SnType::HEX)
        {
            msg << hex;
        }
        msg << "Provided SN (" << sn << ") does not match DUT SN (" << dutSn << ")";
        WriteLog(msg.str(), false);
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerVbattTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerVbattTest::NAME = "CHG:VBATT";

CChargerVbattTest::CChargerVbattTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
      mTestMv(GetSetup().GetValueNum<uint16>("VbattTestMv")),
      mTestMarginMv(GetSetup().GetValueNum<uint16>("VbattTestMarginMv")),
      mUserSet(GetSetup().GetValueNum<bool>("VbattTestUserSet"))
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerVbattTest::Execute()
{
    CPtTimer testTimer(mName);

    if (mUserSet)
    {
        ostringstream msg;
        msg << "Set VBAT to " << mTestMv << "mV, then press a key";
        AskUserHitKey(msg.str());
    }

    // Disable VREG
    mChargerDevice.VregSet(false, CChargerDevice::RegLevel::HIGH);

    uint16 battMv = mChargerDevice.BattVoltsGet();

    ostringstream msgMeas;
    msgMeas << "Measured VBAT = " << battMv << "mV";
    WriteLog(msgMeas.str(), false);

    bool pass = false;
    const uint16 minMv = mTestMv - mTestMarginMv;
    const uint16 maxMv = mTestMv + mTestMarginMv;
    if (battMv >= minMv && battMv <= maxMv)
    {
        pass = true;
    }
    else
    {
        ostringstream msg;
        msg << "VBAT measurement is outside of the given range ("
            << minMv << " - " << maxMv << ")";
        WriteLog(msg.str(), false);
    }

    // Enable VREG
    mChargerDevice.VregSet(true, CChargerDevice::RegLevel::HIGH);

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerBudCommsTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerBudCommsTest::NAME = "CHG:BUD-COMMS";

CChargerBudCommsTest::CChargerBudCommsTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBudCommsTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Connect both L and R earbuds, then press a key");

    const string status = mChargerDevice.EarbudStatus();
    vector<string> statuses = PtUtil::SplitString(status, ",");
    if (statuses.size() != 2)
    {
        ThrowError("Earbud status string format is invalid");
    }

    bool pass = false;
    uint16 lBatt;
    uint16 rBatt;
    istringstream lBattStr(statuses.at(0));
    istringstream rBattStr(statuses.at(1));
    if ((lBattStr >> lBatt) && lBattStr.eof() && lBatt <= 100 && 
        (rBattStr >> rBatt) && rBattStr.eof() && rBatt <= 100)
    {
        WriteLog("Earbud L battery percentage = " + lBattStr.str(), false);
        WriteLog("Earbud R battery percentage = " + rBattStr.str(), false);

        pass = true;
    }
    else
    {
        WriteLog("Comms failed for one or both earbuds", true);
    }


    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerVbusTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerVbusTest::NAME = "CHG:VBUS";

const CChargerVbusTest::RegLevelMap CChargerVbusTest::mSupportedRegLevelMap = {
    { "HIGH", CChargerDevice::RegLevel::HIGH },
    { "LOW", CChargerDevice::RegLevel::LOW },
    { "RESET", CChargerDevice::RegLevel::RESET }
};

CChargerVbusTest::CChargerVbusTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

void CChargerVbusTest::PostConnectionCheck()
{
    for (VbusTest test : mTests)
    {
        // RESET level not supported on fast comms hardware
        // CCOM test not supported unless fast comms hardware
        if ((mChargerDevice.SupportsFastComms() && test.level == CChargerDevice::RegLevel::RESET) ||
            (!mChargerDevice.SupportsFastComms() && test.cCommsPullUpEnable))
        {
            ostringstream msg;
            msg << "VBUS level value \"" << test.levelStr
                << "\" is not supported for this device";
            ThrowError(msg.str());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerVbusTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;

    for (VbusTest test : mTests)
    {
        // Set enable and level and ask for a check
        if (test.cCommsPullUpEnable)
        {
            // Special case for the case comms.
            // Enabling this disables the regulator.
            mChargerDevice.CaseCommsPullUpSet(true);
        }
        else
        {
            if (mChargerDevice.SupportsFastComms())
            {
                // Disable this to ensure we only have the voltage
                // from the regulator.
                mChargerDevice.CaseCommsPullUpSet(false);
            }

            mChargerDevice.VregSet(test.enable, test.level);
        }

        ostringstream msg;
        msg << "Check VBUS is between "
            << test.lowLimitMv << " and " << test.highLimitMv << " mV";
        pass = AskUserCheck(msg.str()) && pass;

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }

        if (test.cCommsPullUpEnable)
        {
            // Disable case comms pull up after testing
            mChargerDevice.CaseCommsPullUpSet(false);
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerVbusTest::GetTests()
{
    static const string TESTS_SETTING = "VbusTests";
    static const regex REGEXP_CHG_TEST_DEF("(.+):(\\d+):(\\d+)");

    vector<string> chgTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = chgTests.begin();
        iStr != chgTests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_CHG_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        VbusTest vbusTest;
        vbusTest.levelStr = match.str(1);

        PtUtil::ToUpper(vbusTest.levelStr);
        RegLevelMap::const_iterator iLevel = mSupportedRegLevelMap.find(vbusTest.levelStr);
        // "OFF" and "CCOM" are special cases, not specified regulator levels
        if (vbusTest.levelStr == "OFF")
        { 
            vbusTest.enable = false;
        }
        else if (vbusTest.levelStr == "CCOM")
        {
            vbusTest.enable = false;
            vbusTest.cCommsPullUpEnable = true;
        }
        else if (iLevel == mSupportedRegLevelMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << vbusTest.levelStr << "\" level is invalid";
            ThrowError(msg.str());
        }
        else
        {
            vbusTest.enable = true;
            vbusTest.level = iLevel->second;
        }

        // Convert the limit strings to values
        istringstream issLowLim(match.str(2));
        istringstream issHighLim(match.str(3));
        if (!(issLowLim >> vbusTest.lowLimitMv) || !issLowLim.eof() ||
            !(issHighLim >> vbusTest.highLimitMv) || !issHighLim.eof())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" low or high limit invalid";
            ThrowError(msg.str());
        }

        mTests.push_back(vbusTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CChargerCurrentSenseTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerCurrentSenseTest::NAME = "CHG:CUR-SENSE";

CChargerCurrentSenseTest::CChargerCurrentSenseTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
      mLowLimit(0), mHighLimit(0)
{
    GetLimits("CurSenseRange", mLowLimit, mHighLimit);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerCurrentSenseTest::Execute()
{
    CPtTimer testTimer(mName);

    // Enable VREG
    mChargerDevice.VregSet(true, CChargerDevice::RegLevel::HIGH);

    bool pass = true;
    vector<uint16> adcValues = mChargerDevice.ReadCurrentSense();
    for (size_t index = 0; index < 2; ++index)
    {
        const string earbud = (index == 0 ? "Left" : "Right");
        const uint16 adcVal = adcValues.at(index);
        if (adcVal < mLowLimit || adcVal > mHighLimit)
        {
            ostringstream msg;
            msg << earbud << " current sense ADC value " << adcVal
                << " is out of range ("<< mLowLimit << " - " << mHighLimit << ")";
            WriteLog(msg.str(), true);
            pass = false;
        }
        else
        {
            ostringstream msg;
            msg << earbud << " current sense ADC value = " << adcVal;
            WriteLog(msg.str(), false);
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerLidSensorTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerLidSensorTest::NAME = "CHG:LID-SENSOR";

CChargerLidSensorTest::CChargerLidSensorTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerLidSensorTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Ensure lid is open, then press a key");
    bool pass = mChargerDevice.LidOpen();
    if (!pass)
    {
        WriteLog("Lid open test FAILED", false);
    }

    if (pass || !GetStation().StopOnFail())
    {
        AskUserHitKey("Close the lid, then press a key");
        bool lidClosed = !mChargerDevice.LidOpen();
        if (!lidClosed)
        {
            WriteLog("Lid closed test FAILED", false);
        }
        pass = pass && lidClosed;
    }
    
    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerBattDischargeTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerBattDischargeITest::NAME = "CHG:BATT-DIS";

CChargerBattDischargeITest::CChargerBattDischargeITest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
      mLowLimitMa(0), mHighLimitMa(0)
{
    GetLimits("BattDischargeRangeMa", mLowLimitMa, mHighLimitMa);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBattDischargeITest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Connect the load onto VBUS, then press a key");

    ostringstream msg;
    msg << "Check that the current is between " << mLowLimitMa << " and "
        << mHighLimitMa << " mA";
    bool pass = AskUserCheck(msg.str());

    AskUserHitKey("Disconnect the load from VBUS, then press a key");

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerBattChargingTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerBattChargingTest::NAME = "CHG:BATT-CHARGE";

const CChargerBattChargingTest::ChargeModeMap CChargerBattChargingTest::mSupportedChargeModeMap = {
    { "100MA", CChargerDevice::ChargerMode::MODE_100_MA },
    { "500MA", CChargerDevice::ChargerMode::MODE_500_MA },
    { "ILIM", CChargerDevice::ChargerMode::MODE_I_LIM },
    { "STANDBY", CChargerDevice::ChargerMode::MODE_STANDBY }
};

CChargerBattChargingTest::CChargerBattChargingTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBattChargingTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Connect USB, then press a key");

    bool chargerDetected = mChargerDevice.ChargerConnected();
    bool pass = chargerDetected;

    if (!pass)
    {
        WriteLog("Charger not detected", false);
    }
    else
    {
        for (BattChargeTest test : mTests)
        {
            // Set the mode and ask for a check
            mChargerDevice.ChargerSet(true, test.mode);

            ostringstream msg;
            msg << "Check charger current is between "
                << test.lowLimitMa << " and " << test.highLimitMa << " mA";
            pass = AskUserCheck(msg.str()) && pass;

            if (!pass && GetStation().StopOnFail())
            {
                break;
            }
        }
    }

    AskUserHitKey("Disconnect USB, then press a key");

    if (chargerDetected && (pass || !GetStation().StopOnFail()))
    {
        chargerDetected = mChargerDevice.ChargerConnected();
        pass = pass && !chargerDetected;
        if (chargerDetected)
        {
            WriteLog("Charger still detected", false);
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerBattChargingTest::GetTests()
{
    static const string TESTS_SETTING = "BattChargeTests";
    static const regex REGEXP_CHG_TEST_DEF("(.+):(\\d+):(\\d+)");

    vector<string> chgTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = chgTests.begin();
        iStr != chgTests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_CHG_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        string mode(match.str(1));
        PtUtil::ToUpper(mode);
        ChargeModeMap::const_iterator iMode = mSupportedChargeModeMap.find(mode);
        if (iMode == mSupportedChargeModeMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" mode is invalid";
            ThrowError(msg.str());
        }
        else
        {
            BattChargeTest chgTest;
            chgTest.mode = iMode->second;

            // Convert the limit strings to values
            istringstream issLowLim(match.str(2));
            istringstream issHighLim(match.str(3));
            if (!(issLowLim >> chgTest.lowLimitMa) || !issLowLim.eof() ||
                !(issHighLim >> chgTest.highLimitMa) || !issHighLim.eof())
            {
                ostringstream msg;
                msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                    << test << "\" low or high limit invalid";
                ThrowError(msg.str());
            }

            mTests.push_back(chgTest);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// CChargerStandbyTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerStandbyTest::NAME = "CHG:STANDBY";

CChargerStandbyTest::CChargerStandbyTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
    mLowLimitUa(0), mHighLimitUa(0)
{
    GetLimits("StandbyRangeUa", mLowLimitUa, mHighLimitUa);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerStandbyTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Apply magnet (close lid), ensure charger (USB) is disconnected, then press a key");

    // Set the power mode before disabling test mode, otherwise the device
    // can enter a mode where the hardware UART can't be used, and therefore
    // doesn't respond to the power mode command.
    mChargerDevice.LowPowerSet(CChargerDevice::LowPowerMode::STANDBY);
    // Test mode has to be disabled in order to enter standby state.
    mChargerDevice.TestMode(false);

    ostringstream msg;
    msg << "Check standby current is between "
        << mLowLimitUa << " and " << mHighLimitUa << " uA";
    bool pass = AskUserCheck(msg.str());

    AskUserHitKey("Remove magnet (open lid), then press a key");

    // Enter test mode again (proves device has woken up).
    mChargerDevice.TestMode(true);

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerThermistorTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerThermistorTest::NAME = "CHG:THERM";

CChargerThermistorTest::CChargerThermistorTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
    mLowLimitMv(0), mHighLimitMv(0)
{
    GetLimits("ThermistorRangeMv", mLowLimitMv, mHighLimitMv);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerThermistorTest::Execute()
{
    CPtTimer testTimer(mName);

    uint16 thermMv = mChargerDevice.ThermistorVoltsGet();

    ostringstream msgMeas;
    msgMeas << "Measured thermistor potential divider = " << thermMv << "mV";
    WriteLog(msgMeas.str(), false);

    bool pass = false;
    if (thermMv >= mLowLimitMv && thermMv <= mHighLimitMv)
    {
        pass = true;
    }
    else
    {
        ostringstream msg;
        msg << "Thermistor potential divider measurement is outside of the given range ("
            << mLowLimitMv << " - " << mHighLimitMv << ")";
        WriteLog(msg.str(), false);
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerSetShipModeTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerSetShipModeTest::NAME = "CHG:SET-SHIPMODE";

CChargerSetShipModeTest::CChargerSetShipModeTest(const CStation& aStation,
    CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut),
      mLogBdAddrs(GetSetup().GetValueNum<bool>("ShipModeLogBdAddrs"))
{
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerSetShipModeTest::Execute()
{
    CPtTimer testTimer(mName);

    if (mLogBdAddrs)
    {
        LogEarbudAddr(CChargerDevice::EarbudSel::LEFT);
        LogEarbudAddr(CChargerDevice::EarbudSel::RIGHT);
    }

    mChargerDevice.SetShippingMode();

    return ReportResult(true);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerSetShipModeTest::LogEarbudAddr(CChargerDevice::EarbudSel aEarbud)
{
    ostringstream msg;
    msg << "Bluetooth device address for " << (aEarbud == CChargerDevice::EarbudSel::LEFT ? 'L' : 'R') 
        << " earbud: " << mChargerDevice.EarbudBtAddress(aEarbud);
    WriteLog(msg.str(), false);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerCfgSetTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerCfgSetTest::NAME = "CHG:CFG-SET";

CChargerCfgSetTest::CChargerCfgSetTest(const CStation& aStation,
    CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
    static const string CFG_ITEMS_SETTING = "CfgSetItems";
    vector<string> items = GetSetup().GetValueList(CFG_ITEMS_SETTING, true);

    for (string item : items)
    {
        vector<string> parts = PtUtil::SplitString(item, "=");
        if (parts.size() != 2)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << CFG_ITEMS_SETTING
                << "\" contains invalid item \"" << item << "\"";
            ThrowError(msg.str());
        }

        PtUtil::TrimString(parts.at(0));
        PtUtil::TrimString(parts.at(1));

        mCfgSettings.push_back(make_pair(parts.at(0), parts.at(1)));
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerCfgSetTest::Execute()
{
    CPtTimer testTimer(mName);

    for (CfgKeyValuePair setting : mCfgSettings)
    {
        mChargerDevice.ConfigWriteValue(setting.first, setting.second);
    }

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerBudCommsTxRxLoopTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerBudCommsTxRxLoopTest::NAME = "CHG:BC-TXRX";

CChargerBudCommsTxRxLoopTest::CChargerBudCommsTxRxLoopTest(
    const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

void CChargerBudCommsTxRxLoopTest::PostConnectionCheck()
{
    // Test only supported on fast comms hardware
    if (!mChargerDevice.SupportsFastComms())
    {
        ThrowError("Test is not supported for this device");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBudCommsTxRxLoopTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = mChargerDevice.TestBudCommsTxRxLoop(false);
    if (!pass)
    {
        WriteLog("TX-RX loop LOW test FAILED", false);
    }

    if (pass || !GetStation().StopOnFail())
    {
        bool highPass = mChargerDevice.TestBudCommsTxRxLoop(true);
        if (!highPass)
        {
            WriteLog("TX-RX loop HIGH test FAILED", false);
        }
        pass = pass && highPass;
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerBudDetectTest
////////////////////////////////////////////////////////////////////////////////

const char* const CChargerBudDetectTest::NAME = "CHG:BUD-DETECT";
const char* const CChargerBudDetectTest::GPIOS_SETTING = "BudDetectGpios";

const CChargerBudDetectTest::BudTestsMap CChargerBudDetectTest::mSupportedBudTestsMap = {
    {"NONE", BudsTest::None},
    {"L", BudsTest::LeftOnly},
    {"R", BudsTest::RightOnly},
    {"LR", BudsTest::Both}
};

////////////////////////////////////////////////////////////////////////////////

CChargerBudDetectTest::CChargerBudDetectTest(const CStation& aStation, CChargerDevice& aDut)
    : CChargerTest(NAME, aStation, aDut), mGpioLeftBud("LEFT"), mGpioRightBud("RIGHT")
{
    // GPIOs
    const vector<string> budGpios = GetSetup().GetValueList(GPIOS_SETTING, true);
    if (budGpios.size() != 2)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << GPIOS_SETTING
            << "\" must be a list of two items (for left and right earbuds)";
        ThrowError(msg.str());
    }

    SetGpio(budGpios.at(0), mGpioLeftBud);
    SetGpio(budGpios.at(1), mGpioRightBud);

    // Tests (comma-separated list of test specifiers)
    static const string TESTS_SETTING = "BudDetectTests";
    const vector<string> budTests = GetSetup().GetValueList(TESTS_SETTING, true);
    for (string test : budTests)
    {
        PtUtil::ToUpper(test);
        BudTestsMap::const_iterator it = mSupportedBudTestsMap.find(test);
        if (it == mSupportedBudTestsMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING
                << "\" contains invalid test mode \"" << test << "\"";
            ThrowError(msg.str());
        }
        else
        {
            mBudTests.push_back(it->second);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CChargerBudDetectTest::SetGpio(const std::string& aGpioCfgStr, DetectGpio& aGpio)
{
    // The format is:
    //  <gpio>:<in_state>, where:
    //    <gpio> is in the format <port><index>, e.g., "A3".
    //    Supported ports are 'A', 'B' and 'C'.The index must be between 0 and 15.
    //    <in_state> whether the pin is expected to be high (1) or low (0) when the earbud is present.
    // E.g.: "A7:0"
    static const regex REGEXP_PIOS_SETTING("([[:alpha:]]{1}[[:digit:]]+):([01]{1})");

    cmatch match;
    if (!regex_match(aGpioCfgStr.c_str(), match, REGEXP_PIOS_SETTING))
    {
        ostringstream msg;
        msg << "Configuration setting \"" << GPIOS_SETTING << "\", value \""
            << aGpioCfgStr << "\" is invalid";
        ThrowError(msg.str());
    }

    try
    {
        // Just for validation
        char portUnused;
        (void)CChargerDevice::ConvertGpioName(match.str(1), portUnused);
    }
    catch (CDutException& e)
    {
        ostringstream msg;
        msg << "Error in configuration setting \"" << GPIOS_SETTING << "\", value \""
            << aGpioCfgStr << "\": " << e.what();
        ThrowError(msg.str());
    }

    aGpio.gpio = match.str(1);
    aGpio.inState = (match.str(2) == "1" ? CChargerDevice::GpioState::HIGH : CChargerDevice::GpioState::LOW);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBudDetectTest::GpioStateAsExpected(const DetectGpio& aGpio,
    CChargerDevice::GpioState aState, bool aBudIn)
{
    bool asExpected = false;

    const string actualStateStr = (aState == CChargerDevice::GpioState::HIGH ? "HIGH" : "LOW");
    string expectedStateStr;
    if (aBudIn)
    {
        expectedStateStr = (aGpio.inState == CChargerDevice::GpioState::HIGH ? "HIGH" : "LOW");
    }
    else
    {
        expectedStateStr = (aGpio.inState == CChargerDevice::GpioState::HIGH ? "LOW" : "HIGH");
    }
    
    // Note that while CChargerDevice::GpioState can be low, high or unknown,
    // unknown is an exception case, so we can ignore it for determining test
    // pass or fail.
    if ((aBudIn && aState == aGpio.inState) ||
        (!aBudIn && aState != aGpio.inState))
    {
        asExpected = true;
    }
    else
    {
        ostringstream msg;
        msg << aGpio.earbud << " expected to be " << expectedStateStr << ", but is " << actualStateStr;
        WriteLog(msg.str(), true);
    }

    return asExpected;
}

////////////////////////////////////////////////////////////////////////////////

std::string CChargerBudDetectTest::GetTestName(BudsTest aTest)
{
    string testStr;
    
    for (BudTestsMap::const_iterator it = mSupportedBudTestsMap.begin();
        it != mSupportedBudTestsMap.end() && testStr.empty();
        ++it)
    {
        if (it->second == aTest)
        {
            testStr = it->first;
        }
    }

    return testStr;
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerBudDetectTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;

    CChargerDevice::GpioState gpioStateL;
    CChargerDevice::GpioState gpioStateR;
    CChargerDevice::GpioFunction gpioFuncUnused;
    bool leftBudIn;
    bool rightBudIn;

    for (BudsTest test : mBudTests)
    {
        if (test == BudsTest::None)
        {
            AskUserHitKey("Ensure NO earbuds in case, then press a key");
            leftBudIn = false;
            rightBudIn = false;
        }
        else if (test == BudsTest::LeftOnly)
        {
            AskUserHitKey("Ensure only LEFT earbud in case, then press a key");
            leftBudIn = true;
            rightBudIn = false;
        }
        else if (test == BudsTest::RightOnly)
        {
            AskUserHitKey("Ensure only RIGHT earbud in case, then press a key");
            leftBudIn = false;
            rightBudIn = true;
        }
        else // BudsTest::Both
        {
            AskUserHitKey("Ensure BOTH earbuds in case, then press a key");
            leftBudIn = true;
            rightBudIn = true;
        }

        gpioStateL = mChargerDevice.GetGpio(mGpioLeftBud.gpio, gpioFuncUnused);
        gpioStateR = mChargerDevice.GetGpio(mGpioRightBud.gpio, gpioFuncUnused);

        bool thisTestPass = GpioStateAsExpected(mGpioLeftBud, gpioStateL, leftBudIn);
        thisTestPass = GpioStateAsExpected(mGpioRightBud, gpioStateR, rightBudIn) && thisTestPass;

        ostringstream msg;
        msg << "\"" << GetTestName(test) << "\" test ";
        if (thisTestPass)
        {
            msg << "PASSED";
            WriteLog(msg.str(), true);
        }
        else
        {
            msg << "FAILED";
            WriteLog(msg.str(), true);
        }

        pass = thisTestPass && pass;

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}
