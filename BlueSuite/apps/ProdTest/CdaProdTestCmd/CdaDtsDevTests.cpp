//**************************************************************************************************
//
//  CdaDtsDevTests.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA DTS device test class definitions, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "CdaDtsDevTests.h"
#include "PtTimer.h"
#include "PtSetup.h"
#include "PtStation.h"
#include "PtUtil.h"
#include "Equipment\SpectrumAnalyserInstrument.h"
#include <sstream>
#include <iomanip>
#include <regex>
#include <chrono>
#include <thread>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////
// CCdaDtsDevTestFactory
////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> CCdaDtsDevTestFactory::mSupportedTests =
{
    CCdaDtsLedTest::NAME,
    CCdaDtsAudioToneTest::NAME,
    CCdaDtsAudioLoopTest::NAME,
    CCdaDtsTouchpadTest::NAME,
    CCdaDtsProximityTest::NAME,
    CCdaDtsHallSensorTest::NAME,
    CCdaDtsTemperatureTest::NAME,
    CCdaDtsBatteryTest::NAME,
    CCdaDtsRssiTest::NAME,
    CCdaDtsRfPowerTest::NAME,
    CCdaDtsCfgSetTest::NAME
};

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsDevTestFactory::TestSupported(const std::string& aName)
{
    return (find(mSupportedTests.begin(), mSupportedTests.end(), aName) != mSupportedTests.end());
}

////////////////////////////////////////////////////////////////////////////////

std::vector<CTest*> CCdaDtsDevTestFactory::CreateTests(const CStation& aStation)
{
    // These tests are for CdaDtsDevice DUTs only
    CCdaDtsDevice* pDev = dynamic_cast<CCdaDtsDevice*>(aStation.GetDut());
    if (pDev == NULL)
    {
        throw CPtException("DUT is not supported by CDA device DTS tests");
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

        if (testName == CCdaDtsLedTest::NAME)
        {
            tests.push_back(new CCdaDtsLedTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsAudioToneTest::NAME)
        {
            tests.push_back(new CCdaDtsAudioToneTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsAudioLoopTest::NAME)
        {
            tests.push_back(new CCdaDtsAudioLoopTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsTouchpadTest::NAME)
        {
            tests.push_back(new CCdaDtsTouchpadTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsProximityTest::NAME)
        {
            tests.push_back(new CCdaDtsProximityTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsHallSensorTest::NAME)
        {
            tests.push_back(new CCdaDtsHallSensorTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsTemperatureTest::NAME)
        {
            tests.push_back(new CCdaDtsTemperatureTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsBatteryTest::NAME)
        {
            tests.push_back(new CCdaDtsBatteryTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsRssiTest::NAME)
        {
            tests.push_back(new CCdaDtsRssiTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsRfPowerTest::NAME)
        {
            tests.push_back(new CCdaDtsRfPowerTest(aStation, *pDev));
        }
        else if (testName == CCdaDtsCfgSetTest::NAME)
        {
            tests.push_back(new CCdaDtsCfgSetTest(aStation, *pDev));
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
// CCdaDtsDevTest
////////////////////////////////////////////////////////////////////////////////

CCdaDtsDevTest::CCdaDtsDevTest(const std::string& aName, const CStation& aStation,
    CCdaDtsDevice& aDut)
    : CTest(aName, aStation), mCdaDtsDevice(aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

uint16 CCdaDtsDevTest::GetTestTimeoutS(const std::string& aSettingName)
{
    static const uint16 MIN_TEST_TIMEOUT_SECONDS = 1;
    static const uint16 MAX_TEST_TIMEOUT_SECONDS = 60;

    // Get and validate the test timeout
    uint16 testTimeoutSeconds = GetSetup().GetValueNum<uint16>(aSettingName);
    if (testTimeoutSeconds < MIN_TEST_TIMEOUT_SECONDS
        || testTimeoutSeconds > MAX_TEST_TIMEOUT_SECONDS)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aSettingName << "\", value \""
            << testTimeoutSeconds << "\" is out of range (must be between "
            << MIN_TEST_TIMEOUT_SECONDS << " and " << MAX_TEST_TIMEOUT_SECONDS << ")";
        ThrowError(msg.str());
    }

    return testTimeoutSeconds;
}


////////////////////////////////////////////////////////////////////////////////
// CChargerLedTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsLedTest::NAME = "DTS:LEDS";

CCdaDtsLedTest::CCdaDtsLedTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsLedTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;
    for (LedTest test : mTests)
    {
        // Set the mode and ask for a check
        mCdaDtsDevice.LedSet(static_cast<uint8>(1 << test.id));

        ostringstream onMsg;
        onMsg << "Check " << test.description << " LED switched ON";
        pass = AskUserCheck(onMsg.str()) && pass;

        // Switch off the LED(s) and ask for check
        mCdaDtsDevice.LedSet(0);
        if (pass || !GetStation().StopOnFail())
        {
            ostringstream offMsg;
            offMsg << "Check " << test.description << " LED now OFF";
            pass = AskUserCheck(offMsg.str()) && pass;
        }

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsLedTest::GetTests()
{
    // Get and validate the LEDs to test
    /// The format of the setting string is "ID:description,...,ID:description",
    /// e.g. "0:Green,1:Red".
    static const string TESTS_SETTING = "Leds";
    static const regex REGEXP_LED_DEF("(\\d+):(.+)");

    vector<string> ledTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = ledTests.begin();
        iStr != ledTests.end();
        ++iStr)
    {
        cmatch match;
        if (!regex_match(iStr->c_str(), match, REGEXP_LED_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << *iStr << "\" is invalid";
            ThrowError(msg.str());
        }
        else
        {
            uint16 led; // Using uint16 rather than uint8 to avoid interpretation as char
            istringstream iss(match.str(1));
            if (!(iss >> led) || !iss.eof() || led > CCdaDtsDevice::LED_ID_MAX)
            {
                ostringstream msg;
                msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                    << *iStr << "\" contains an invalid ID value (must be a decimal between 0 and "
                    << static_cast<uint16>(CCdaDtsDevice::LED_ID_MAX) << ")";
                ThrowError(msg.str());
            }
            else
            {
                LedTest test;
                test.id = static_cast<uint8>(led);
                test.description = match.str(2);
                
                mTests.push_back(test);
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// CAudioToneTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsAudioToneTest::NAME = "DTS:AUDIO-TONE";

CCdaDtsAudioToneTest::CCdaDtsAudioToneTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsAudioToneTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;

    for (ToneTest test : mTests)
    {
        mCdaDtsDevice.AudioToneStart(CCdaDtsDevice::AudioHardware::CODEC, 0,
            test.channel, test.tone);

        ostringstream msg;
        msg << "Check tone is playing from the channel "
            << static_cast<uint16>(test.channel) << " speaker";
        pass = AskUserCheck(msg.str()) && pass;

        mCdaDtsDevice.AudioToneStop();

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsAudioToneTest::GetTests()
{
    static const string TESTS_SETTING = "AudioToneTests";
    static const regex REGEXP_TONE_TEST_DEF("(\\d+):(\\d+)");

    vector<string> toneTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = toneTests.begin();
        iStr != toneTests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_TONE_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        ToneTest toneTest;
        istringstream issChannel(match.str(1));
        uint16 chan;
        if (!(issChannel >> chan) || !issChannel.eof() || 
            static_cast<CCdaDtsDevice::AudioChannel>(chan) > CCdaDtsDevice::AudioChannel::MAX)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" channel invalid (must be a value between 0 and "
                << static_cast<uint16>(CCdaDtsDevice::AudioChannel::MAX) << ")";
            ThrowError(msg.str());
        }
        else
        {
            toneTest.channel = static_cast<CCdaDtsDevice::AudioChannel>(chan);
        }

        istringstream issTone(match.str(2));
        uint16 tone; // uint16 to avoid treatment as char
        if (!(issTone >> tone) || !issTone.eof() || tone > CCdaDtsDevice::AUDIO_TONE_MAX)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" tone invalid (must be a value between 0 and "
                << static_cast<uint16>(CCdaDtsDevice::AUDIO_TONE_MAX) << ")";
            ThrowError(msg.str());
        }
        else
        {
            toneTest.tone = static_cast<uint8>(tone);
        }

        mTests.push_back(toneTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsAudioLoopTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsAudioLoopTest::NAME = "DTS:AUDIO-LOOP";

CCdaDtsAudioLoopTest::CCdaDtsAudioLoopTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsAudioLoopTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;

    for (LoopTest test : mTests)
    {
        mCdaDtsDevice.AudioLoopbackStart(test.mic, test.outHardware,
            test.outInstance, test.outChannel);

        ostringstream msg;
        msg << "Check audio loop \"" << test.desc << "\"";
        pass = AskUserCheck(msg.str()) && pass;

        mCdaDtsDevice.AudioLoopbackStop();

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsAudioLoopTest::GetTests()
{
    static const string TESTS_SETTING = "AudioLoopTests";
    static const regex REGEXP_LOOP_TEST_DEF("(.+):(\\d+):(\\d+):(\\d+):(\\d+)");

    vector<string> loopTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = loopTests.begin();
        iStr != loopTests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_LOOP_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        LoopTest loopTest;
        loopTest.desc = match.str(1);

        // mic
        istringstream issMic(match.str(2));
        uint16 mic; // uint16 to avoid treatment as char
        if (!(issMic >> mic) || !issMic.eof() || mic > numeric_limits<uint8>::max())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" mic invalid (must be a value between 0 and "
                << numeric_limits<uint8>::max() << ")";
            ThrowError(msg.str());
        }
        else
        {
            loopTest.mic = static_cast<uint8>(mic);
        }

        // output hardware
        istringstream issOutHw(match.str(3));
        uint16 outHw;
        if (!(issOutHw >> outHw) || !issOutHw.eof() ||
            static_cast<CCdaDtsDevice::AudioHardware>(outHw) > CCdaDtsDevice::AudioHardware::MAX)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" output hardware invalid (must be a value between 0 and "
                << static_cast<uint16>(CCdaDtsDevice::AudioHardware::MAX) << ")";
            ThrowError(msg.str());
        }
        else
        {
            loopTest.outHardware = static_cast<CCdaDtsDevice::AudioHardware>(outHw);
        }

        // output instance
        istringstream issOutInst(match.str(4));
        uint16 outInst; // uint16 to avoid treatment as char
        if (!(issOutInst >> outInst) || !issOutInst.eof() || outInst > numeric_limits<uint8>::max())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" output instance invalid (must be a value between 0 and "
                << numeric_limits<uint8>::max() << ")";
            ThrowError(msg.str());
        }
        else
        {
            loopTest.outInstance = static_cast<uint8>(outInst);
        }

        // output channel
        istringstream issOutChan(match.str(5));
        uint16 outChan;
        if (!(issOutChan >> outChan) || !issOutChan.eof() ||
            static_cast<CCdaDtsDevice::AudioChannel>(outChan) > CCdaDtsDevice::AudioChannel::MAX)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" output channel invalid (must be a value between 0 and "
                << static_cast<uint16>(CCdaDtsDevice::AudioChannel::MAX) << ")";
            ThrowError(msg.str());
        }
        else
        {
            loopTest.outChannel = static_cast<CCdaDtsDevice::AudioChannel>(outChan);
        }

        mTests.push_back(loopTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsTouchpadTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsTouchpadTest::NAME = "DTS:TOUCHPAD";

const CCdaDtsTouchpadTest::TouchActionMap CCdaDtsTouchpadTest::mSupportedActionsMap = {
    { "TOUCH", CCdaDtsDevice::TouchType::TOUCH },
    { "SWIPE", CCdaDtsDevice::TouchType::SLIDE },
    { "COVER", CCdaDtsDevice::TouchType::HAND_COVER }
};

CCdaDtsTouchpadTest::CCdaDtsTouchpadTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    mTestTimeoutSeconds = GetTestTimeoutS("TouchTestTimeoutSeconds");

    static const string TOUCH_TESTS_SETTING = "TouchTests";
    vector<string> actions = GetSetup().GetValueList(TOUCH_TESTS_SETTING, true);

    for (string action : actions)
    {
        PtUtil::ToUpper(action);
        TouchActionMap::const_iterator it = mSupportedActionsMap.find(action);
        if (it == mSupportedActionsMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TOUCH_TESTS_SETTING
                << "\" contains invalid touchpad action \"" << action << "\"";
            ThrowError(msg.str());
        }
        else
        {
            mActions.push_back(*it);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsTouchpadTest::Execute()
{
    CPtTimer testTimer(mName);

    // First test that nothing is detected as expected without any touch
    bool pass = !mCdaDtsDevice.DetectTouch(1, CCdaDtsDevice::TouchType::ANY);
    if (!pass)
    {
        WriteLog("Touchpad no touch test FAILED", false);
    }

    if (pass || !GetStation().StopOnFail())
    {
        for (TouchActionPair touchAction : mActions)
        {
            ostringstream msg;
            msg << "Please hit a key, then " << touchAction.first << " the touchpad until the test completes";
            AskUserHitKey(msg.str());

            bool detected = mCdaDtsDevice.DetectTouch(mTestTimeoutSeconds, touchAction.second);

            if (!detected)
            {
                ostringstream errMsg;
                errMsg << "Touchpad " << touchAction.first << " test FAILED";
                WriteLog(errMsg.str(), false);
            }
            pass = pass && detected;

            if (!pass && GetStation().StopOnFail())
            {
                break;
            }
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsProximityTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsProximityTest::NAME = "DTS:PROXIMITY";

CCdaDtsProximityTest::CCdaDtsProximityTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    mTestTimeoutSeconds = GetTestTimeoutS("ProximityTestTimeoutSeconds");
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsProximityTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Please hit a key, then place/remove hand near the proximity sensor");

    bool pass = mCdaDtsDevice.TestProximity(mTestTimeoutSeconds);

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsHallSensorTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsHallSensorTest::NAME = "DTS:HALL-SENSOR";

CCdaDtsHallSensorTest::CCdaDtsHallSensorTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    mTestTimeoutSeconds = GetTestTimeoutS("HallSensorTestTimeoutSeconds");
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsHallSensorTest::Execute()
{
    CPtTimer testTimer(mName);

    AskUserHitKey("Please hit a key, then place/remove magnet near the DUT");

    bool pass = mCdaDtsDevice.TestHallEffect(mTestTimeoutSeconds);

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsTemperatureTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsTemperatureTest::NAME = "DTS:TEMP";

CCdaDtsTemperatureTest::CCdaDtsTemperatureTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetLimits("TempTestRangeDegC", mLowLimit, mHighLimit);
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsTemperatureTest::Execute()
{
    CPtTimer testTimer(mName);

    int16 tempC = mCdaDtsDevice.GetTemperatureDegC();

    bool pass = false;
    if (tempC < mLowLimit || tempC > mHighLimit)
    {
        ostringstream msg;
        msg << "Temperature value " << tempC
            << " (degrees C) is out of range (" << mLowLimit << " - " << mHighLimit << ")";
        WriteLog(msg.str(), true);
    }
    else
    {
        ostringstream msg;
        msg << "Temperature value = " << tempC << " (degrees C)";
        WriteLog(msg.str(), false);

        pass = true;
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsBatteryTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsBatteryTest::NAME = "DTS:BATT";

CCdaDtsBatteryTest::CCdaDtsBatteryTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetLimits("BattTestRangeMv", mLowLimit, mHighLimit);
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsBatteryTest::Execute()
{
    CPtTimer testTimer(mName);

    uint16 battMv = mCdaDtsDevice.GetBatteryMv();

    bool pass = false;
    if (battMv < mLowLimit || battMv > mHighLimit)
    {
        ostringstream msg;
        msg << "Battery milli-volts " << battMv
            << " is out of range (" << mLowLimit << " - " << mHighLimit << ")";
        WriteLog(msg.str(), true);
    }
    else
    {
        ostringstream msg;
        msg << "Battery milli-volts = " << battMv;
        WriteLog(msg.str(), false);

        pass = true;
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsRssiTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsRssiTest::NAME = "DTS:RSSI";

CCdaDtsRssiTest::CCdaDtsRssiTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetLimits("RssiTestRangeDbm", mLowLimit, mHighLimit);
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsRssiTest::Execute()
{
    CPtTimer testTimer(mName);

    int16 rssiDbm = mCdaDtsDevice.GetRssiDbm();

    bool pass = false;
    if (rssiDbm < mLowLimit || rssiDbm > mHighLimit)
    {
        ostringstream msg;
        msg << "RSSI value " << rssiDbm
            << " dBm is out of range (" << mLowLimit << " - " << mHighLimit << ")";
        WriteLog(msg.str(), true);
    }
    else
    {
        ostringstream msg;
        msg << "RSSI value = " << rssiDbm << " dBm";
        WriteLog(msg.str(), false);

        pass = true;
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CRfPowerTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsRfPowerTest::NAME = "DTS:RF-POWER";

const CCdaDtsRfPowerTest::RfStopMethodMap CCdaDtsRfPowerTest::mSupportedStopMethodsMap = {
    { "TIME", RfStopMethod::TIME },
    { "PIO", RfStopMethod::PIO },
    { "TOUCH", RfStopMethod::TOUCH }
};

////////////////////////////////////////////////////////////////////////////////

CCdaDtsRfPowerTest::CCdaDtsRfPowerTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut),
    mpSpecAn(aStation.GetSpectrumAnalyser()),
    mChannel(0),
    mExpPowerDbm(GetSetup().GetValueNum<float64>("ExpectedPowerDbm")),
    mPowerMarginDbm(GetSetup().GetValueNum<float64>("PowerMarginDbm")),
    mRebootWaitS(0)
{
    // Validate the channel setting
    static const string CHANNEL_SETTING = "RfTestChannel";
    static const string TEST_STOP_METHOD_SETTING = "RfTestStopMethod";
    static const string REBOOT_WAIT_SETTING = "DtsDutRebootWaitS";
    static const uint16 MAX_CHANNEL = 79;
    static const uint16 MAX_REBOOT_WAIT_S = 60;

    // Validate the channel
    mChannel = GetSetup().GetValueNum<uint16>(CHANNEL_SETTING);
    if (mChannel > MAX_CHANNEL)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << CHANNEL_SETTING << "\" exceeds maximum ("
            << MAX_CHANNEL << ")";
        ThrowError(msg.str());
    }

    // Deal with the stop method
    string stopMethod = GetSetup().GetValue(TEST_STOP_METHOD_SETTING, true);
    vector<string> stopFields = PtUtil::SplitString(stopMethod, ":");
    // Remove blank fields
    auto strIsEmpty = [](const string& aStr) { return aStr.empty(); };
    stopFields.erase(std::remove_if(stopFields.begin(), stopFields.end(), strIsEmpty), stopFields.end());

    if (stopFields.size() > 3)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << TEST_STOP_METHOD_SETTING
            << "\" is invalid, requires a maximum of 3 fields separated by \':\'";
        ThrowError(msg.str());
    }
    else
    {
        string& method = stopFields.at(0);
        PtUtil::ToUpper(method);
        RfStopMethodMap::const_iterator it = mSupportedStopMethodsMap.find(method);
        if (it == mSupportedStopMethodsMap.end())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TEST_STOP_METHOD_SETTING
                << "\" contains invalid method \"" << method << "\"";
            ThrowError(msg.str());
        }
        else
        {
            mRfStopData.method = it->second;

            if (mRfStopData.method == RfStopMethod::TIME ||
                mRfStopData.method == RfStopMethod::PIO)
            {
                if (stopFields.size() < 2)
                {
                    ostringstream msg;
                    msg << "Configuration setting \"" << TEST_STOP_METHOD_SETTING
                        << "\" is invalid, requires a minimum of 2 fields separated by \':\' for the specified \""
                        << method << "\" stop method";
                    ThrowError(msg.str());
                }
                else
                {
                    const string& param = stopFields.at(1);
                    istringstream iss(param);
                    uint16 value; // Using this as temporary to avoid interpretation as char
                    if (param.at(0) == '-' || !(iss >> value) || !iss.eof())
                    {
                        std::ostringstream msg;
                        msg << "Configuration setting \"" << TEST_STOP_METHOD_SETTING << "\", value \""
                            << param << "\" could not be converted (must be a decimal between 0 and 255)";
                        ThrowError(msg.str());
                    }
                    else if (mRfStopData.method == RfStopMethod::TIME && (value < 1 || value > 60))
                    {
                        std::ostringstream msg;
                        msg << "Configuration setting \"" << TEST_STOP_METHOD_SETTING << "\", value \""
                            << param << "\" must be between 1 and 60 for method \"" << method << "\"";
                        ThrowError(msg.str());
                    }
                    else
                    {
                        mRfStopData.param = static_cast<uint8>(value);
                    }

                    if (stopFields.size() == 3)
                    {
                        mRfStopData.pioDesc = stopFields.at(2);
                    }
                }
            }
        }
    }

    mRebootWaitS = GetSetup().GetValueNum<uint16>(REBOOT_WAIT_SETTING);
    if (mRebootWaitS > MAX_REBOOT_WAIT_S)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << REBOOT_WAIT_SETTING
            << "\" exceeds maximum (" << MAX_REBOOT_WAIT_S << ")";
        ThrowError(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsRfPowerTest::Execute()
{
    const uint8 POW_LVL = 6; // Default max power

    CPtTimer testTimer(mName);

    const uint16 channelFreqMHz = GetBtFreqMhz(mChannel);
    bool pass = false;
    time_point<steady_clock> txStartTime;

    // Have to set the power level before starting an RF TX test
    mCdaDtsDevice.RadioCfgTxPower(POW_LVL);

    // Set the stop method
    if (mRfStopData.method == RfStopMethod::TIME)
    {
        mCdaDtsDevice.RadioCfgStopTime(mRfStopData.param);
    }
    else if (mRfStopData.method == RfStopMethod::PIO)
    {
        mCdaDtsDevice.RadioCfgStopPio(mRfStopData.param);
    }
    else if (mRfStopData.method == RfStopMethod::TOUCH)
    {
        mCdaDtsDevice.RadioCfgStopTouch();
    }

    // Run the test
    if (mpSpecAn == NULL)
    {
        mCdaDtsDevice.RadioTxCwStart(static_cast<uint8>(mChannel));
        txStartTime = steady_clock::now();

        // Operator check of the output
        ostringstream msg;
        msg << "Check carrier wave TX on channel "
            << mChannel << " (" << channelFreqMHz << " MHz) is between "
            << mExpPowerDbm - mPowerMarginDbm << " and "
            << mExpPowerDbm + mPowerMarginDbm << " dBm";
        pass = AskUserCheck(msg.str());
    }
    else
    {
        mpSpecAn->SetCentreFrequency(channelFreqMHz * 1000000);

        mCdaDtsDevice.RadioTxCwStart(static_cast<uint8>(mChannel));
        txStartTime = steady_clock::now();

        float64 powerDbm = mpSpecAn->MeasurePowerDbm(mExpPowerDbm + mPowerMarginDbm);

        ostringstream msg;
        msg << "DUT power = " << fixed << setprecision(2) << powerDbm << " dBm";
        WriteLog(msg.str(), false);

        if ((powerDbm <= mExpPowerDbm + mPowerMarginDbm) &&
            (powerDbm >= mExpPowerDbm - mPowerMarginDbm))
        {
            pass = true;
        }
    }

    // RF test mode needs to stop / be stopped
    if (mRfStopData.method == RfStopMethod::TIME)
    {
        // Need to wait until the test mode stops
        uint64 totalWaitS = mRfStopData.param;
        uint64 elapsedS;
        do
        {
            elapsedS = duration_cast<seconds>(steady_clock::now() - txStartTime).count();

            ostringstream waitMsg;
            waitMsg << "Waiting for test mode to finish, time remaining = "
                    << (elapsedS < totalWaitS ? totalWaitS - elapsedS : 0) << " seconds";
            WriteUpdateMsg(waitMsg.str());

            if (elapsedS < totalWaitS)
            {
                this_thread::sleep_for(seconds(1));
            }
        }
        while (elapsedS < totalWaitS);

        // End the line after updating the wait time.
        WriteLogNoPrefix("", true);
    }
    else if (mRfStopData.method == RfStopMethod::PIO)
    {
        ostringstream msg;
        msg << "Please trigger a ";
        if (mRfStopData.pioDesc.empty())
        {
            msg << "PIO " << static_cast<uint16>(mRfStopData.param);
        }
        else
        {
            msg << mRfStopData.pioDesc;
        }
        msg << " state change (to stop the RF test mode), then hit a key";
        AskUserHitKey(msg.str());
    }
    else if (mRfStopData.method == RfStopMethod::TOUCH)
    {
        AskUserHitKey("Please touch the touchpad (to stop the RF test mode), then hit a key");
    }

    // Allow time for reboot (needs to be up and ready otherwise opening the DTS COM port fails)
    ostringstream waitMsg;
    waitMsg << "Waiting " << mRebootWaitS << " seconds for DUT reboot";
    WriteLog(waitMsg.str(), true);
    this_thread::sleep_for(seconds(mRebootWaitS));

    // Entering the RF Test mode breaks DTS connectivity, so need to reconnect
    mCdaDtsDevice.Connect();

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

uint16 CCdaDtsRfPowerTest::GetBtFreqMhz(uint16 aChannel)
{
    static const uint16 CHANNEL_0_FREQ_MHZ = 2402;

    return CHANNEL_0_FREQ_MHZ + aChannel;
}


////////////////////////////////////////////////////////////////////////////////
// CCdaDtsCfgSetTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsCfgSetTest::NAME = "DTS:CFG-SET";

CCdaDtsCfgSetTest::CCdaDtsCfgSetTest(const CStation& aStation, CCdaDtsDevice& aDut)
    : CCdaDtsDevTest(NAME, aStation, aDut)
{
    GetPsKeys();

    if (mPsKeys.empty())
    {
        ThrowError("No PS keys specified");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsCfgSetTest::Execute()
{
    CPtTimer testTimer(mName);

    // PS store keys
    if (!mPsKeys.empty())
    {
        for (vector<CPtPsStoreKey>::const_iterator iKey = mPsKeys.begin();
            iKey != mPsKeys.end();
            ++iKey)
        {
            if (iKey->Value().empty())
            {
                mCdaDtsDevice.PsClear(static_cast<uint16>(iKey->Id()));
            }
            else
            {
                mCdaDtsDevice.PsSetValue(static_cast<uint16>(iKey->Id()),
                    iKey->Value());
            }

            LogPsKeyWrite(*iKey);
        }
    }

    return ReportResult(true);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsCfgSetTest::GetPsKeys()
{
    static const string PS_SETTING = "CfgSetPs";

    vector<string> psSettings = GetSetup().GetValueList(PS_SETTING, false);
    if (!psSettings.empty())
    {
        for (vector<string>::const_iterator iStr = psSettings.begin();
            iStr != psSettings.end();
            ++iStr)
        {
            string setting = *iStr;
            PtUtil::TrimString(setting);
            try
            {
                CPtPsStoreKey psKey(setting, false);
                mPsKeys.push_back(psKey);
            }
            catch (CPtException& ex)
            {
                ostringstream msg;
                msg << "Configuration setting \"" << PS_SETTING
                    << "\" is invalid (" << ex.what() << ")";
                ThrowError(msg.str());
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsCfgSetTest::LogPsKeyWrite(const CPtPsStoreKey& aKey)
{
    ostringstream msg;
    msg << "Wrote PS store setting \"" << aKey.ToString() << "\" to device";
    WriteLog(msg.str(), false);
}
