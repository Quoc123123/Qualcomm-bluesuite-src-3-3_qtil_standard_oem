//**************************************************************************************************
//
//  CdaDevTests.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA device test class definitions, part of an example application for production test.
//
//**************************************************************************************************

#include "CdaDevTests.h"
#include "CdaDevice.h"
#include "Equipment\SpectrumAnalyserInstrument.h"
#include "PtTimer.h"
#include "PtStation.h"
#include "PtBdAddrMgr.h"
#include "PtUtil.h"
#include "PtSerialNum.h"
#include <sstream>
#include <iomanip>
#include <regex>
#include <fstream>
#include <chrono>
#include <thread>
#include <assert.h>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////
// CCdaDevTestFactory
////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> CCdaDevTestFactory::mSupportedTests =
{
    CCdaLedTest::NAME,
    CCdaButtonsTest::NAME,
    CCdaChargerTest::NAME,
    CCdaAudioToneTest::NAME,
    CCdaAudioLoopTest::NAME,
    CCdaXtalTrimTest::NAME,
    CCdaRfPowerTest::NAME,
    CCdaSetBdAddrTest::NAME,
    CCdaSetBdNameTest::NAME,
    CCdaSetProdTestModeTest::NAME,
    CCdaSetSnTest::NAME,
    CCdaCheckSnTest::NAME,
    CCdaCfgMergeTest::NAME,
    CCdaI2CDevicesTest::NAME,
    CCdaI2CTouchpadTest::NAME,
    CCdaUserHcTest::NAME
};

////////////////////////////////////////////////////////////////////////////////

bool CCdaDevTestFactory::TestSupported(const std::string& aName)
{
    return (find(mSupportedTests.begin(), mSupportedTests.end(), aName) != mSupportedTests.end());
}

////////////////////////////////////////////////////////////////////////////////

std::vector<CTest*> CCdaDevTestFactory::CreateTests(const CStation& aStation)
{
    // These tests are for CdaDevice DUTs only
    CCdaDevice* pDev = dynamic_cast<CCdaDevice*>(aStation.GetDut());
    if (pDev == NULL)
    {
        throw CPtException("DUT is not supported by CDA device tests");
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

        if (testName == CCdaLedTest::NAME)
        {
            tests.push_back(new CCdaLedTest(aStation, *pDev));
        }
        else if (testName == CCdaButtonsTest::NAME)
        {
            tests.push_back(new CCdaButtonsTest(aStation, *pDev));
        }
        else if (testName == CCdaChargerTest::NAME)
        {
            tests.push_back(new CCdaChargerTest(aStation, *pDev));
        }
        else if (testName == CCdaAudioToneTest::NAME)
        {
            tests.push_back(new CCdaAudioToneTest(aStation, *pDev));
        }
        else if (testName == CCdaAudioLoopTest::NAME)
        {
            tests.push_back(new CCdaAudioLoopTest(aStation, *pDev));
        }
        else if (testName == CCdaXtalTrimTest::NAME)
        {
            tests.push_back(new CCdaXtalTrimTest(aStation, *pDev));
        }
        else if (testName == CCdaRfPowerTest::NAME)
        {
            tests.push_back(new CCdaRfPowerTest(aStation, *pDev));
        }
        else if (testName == CCdaSetBdAddrTest::NAME)
        {
            tests.push_back(new CCdaSetBdAddrTest(aStation, *pDev));
        }
        else if (testName == CCdaSetBdNameTest::NAME)
        {
            tests.push_back(new CCdaSetBdNameTest(aStation, *pDev));
        }
        else if (testName == CCdaSetProdTestModeTest::NAME)
        {
            tests.push_back(new CCdaSetProdTestModeTest(aStation, *pDev));
        }
        else if (testName == CCdaSetSnTest::NAME)
        {
            tests.push_back(new CCdaSetSnTest(aStation, *pDev));
        }
        else if (testName == CCdaCheckSnTest::NAME)
        {
            tests.push_back(new CCdaCheckSnTest(aStation, *pDev));
        }
        else if (testName == CCdaCfgMergeTest::NAME)
        {
            tests.push_back(new CCdaCfgMergeTest(aStation, *pDev));
        }
        else if (testName == CCdaI2CDevicesTest::NAME)
        {
            tests.push_back(new CCdaI2CDevicesTest(aStation, *pDev));
        }
        else if (testName == CCdaI2CTouchpadTest::NAME)
        {
            tests.push_back(new CCdaI2CTouchpadTest(aStation, *pDev));
        }
        else if (testName == CCdaUserHcTest::NAME)
        {
            tests.push_back(new CCdaUserHcTest(aStation, *pDev));
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
// CCdaDevTest
////////////////////////////////////////////////////////////////////////////////

CCdaDevTest::CCdaDevTest(const std::string& aName, const CStation& aStation,
    CCdaDevice& aDut)
    : CTest(aName, aStation), mCdaDevice(aDut)
{
}


////////////////////////////////////////////////////////////////////////////////
// CRfTest
////////////////////////////////////////////////////////////////////////////////

CCdaRfTest::CCdaRfTest(const std::string& aName, const CStation& aStation,
    CCdaDevice& aDut)
    : CCdaDevTest(aName, aStation, aDut),
      mpSpecAn(aStation.GetSpectrumAnalyser())
{
    // Validate the channel setting
    static const string CHANNEL_SETTING = "RfTestChannel";
    static const uint16 MAX_CHANNEL = 79;

    mChannel = GetSetup().GetValueNum<uint16>(CHANNEL_SETTING);
    if (mChannel > MAX_CHANNEL)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << CHANNEL_SETTING << "\" exceeds maximum ("
            << MAX_CHANNEL << ")";
        ThrowError(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

uint16 CCdaRfTest::GetBtFreqMhz(uint16 aChannel)
{
    static const uint16 CHANNEL_0_FREQ_MHZ = 2402;

    return CHANNEL_0_FREQ_MHZ + aChannel;
}


////////////////////////////////////////////////////////////////////////////////
// CXtalTrimTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaXtalTrimTest::NAME = "XTAL-TRIM";

CCdaXtalTrimTest::CCdaXtalTrimTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaRfTest(NAME, aStation, aDut),
      mIncrementalSearch(GetSetup().GetValueNum<bool>("XtalTrimIncremental")),
      mTrimMarginCoarseHz(GetSetup().GetValueNum<uint16>("XtalCoarseMarginHz")),
      mTrimMarginFineHz(GetSetup().GetValueNum<uint16>("XtalFineMarginHz"))
{
    if (mpSpecAn == NULL)
    {
        ThrowError("Spectrum analyser required for this test");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaXtalTrimTest::Execute()
{
    CPtTimer testTimer(mName);

    const uint16 channelFreqMHz = GetBtFreqMhz(mChannel);

    ostringstream xtalMsg;
    xtalMsg << "Trimming on channel " << mChannel << " (" << channelFreqMHz << " MHz)";
    WriteLog(xtalMsg.str(), false);

    mCdaDevice.RadioTxCwStart(mChannel, channelFreqMHz);

    mpSpecAn->SetCentreFrequency(channelFreqMHz * 1000000);

    uint16 xtalLoadCap;
    int16 xtalTrim;
    bool pass;

    {
        CPtTimer searchTimer(mName + " search");
        if (mIncrementalSearch)
        {
            pass = CalcXtalTrimIncremental(channelFreqMHz, xtalLoadCap, xtalTrim);
        }
        else
        {
            pass = CalcXtalTrimBinChop(channelFreqMHz, xtalLoadCap, xtalTrim);
        }
    }

    // Stop the device from transmitting
    mCdaDevice.RadioStop();

    ostringstream msg;
    msg << "LoadCap = " << xtalLoadCap << ", Trim = " << xtalTrim;
    WriteLog(msg.str(), false);

    // Write updated XTAL Load Cap value
    ostringstream loadCapValStr;
    loadCapValStr << xtalLoadCap;
    mCdaDevice.ConfigSetValue("curator", "XtalLoadCapacitance", loadCapValStr.str());

    // Write updated XTAL trim value
    ostringstream trimValStr;
    trimValStr << xtalTrim;
    mCdaDevice.ConfigSetValue("curator", "XtalFreqTrim", trimValStr.str());

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaXtalTrimTest::CalcXtalTrimIncremental(uint32 aFreqMHz,
    uint16& aLoadCap, int16& aTrim)
{
    static const uint16 MAX_TRIM_STEPS = 32; // To prevent endless cycling up/down

    bool pass = false;

    // Start mid-range
    aLoadCap = 16;
    aTrim = 0;

    // Init fine trim value prior to coarse value trim
    mCdaDevice.XtalSetTrim(aTrim);

    int32 measuredFreqOffsetHz = 0;
    bool xtalLoadCapFound = false;
    int32 freqOffsetLowerLim = mTrimMarginCoarseHz * -1;
    int32 freqOffsetUpperLim = mTrimMarginCoarseHz;

    // Find the load cap (coarse trim) value
    for (uint16 i = 0;
        i < MAX_TRIM_STEPS && !xtalLoadCapFound && aLoadCap >= MIN_XTAL_LCAP && aLoadCap <= MAX_XTAL_LCAP;
        ++i)
    {
        ostringstream msg;
        msg << "LoadCap = " << aLoadCap;
        WriteLog(msg.str(), false);

        mCdaDevice.XtalSetLoadCap(aLoadCap);

        measuredFreqOffsetHz = ReadFreqOffsetHz(aFreqMHz);

        if (measuredFreqOffsetHz > freqOffsetUpperLim)
        {
            ++aLoadCap;
        }
        else if (measuredFreqOffsetHz < freqOffsetLowerLim)
        {
            --aLoadCap;
        }
        else
        {
            xtalLoadCapFound = true;
        }
    }

    if (!xtalLoadCapFound)
    {
        WriteLog("LoadCap value not found", false);
    }
    else
    {
        // Find the fine trim value
        bool xtalTrimFound = false;
        freqOffsetLowerLim = mTrimMarginFineHz * -1;
        freqOffsetUpperLim = mTrimMarginFineHz;

        for (uint16 i = 0;
            i < MAX_TRIM_STEPS && !xtalTrimFound && aTrim >= MIN_XTAL_TRIM && aTrim <= MAX_XTAL_TRIM;
            ++i)
        {
            // The trim value was set to the initial value prior to the LoadCap search, and 
            // we already have a measurement to check from that process too.
            if (i != 0)
            {
                ostringstream msg;
                msg << "Trim = " << aTrim;
                WriteLog(msg.str(), false);

                mCdaDevice.XtalSetTrim(aTrim);

                measuredFreqOffsetHz = ReadFreqOffsetHz(aFreqMHz);
            }

            if (measuredFreqOffsetHz > freqOffsetUpperLim)
            {
                ++aTrim;
            }
            else if (measuredFreqOffsetHz < freqOffsetLowerLim)
            {
                --aTrim;
            }
            else
            {
                xtalTrimFound = true;
            }
        }

        if (!xtalTrimFound)
        {
            WriteLog("Trim value not found", false);
        }
        else
        {
            pass = true;
        }
    }

    return pass;
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaXtalTrimTest::CalcXtalTrimBinChop(uint32 aFreqMHz, uint16& aLoadCap,
    int16& aTrim)
{
    bool pass = false;

    // Start mid-range
    aLoadCap = 16;
    aTrim = 0;

    int16 lowerBound = MIN_XTAL_LCAP;
    int16 upperBound = MAX_XTAL_LCAP;
    int32 measuredFreqOffsetHz = 0;
    int16 adjustment;

    // Init fine trim value prior to coarse value trim
    mCdaDevice.XtalSetTrim(aTrim);

    // Find the load capacitance (coarse trim) value
    bool xtalLoadCapFound = false;
    int32 freqOffsetLowerLim = mTrimMarginCoarseHz * -1;
    int32 freqOffsetUpperLim = mTrimMarginCoarseHz;

    while (!xtalLoadCapFound)
    {
        mCdaDevice.XtalSetLoadCap(aLoadCap);

        ostringstream msg;
        msg << "LoadCap = " << aLoadCap << ". Upper = " << upperBound
            << ". Lower = " << lowerBound;
        WriteLog(msg.str(), false);

        measuredFreqOffsetHz = ReadFreqOffsetHz(aFreqMHz);

        if (measuredFreqOffsetHz > freqOffsetUpperLim)
        {
            if (aLoadCap == lowerBound)
            {
                break;
            }
            lowerBound = aLoadCap;
            adjustment = ((upperBound - lowerBound) / 2);
            adjustment > 0 ? aLoadCap += adjustment : aLoadCap += 1;
        }
        else if (measuredFreqOffsetHz < freqOffsetLowerLim)
        {
            if (aLoadCap == upperBound)
            {
                break;
            }
            upperBound = aLoadCap;
            adjustment = ((upperBound - lowerBound) / 2);
            adjustment > 0 ? aLoadCap -= adjustment : aLoadCap -= 1;
        }
        else
        {
            xtalLoadCapFound = true;
        }
    }

    if (!xtalLoadCapFound)
    {
        WriteLog("LoadCap value not found", false);
    }
    else
    {
        // Find fine trim value
        lowerBound = MIN_XTAL_TRIM;
        upperBound = MAX_XTAL_TRIM;

        bool xtalTrimFound = false;
        freqOffsetLowerLim = mTrimMarginFineHz * -1;
        freqOffsetUpperLim = mTrimMarginFineHz;

        while (!xtalTrimFound)
        {
            // The trim value was set to the initial value prior to the LoadCap search, and 
            // we already have a measurement to check from that process too.
            if (measuredFreqOffsetHz > freqOffsetUpperLim)
            {
                if (aTrim == lowerBound)
                {
                    break;
                }
                lowerBound = aTrim;
                adjustment = ((upperBound - lowerBound) / 2);
                adjustment > 0 ? aTrim += adjustment : aTrim += 1;
            }
            else if (measuredFreqOffsetHz < freqOffsetLowerLim)
            {
                if (aTrim == upperBound)
                {
                    break;
                }
                upperBound = aTrim;
                adjustment = ((upperBound - lowerBound) / 2);
                adjustment > 0 ? aTrim -= adjustment : aTrim -= 1;
            }
            else
            {
                xtalTrimFound = true;
            }

            if (!xtalTrimFound)
            {
                mCdaDevice.XtalSetTrim(aTrim);

                ostringstream msg;
                msg << "Trim = " << aTrim << ". Upper = " << upperBound
                    << ". Lower = " << lowerBound;
                WriteLog(msg.str(), false);

                measuredFreqOffsetHz = ReadFreqOffsetHz(aFreqMHz);
            }
        }

        if (!xtalTrimFound)
        {
            WriteLog("Trim value not found", false);
        }
        else
        {
            pass = true;
        }
    }

    return pass;
}

////////////////////////////////////////////////////////////////////////////////

int32 CCdaXtalTrimTest::ReadFreqOffsetHz(uint32 aNomFreqMHz)
{
    int32 freqOffsetHz = mpSpecAn->MeasureFrequencyHz() - (aNomFreqMHz * 1000000);
    ostringstream msg;
    msg << "Frequency offset = " << freqOffsetHz << " Hz";
    WriteLog(msg.str(), false);

    return freqOffsetHz;
}


////////////////////////////////////////////////////////////////////////////////
// CCdaRfPowerTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaRfPowerTest::NAME = "RF-POWER";

CCdaRfPowerTest::CCdaRfPowerTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaRfTest(NAME, aStation, aDut),
      mExpPowerDbm(GetSetup().GetValueNum<float64>("ExpectedPowerDbm")),
      mPowerMarginDbm(GetSetup().GetValueNum<float64>("PowerMarginDbm"))
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaRfPowerTest::Execute()
{
    CPtTimer testTimer(mName);

    const uint16 channelFreqMHz = GetBtFreqMhz(mChannel);
    bool pass = false;

    if (mpSpecAn == NULL)
    {
        mCdaDevice.RadioTxCwStart(mChannel, channelFreqMHz);

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

        mCdaDevice.RadioTxCwStart(mChannel, channelFreqMHz);

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

    // stop the device from transmitting
    mCdaDevice.RadioStop();

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaPioTest
////////////////////////////////////////////////////////////////////////////////

CCdaPioTest::CCdaPioTest(const std::string& aName, const CStation& aStation,
    CCdaDevice& aDut, const std::string& aSettingName)
    : CCdaDevTest(aName, aStation, aDut)
{
    // Get and validate the PIOs to test
    /// The format of the setting string is:
    /// "PIO:on_state:description:first_state,...,PIO:on_state:description:first_state",
    /// e.g. "66:0:Green:1,67:0:Red:1".
    static const regex REGEXP_PIO_DEF("(\\d+):(\\d+):(.+):(\\d+)");

    vector<string> pioTests = GetSetup().GetValueList(aSettingName, true);

    for (vector<string>::const_iterator iStr = pioTests.begin();
        iStr != pioTests.end();
        ++iStr)
    {
        cmatch match;
        if (!regex_match(iStr->c_str(), match, REGEXP_PIO_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << aSettingName << "\", value \""
                << *iStr << "\" is invalid";
            ThrowError(msg.str());
        }
        else
        {
            uint16 pio; // Using uint16 rather than uint8 to avoid interpretation as char
            istringstream iss(match.str(1));
            if (!(iss >> pio) || !iss.eof() || pio > 255)
            {
                ostringstream msg;
                msg << "Configuration setting \"" << aSettingName << "\", value \""
                    << *iStr << "\" contains an invalid PIO number (must be a decimal between 0 and 255)";
                ThrowError(msg.str());
            }

            PioTest pioTest;
            pioTest.pio = static_cast<uint8>(pio);

            istringstream issOnState(match.str(2));
            if (!(issOnState >> pioTest.onState) || !issOnState.eof())
            {
                ostringstream msg;
                msg << "Configuration setting \"" << aSettingName << "\", value \""
                    << *iStr << "\" contains an invalid on_state value (must be 0 or 1)";
                ThrowError(msg.str());
            }

            pioTest.description = match.str(3);

            istringstream issFirstState(match.str(4));
            if (!(issFirstState >> pioTest.firstState) || !issFirstState.eof())
            {
                ostringstream msg;
                msg << "Configuration setting \"" << aSettingName << "\", value \""
                    << *iStr << "\" contains an invalid first_state value (must be 0 or 1)";
                ThrowError(msg.str());
            }

            mTests.push_back(pioTest);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// CLedTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaLedTest::NAME = "LEDS";

CCdaLedTest::CCdaLedTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaPioTest(NAME, aStation, aDut, "Leds")
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaLedTest::Execute()
{
    static const uint32 PIO_DIRECTION = ~0U; // Direction for all masked PIOs - '1' (output).

    CPtTimer testTimer(mName);

    bool pass = true;
    for (const PioTest& test : mTests)
    {
        const uint16 pioBank = test.pio / 32; // The PIO bank (banks are 32 lines wide)
        const uint32 pioMask = 1UL << (test.pio & 0x1F); // The line in the bank
        const uint32 onValue = (test.onState ? ~0U : 0); // The value to set for LEDs on

        mCdaDevice.PioMap(pioBank, pioMask, pioMask);

        bool stateToTest = test.firstState;
        for (size_t i = 0; i < 2 && (pass || !GetStation().StopOnFail()); ++i)
        {
            if (stateToTest)
            {
                // Switch on the LED(s) and ask for check
                mCdaDevice.PioSet(pioBank, pioMask, PIO_DIRECTION, onValue);

                ostringstream onMsg;
                onMsg << "Check " << test.description << " LED switched ON";
                pass = AskUserCheck(onMsg.str()) && pass;

                // Don't leave the LED on after the test
                if (!test.firstState || (!pass && GetStation().StopOnFail()))
                {
                    mCdaDevice.PioSet(pioBank, pioMask, PIO_DIRECTION, ~onValue);
                }
            }
            else
            {
                // Switch off the LED(s) and ask for check
                mCdaDevice.PioSet(pioBank, pioMask, PIO_DIRECTION, ~onValue);

                ostringstream offMsg;
                offMsg << "Check " << test.description << " LED now OFF";
                pass = AskUserCheck(offMsg.str()) && pass;
            }
            
            stateToTest = !stateToTest;
        }

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CButtonsTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaButtonsTest::NAME = "BUTTONS";

CCdaButtonsTest::CCdaButtonsTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaPioTest(NAME, aStation, aDut, "Buttons")
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaButtonsTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;
    for (const PioTest& test : mTests)
    {
        uint16 pioBank = test.pio / 32; // The PIO bank (banks are 32 lines wide)
        uint32 pioMask = 1UL << (test.pio & 0x1F); // The line in the bank

        mCdaDevice.PioMap(pioBank, pioMask, pioMask);

        uint32 direction = 0;
        uint32 value = 0;
        bool stateToTest = test.firstState;
        for (size_t i = 0; i < 2 && (pass || !GetStation().StopOnFail()); ++i)
        {
            if (stateToTest)
            {
                // Ask the user to hold the button down then hit any key
                ostringstream msg;
                msg << "Press and hold the " << test.description << " and hit a key";
                AskUserHitKey(msg.str());

                // Read the value of the PIO(s)
                value = mCdaDevice.PioGet(pioBank, direction);

                // Check value indicates button is pressed
                const uint32 maskedValue = value & pioMask;
                bool pressed = ((direction & pioMask) == 0) && 
                    (test.onState ? maskedValue != 0 : maskedValue == 0);
                if (!pressed)
                {
                    ostringstream errMsg;
                    errMsg << "Button press test for \"" << test.description << "\" FAILED";
                    WriteLog(errMsg.str(), false);
                }
                pass = pass && pressed;
            }
            else
            {
                // Ask the user to hold the button down then hit any key
                ostringstream msg;
                msg << "Release the " << test.description << " and hit a key";
                AskUserHitKey(msg.str());

                // Read the value of the PIO(s)
                value = mCdaDevice.PioGet(pioBank, direction);

                // Check value indicates button is NOT pressed
                const uint32 maskedValue = value & pioMask;
                bool released = ((direction & pioMask) == 0) && 
                    (test.onState ? maskedValue == 0 : maskedValue != 0);
                if (!released)
                {
                    ostringstream errMsg;
                    errMsg << "Button release test for \"" << test.description << "\" FAILED";
                    WriteLog(errMsg.str(), false);
                }

                pass = pass && released;
            }

            stateToTest = !stateToTest;
        }

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CChargerTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaChargerTest::NAME = "CHARGER";

CCdaChargerTest::CCdaChargerTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaChargerTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = mCdaDevice.IsCharging();

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CAudioToneTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaAudioToneTest::NAME = "AUDIO-TONE";

CCdaAudioToneTest::CCdaAudioToneTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaAudioToneTest::Execute()
{
    CPtTimer testTimer(mName);

    mCdaDevice.AudioToneStart(CCdaDevice::AudioDevice::CODEC, 0,
        CCdaDevice::AudioChannel::LEFT);

    bool pass = AskUserCheck("Check tone is playing from the speaker");

    mCdaDevice.AudioToneStop();

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CAudioLoopTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaAudioLoopTest::NAME = "AUDIO-LOOP";

CCdaAudioLoopTest::CCdaAudioLoopTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
    GetTests();
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaAudioLoopTest::Execute()
{
    static const uint16 MIC_BIAS_INSTANCE = 0;
    static const uint16 MIC_BIAS_ENABLE_KEY_ID = 0;
    static const uint16 MIC_BIAS_ENABLE_VALUE = 1;
    static const uint16 MIC_BIAS_DISABLE_VALUE = 0;
    static const uint32 PIO_DIRECTION = ~0U; // Direction for all masked PIOs - '1' (output).
    
    CPtTimer testTimer(mName);

    bool pass = true;

    for (LoopTest test : mTests)
    {
        const uint32 micPioOn = (test.micPioActive == 1 ? ~0U : 0);
        uint16 micPioBank = test.micPio / 32; // The PIO bank (banks are 32 lines wide)
        uint32 micPioMask = 1U << (test.micPio & 0x1F); // The line in the bank

        // For analogue mics, the PIO value 0 is used as a special value indicating
        // that we should enable mic bias and not use a PIO. A value >0 means a PIO
        // needs to be used to enable the mic, no mic bias required. For digital
        // mics the PIO is always used.
        if (test.inDevice == CCdaDevice::AudioDevice::CODEC && test.micPio == 0)
        {
            mCdaDevice.AudioConfigMicBias(MIC_BIAS_INSTANCE,
                MIC_BIAS_ENABLE_KEY_ID, MIC_BIAS_ENABLE_VALUE);
        }
        else if (test.inDevice == CCdaDevice::AudioDevice::DIGITAL_MIC ||
                 test.inDevice == CCdaDevice::AudioDevice::CODEC && test.micPio != 0)
        {
            mCdaDevice.PioMap(micPioBank, micPioMask, micPioMask);
            mCdaDevice.PioSet(micPioBank, micPioMask, PIO_DIRECTION, micPioOn);
        }

        mCdaDevice.AudioLoopbackStart(test.inDevice, test.inIface, test.inChannel,
            test.outDevice, test.outIface, test.outChannel);

        ostringstream msg;
        msg << "Check audio loop \"" << test.desc << "\"";
        pass = AskUserCheck(msg.str()) && pass;

        mCdaDevice.AudioLoopbackStop();

        if (test.inDevice == CCdaDevice::AudioDevice::CODEC && test.micPio == 0)
        {
            mCdaDevice.AudioConfigMicBias(MIC_BIAS_INSTANCE,
                MIC_BIAS_ENABLE_KEY_ID, MIC_BIAS_DISABLE_VALUE);
        }
        else if (test.inDevice == CCdaDevice::AudioDevice::DIGITAL_MIC ||
                 test.inDevice == CCdaDevice::AudioDevice::CODEC && test.micPio != 0)
        {
            mCdaDevice.PioSet(micPioBank, micPioMask, PIO_DIRECTION, ~micPioOn);
        }

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaAudioLoopTest::GetTests()
{
    static const string TESTS_SETTING = "AudioLoopTests";
    static const regex REGEXP_AUDIO_LOOP_TEST_DEF("(.+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");

    vector<string> loopTests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = loopTests.begin();
        iStr != loopTests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_AUDIO_LOOP_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        LoopTest loopTest;
        loopTest.desc = match.str(1);

        // Convert the value strings to uint16s
        vector<uint16> values;
        for (cmatch::const_iterator iMatch = match.begin() + 2;
            iMatch != match.end();
            ++iMatch)
        {
            istringstream iss(iMatch->str());
            uint16 value;
            if (!(iss >> value) || !iss.eof())
            {
                ostringstream msg;
                msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                    << test << "\" value invalid (\"" << iMatch->str() << "\" is not an unsigned 16 bit integer)";
                ThrowError(msg.str());
            }
            else
            {
                values.push_back(value);
            }
        }

        loopTest.inDevice = static_cast<CCdaDevice::AudioDevice>(values.at(0));
        loopTest.inIface = values.at(1);
        loopTest.inChannel = static_cast<CCdaDevice::AudioChannel>(values.at(2));
        loopTest.micPio = values.at(3);
        loopTest.micPioActive = values.at(4);
        loopTest.outDevice = static_cast<CCdaDevice::AudioDevice>(values.at(5));
        loopTest.outIface = values.at(6);
        loopTest.outChannel = static_cast<CCdaDevice::AudioChannel>(values.at(7));

        // Special case check for the DMIC PIO active value (should be 0 or 1)
        if (loopTest.micPioActive > 1)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" contains an invalid in_dmic_en value (must be either 0 or 1)";
            ThrowError(msg.str());
        }

        mTests.push_back(loopTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CSetBdAddrTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaSetBdAddrTest::NAME = "SET-BDADDR";

CCdaSetBdAddrTest::CCdaSetBdAddrTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut),
      mUserProvidedAddress(false),
      mpBdAddrMgr(NULL)
{
    static const string BD_ADDR_FILE_SETTING = "BdAddrFile";
    static const string BD_ADDR_REC_FILE_SETTING = "BdAddrRecordFile";
    static const string BD_ADDR_INC_SETTING = "BdAddrIncrement";

    // As there's an alternative BD address mechanism, this isn't mandatory
    string bdAddrFile = GetSetup().GetValue(BD_ADDR_FILE_SETTING, false);
    if (!bdAddrFile.empty())
    {
        // Check exists and is writable
        ofstream bdFile(bdAddrFile, (ios_base::in | ios_base::out));
        if (!bdFile.good())
        {
            ostringstream msg;
            msg << BD_ADDR_FILE_SETTING << " \"" << bdAddrFile
                << "\" cannot be opened for read/write (or does not exist)";
            ThrowError(msg.str());
        }
    }

    // This is mandatory as we need to record the address assignment
    string recFile = GetSetup().GetValue(BD_ADDR_REC_FILE_SETTING, true);
    
    // If file doesn't already exist create it
    ofstream rFile(recFile, ios_base::app);
    if (!rFile.good())
    {
        ostringstream msg;
        msg << BD_ADDR_REC_FILE_SETTING << " \"" << recFile << "\" cannot be opened";
        ThrowError(msg.str());
    }

    mpBdAddrMgr = new CPtBdAddrMgr(bdAddrFile,
        GetSetup().GetValueNum<uint16>(BD_ADDR_INC_SETTING),
        recFile);

    mUserProvidedAddress = bdAddrFile.empty();
}

////////////////////////////////////////////////////////////////////////////////

CCdaSetBdAddrTest::~CCdaSetBdAddrTest()
{ 
    delete mpBdAddrMgr;
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaSetBdAddrTest::Execute()
{
    CPtTimer testTimer(mName);

    if (mUserProvidedAddress)
    {
        mBdAddr = mCdaDevice.GetAddress();
        if (mBdAddr.empty())
        {
            ThrowError("Bluetooth address must be provided");
        }
    }
    else
    {
        mBdAddr = mpBdAddrMgr->GetNext();
    }

    string keyName;
    ostringstream value;

    if (mCdaDevice.UsesNewRadioApis())
    {
        keyName = "BD_ADDRESS";

        // Value example: "00025b123456" becomes "[56 34 12 5b 02 00]"
        value << '[';

        for (string::const_reverse_iterator it = mBdAddr.rbegin();
            it != mBdAddr.rend();
            it += 2)
        {
            value << *(it + 1) << *it;

            if (it != mBdAddr.rend() - 2)
            {
                value << ' ';
            }
        }

        value << ']';
    }
    else
    {
        keyName = "PSKEY_BDADDR";

        // Value example: "00025b123456" becomes "{0x123456,0x5B,0x0002}"
        string nap = mBdAddr.substr(0, 4);
        string uap = mBdAddr.substr(4, 2);
        string lap = mBdAddr.substr(6, 6);

        value << "{0x" << lap << ",0x" << uap << ",0x" << nap << '}';
    }

    mCdaDevice.ConfigSetValue("bt", keyName, value.str());

    ostringstream msg;
    msg << "Bluetooth device address set to 0x" << mBdAddr;
    WriteLog(msg.str(), false);

    if (!mUserProvidedAddress)
    {
        // Successfully wrote Bluetooth address to device, need to mark as used
        mpBdAddrMgr->MarkUsed();
    }

    // Update the record with the assigned Bluetooth address
    mpBdAddrMgr->UpdateRecord(mCdaDevice.GetSerialNum()->ToString(), mBdAddr);

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CSetBdNameTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaSetBdNameTest::NAME = "SET-BDNAME";

CCdaSetBdNameTest::CCdaSetBdNameTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut),
      mBdDeviceName(GetSetup().GetValue("DeviceName", true))
{
    if (mBdDeviceName.empty())
    {
        ThrowError("Bluetooth device name must be set");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaSetBdNameTest::Execute()
{
    CPtTimer testTimer(mName);

    string keyName;
    string subsys;
    if (mCdaDevice.UsesNewRadioApis())
    {
        subsys = "app";
        keyName = "DeviceName";
    }
    else
    {
        subsys = "bt";
        keyName = "PSKEY_DEVICE_NAME";
    }

    // Quoted string required
    ostringstream value;
    value << "\"" << mBdDeviceName << "\"";

    mCdaDevice.ConfigSetValue(subsys, keyName, value.str());

    ostringstream msg;
    msg << "Bluetooth device name set to " << value.str();
    WriteLog(msg.str(), false);

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CSetProdTestModeTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaSetProdTestModeTest::NAME = "SET-PTMODE";

CCdaSetProdTestModeTest::CCdaSetProdTestModeTest(const CStation& aStation,
    CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut),
      mProdTestPsKeyId(GetSetup().GetValueNum<uint16>("ProdTestPsKey"))
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaSetProdTestModeTest::Execute()
{
    const uint16 PT_MODE_ENABLE_VALUE = 1;

    CPtTimer testTimer(mName);

    mCdaDevice.PsSetValue(mProdTestPsKeyId, 1, &PT_MODE_ENABLE_VALUE);

    ostringstream msg;
    msg << "DUT production test mode enabled (PSKEY ID = " << mProdTestPsKeyId << ")";
    WriteLog(msg.str(), false);

    // Reset the DUT to activate production test mode
    mCdaDevice.Reset(CCdaDevice::ResetMode::WAIT);

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CSetSnTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaSetSnTest::NAME = "SET-SN";

CCdaSetSnTest::CCdaSetSnTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaSetSnTest::Execute()
{
    CPtTimer testTimer(mName);

    string sn = mCdaDevice.GetSerialNum()->ToString();
    if (sn.empty())
    {
        ThrowError("Serial number must be provided");
    }

    // Must be quoted string
    ostringstream value;
    value << "\"" << sn << "\"";
    mCdaDevice.ConfigSetValue("app", "USBSerialNumberString", value.str());

    ostringstream msg;
    msg << "Serial number set to " << sn;
    WriteLog(msg.str(), false);

    return ReportResult(true);
}


////////////////////////////////////////////////////////////////////////////////
// CCheckSnTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaCheckSnTest::NAME = "CHECK-SN";

CCdaCheckSnTest::CCdaCheckSnTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaCheckSnTest::Execute()
{
    CPtTimer testTimer(mName);

    string sn = mCdaDevice.GetSerialNum()->ToString();
    if (sn.empty())
    {
        ThrowError("Serial number must be provided");
    }

    bool pass = true;

    string dutSn = mCdaDevice.ConfigGetValue("app", "USBSerialNumberString");
    if (dutSn.empty())
    {
        pass = false;

        WriteLog("DUT SN is not set", true);
    }
    else
    {
        if (dutSn.size() <= 2 || dutSn.at(0) != '\"' || dutSn.at(dutSn.size() - 1) != '\"')
        {
            pass = false;
            ostringstream msg;
            msg << "DUT SN (" << dutSn << ") is invalid. Expecting double quoted value.";
            WriteLog(msg.str(), true);
        }
        else
        {
            // Remove the quotes for the comparison
            dutSn.erase(0, 1);
            dutSn.erase(dutSn.size() - 1);

            if (sn == dutSn)
            {
                ostringstream msg;
                msg << "DUT SN is " << dutSn;
                WriteLog(msg.str(), false);
            }
            else
            {
                pass = false;

                ostringstream msg;
                msg << "Provided SN (" << sn << ") does not match DUT SN (" << dutSn << ")";
                WriteLog(msg.str(), false);
            }
        }
    }

    return ReportResult(pass);
}


////////////////////////////////////////////////////////////////////////////////
// CCfgMergeTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaCfgMergeTest::NAME = "CFG-MERGE";

CCdaCfgMergeTest::CCdaCfgMergeTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
    static const string MIB_FILE_SETTING = "CfgMergeMibFile";

    mMergeFile = GetSetup().GetValue(MIB_FILE_SETTING, false);
    if (!mMergeFile.empty())
    {
        // Check exists and is readable
        if (!PtUtil::FileExists(mMergeFile))
        {
            ostringstream msg;
            msg << MIB_FILE_SETTING << " \"" << mMergeFile 
                << "\" cannot be opened for read (or does not exist)";
            ThrowError(msg.str());
        }
    }

    GetPsKeys();

    if (mMergeFile.empty() && mPsKeys.empty())
    {
        ThrowError("No configuration merge file or PS keys specified");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaCfgMergeTest::Execute()
{
    CPtTimer testTimer(mName);

    // MIB keys
    if (!mMergeFile.empty())
    {
        mCdaDevice.ConfigMerge(mMergeFile);

        ostringstream msg;
        msg << "Merged configuration key(s) from \"" << mMergeFile << "\"";
        WriteLog(msg.str(), false);
    }

    // PS store keys
    if (!mPsKeys.empty())
    {
        for (vector<CPtPsStoreKey>::const_iterator iKey = mPsKeys.begin();
            iKey != mPsKeys.end();
            ++iKey)
        {
            if (iKey->IsAudioKey())
            {
                mCdaDevice.PsAudioSetValue(iKey->Id(),
                    static_cast<uint16>(iKey->Value().size()),
                    (iKey->Value().empty() ? NULL : &(iKey->Value()[0])));
            }
            else
            {
                mCdaDevice.PsSetValue(static_cast<uint16>(iKey->Id()),
                    static_cast<uint16>(iKey->Value().size()),
                    (iKey->Value().empty() ? NULL : &(iKey->Value()[0])));
            }
            
            LogPsKeyWrite(*iKey);
        }
    }

    return ReportResult(true);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaCfgMergeTest::GetPsKeys()
{
    static const string PS_SETTING = "CfgMergePs";

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
                CPtPsStoreKey psKey(setting, true);
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

void CCdaCfgMergeTest::LogPsKeyWrite(const CPtPsStoreKey& aKey)
{
    ostringstream msg;
    msg << "Wrote PS store setting \"" << aKey.ToString() << "\" to device";
    WriteLog(msg.str(), false);
}


////////////////////////////////////////////////////////////////////////////////
// CCdaI2CTest
////////////////////////////////////////////////////////////////////////////////

const std::string CCdaI2CTest::BUSES_SETTING_NAME = "I2cBuses";

CCdaI2CTest::CCdaI2CTest(const std::string& aName,
    const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(aName, aStation, aDut)
{
    GetBuses();
}

////////////////////////////////////////////////////////////////////////////////

void CCdaI2CTest::GetBuses()
{
    static const regex REGEXP_TEST_DEF("(\\d+):(\\d+):(\\d+)");

    vector<string> buses = GetSetup().GetValueList(BUSES_SETTING_NAME, true);

    for (vector<string>::const_iterator iStr = buses.begin();
        iStr != buses.end();
        ++iStr)
    {
        string bus = *iStr;
        PtUtil::TrimString(bus);

        cmatch match;
        if (!regex_match(bus.c_str(), match, REGEXP_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << BUSES_SETTING_NAME << "\", value \""
                << bus << "\" is invalid";
            ThrowError(msg.str());
        }

        I2cBus i2cBus;

        istringstream issScl(match.str(1));
        if (!(issScl >> i2cBus.pioScl) || !issScl.eof())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << BUSES_SETTING_NAME << "\", value \""
                << bus << "\" SCL PIO is invalid (must be an unsigned decimal < "
                << numeric_limits<uint16>::max() << ")";
            ThrowError(msg.str());
        }

        istringstream issSda(match.str(2));
        if (!(issSda >> i2cBus.pioSda) || !issSda.eof())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << BUSES_SETTING_NAME << "\", value \""
                << bus << "\" SDA PIO is invalid (must be an unsigned decimal < "
            << numeric_limits<uint16>::max() << ")";
            ThrowError(msg.str());
        }

        istringstream issClock(match.str(3));
        if (!(issClock >> i2cBus.clockKhz) || !issClock.eof())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << BUSES_SETTING_NAME << "\", value \""
                << bus << "\" clock speed is invalid (must be an unsigned decimal < "
                << numeric_limits<uint16>::max() << ")";
            ThrowError(msg.str());
        }

        mBuses.push_back(i2cBus);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaI2CTest::TestSensor(const I2cConnTest& aTest)
{
    bool result = false;

    // Note: bus index value should have been checked to be in range when the settings were fetched.
    assert(aTest.busIndex < mBuses.size());
    const I2cBus& bus = mBuses.at(aTest.busIndex);

    vector<uint8> read = mCdaDevice.I2CTransfer(bus.pioScl, bus.pioSda, bus.clockKhz,
        aTest.devAddr, aTest.write, static_cast<uint16>(aTest.read.size()));

    ostringstream readMsg;
    // Note that the device address is actually a single octet
    readMsg << "Read from bus " << static_cast<uint16>(aTest.busIndex) << ", device address 0x"
        << hex << uppercase << setfill('0') << setw(2) << aTest.devAddr << " =";
    for (const uint8& octet : read)
    {
        readMsg << " " << setw(2) << static_cast<uint16>(octet);
    }
    WriteLog(readMsg.str(), false);

    ostringstream statusMsg;
    statusMsg << "Test \"" << aTest.name << "\" ";
    if (read == aTest.read)
    {
        statusMsg << "PASSED";
        result = true;
    }
    else
    {
        statusMsg << "FAILED (expected read =" << hex << uppercase << setfill('0');
        for (const uint8& octet : aTest.read)
        {
            statusMsg << " " << setw(2) << static_cast<uint16>(octet);
        }
        statusMsg << ")";
    }
    WriteLog(statusMsg.str(), true);

    return result;
}


////////////////////////////////////////////////////////////////////////////////
// CI2CDeviceTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaI2CDevicesTest::NAME = "I2C-DEVS";

CCdaI2CDevicesTest::CCdaI2CDevicesTest(const CStation& aStation,
    CCdaDevice& aDut)
    : CCdaI2CTest(NAME, aStation, aDut)
{
    GetTests();

    if (mTests.empty())
    {
        ThrowError("No I2C device tests specified");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaI2CDevicesTest::Execute()
{
    static const size_t MAX_RETRIES = 2;
   
    CPtTimer testTimer(mName);

    bool result = true;
   
    for (const I2cConnTest& test : mTests)
    {
        bool pass = false;
        size_t attempts = 0;
        do
        {
            pass = TestSensor(test);
            ++attempts;
            if (!pass && attempts < MAX_RETRIES)
            {
                WriteLog("Retrying", true);
            }
        } while (!pass && attempts < MAX_RETRIES);

        result = pass && result;

        if (!result && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(result);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaI2CDevicesTest::GetTests()
{
    static const string TESTS_SETTING = "I2cConnTests";
    static const regex REGEXP_TEST_DEF("(.+):(\\d+):(\\w+):\\[\\s*(.*)\\s*\\]:\\[\\s*(.+)\\s*\\]");

    vector<string> tests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = tests.begin();
        iStr != tests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        I2cConnTest i2cTest;
        i2cTest.name = match.str(1);

        // Convert the bus index string to a value
        istringstream issBus(match.str(2));
        uint16 busIndex = 0; // Using uint16 to avoid interpretation as char
        // Can't exceed number of specified buses
        if (!(issBus >> busIndex) || !issBus.eof() || busIndex > 0xFF || busIndex >= mBuses.size())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \"" << test 
                << "\" bus index is invalid (must be a decimal value less than the number of buses specified in the \""
                << BUSES_SETTING_NAME << "\" setting (" << mBuses.size() << "))";
            ThrowError(msg.str());
        }

        i2cTest.busIndex = static_cast<uint8>(busIndex);

        // Convert the device address string to a value
        istringstream issAddr(match.str(3));
        // Address should actually be a single octet
        if (!(issAddr >> hex >> i2cTest.devAddr) || !issAddr.eof() || i2cTest.devAddr > 0xFF)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" device address is invalid (must be a hex octet)";
            ThrowError(msg.str());
        }

        try
        {
            CPtSetup::ExtractHexOctetsFromString(match.str(4), i2cTest.write);
            CPtSetup::ExtractHexOctetsFromString(match.str(5), i2cTest.read);
        }
        catch (CPtException& ex)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid (" << ex.what() << ")";
            ThrowError(msg.str());
        }

        mTests.push_back(i2cTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCdaI2CTouchpadTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaI2CTouchpadTest::NAME = "I2C-TOUCH";

CCdaI2CTouchpadTest::CCdaI2CTouchpadTest(const CStation& aStation,
    CCdaDevice& aDut)
    : CCdaI2CTest(NAME, aStation, aDut)
{
    GetTests();

    if (mTests.empty())
    {
        ThrowError("No I2C touchpad tests specified");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaI2CTouchpadTest::Execute()
{
    static const uint32 PIO_DIRECTION = ~0U; // Direction for all masked PIOs - '1' (output).
    static const uint32 PIO_VALUE_ASSERT_RESET = 0; // Pull low to reset
    static const milliseconds RESET_HOLD_MS(50);

    CPtTimer testTimer(mName);

    bool result = true;

    for (const I2cTouchTest& test : mTests)
    {
        uint16 pioBank = test.resetPio / 32; // The PIO bank (banks are 32 lines wide)
        uint32 pioMask = 1UL << (test.resetPio & 0x1F); // The line in the bank

        mCdaDevice.PioMap(pioBank, pioMask, pioMask);

        // Toggle the reset line, with hold period
        mCdaDevice.PioSet(pioBank, pioMask, PIO_DIRECTION, PIO_VALUE_ASSERT_RESET);
        this_thread::sleep_for(RESET_HOLD_MS);
        mCdaDevice.PioSet(pioBank, pioMask, PIO_DIRECTION, ~PIO_VALUE_ASSERT_RESET);

        // Allow time for the touchpad to come out of reset and take a reading
        this_thread::sleep_for(milliseconds(test.resetWaitMs));

        result = TestSensor(test) && result;

        if (!result && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(result);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaI2CTouchpadTest::GetTests()
{
    static const string TESTS_SETTING = "I2cTouchTests";
    static const regex REGEXP_TEST_DEF("(.+):(\\w+):(\\w+):(\\d+):(\\w+):\\[\\s*(.*)\\s*\\]:\\[\\s*(.+)\\s*\\]");

    vector<string> tests = GetSetup().GetValueList(TESTS_SETTING, true);

    for (vector<string>::const_iterator iStr = tests.begin();
        iStr != tests.end();
        ++iStr)
    {
        string test = *iStr;
        PtUtil::TrimString(test);

        cmatch match;
        if (!regex_match(test.c_str(), match, REGEXP_TEST_DEF))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid";
            ThrowError(msg.str());
        }

        I2cTouchTest i2cTest;
        i2cTest.name = match.str(1);

        // Convert the reset PIO string to a value
        istringstream issPio(match.str(2));
        // Reset PIO should be <= 255
        if (!(issPio >> i2cTest.resetPio) || !issPio.eof() || i2cTest.resetPio > 0xFF)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" reset PIO is invalid (must be decimal, <= 255)";
            ThrowError(msg.str());
        }

        // Convert the reset wait string to a value
        istringstream issRstWait(match.str(3));
        if (!(issRstWait >> i2cTest.resetWaitMs) || !issRstWait.eof())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" reset wait is invalid (must be decimal, <= 65535)";
            ThrowError(msg.str());
        }

        // Convert the bus index string to a value
        istringstream issBus(match.str(4));
        uint16 busIndex = 0; // Using uint16 to avoid interpretation as char
        // Can't exceed number of specified buses
        if (!(issBus >> busIndex) || !issBus.eof() || busIndex > 0xFF || busIndex >= mBuses.size())
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \"" << test
                << "\" bus index is invalid (must be a decimal value less than the number of buses specified in the \""
                << BUSES_SETTING_NAME << "\" setting (" << mBuses.size() << "))";
            ThrowError(msg.str());
        }

        i2cTest.busIndex = static_cast<uint8>(busIndex);

        // Convert the device address string to a value
        istringstream issAddr(match.str(5));
        // Address should actually be a single octet
        if (!(issAddr >> hex >> i2cTest.devAddr) || !issAddr.eof() || i2cTest.devAddr > 0xFF)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" device address is invalid (must be a hex octet)";
            ThrowError(msg.str());
        }

        try
        {
            CPtSetup::ExtractHexOctetsFromString(match.str(6), i2cTest.write);
            CPtSetup::ExtractHexOctetsFromString(match.str(7), i2cTest.read);
        }
        catch (CPtException& ex)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << TESTS_SETTING << "\", value \""
                << test << "\" is invalid (" << ex.what() << ")";
            ThrowError(msg.str());
        }

        mTests.push_back(i2cTest);
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCdaUserHcTest
////////////////////////////////////////////////////////////////////////////////

const char* const CCdaUserHcTest::NAME = "USER-HC";

CCdaUserHcTest::CCdaUserHcTest(const CStation& aStation, CCdaDevice& aDut)
    : CCdaDevTest(NAME, aStation, aDut)
{
    if (GetSetup().GetValueNum<bool>(CCdaDevice::APP_DISABLE_SETTING_NAME))
    {
        ostringstream msg;
        msg << NAME << " test cannot be run with the device application disabled (update the \""
            << CCdaDevice::APP_DISABLE_SETTING_NAME << "\" setting)";
        ThrowError(msg.str());
    }
    
    GetConfig();

    if (mTests.empty())
    {
        ThrowError("No tests specified");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaUserHcTest::Execute()
{
    CPtTimer testTimer(mName);

    bool pass = true;
    for (const UserTest& test : mTests)
    {
        mCdaDevice.AppWrite(test.channel, test.message);

        ostringstream msg;
        msg << "Check " << test.description;
        pass = AskUserCheck(msg.str()) && pass;

        if (!pass && GetStation().StopOnFail())
        {
            break;
        }
    }

    return ReportResult(pass);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaUserHcTest::GetConfig()
{
    static const string SETTING_NAME = "UserHcTests";
    static const regex REGEXP_TEST_DEF("(.+):(\\w+):\\[\\s*(.*)\\s*\\]");

    vector<string> tests = GetSetup().GetValueList(SETTING_NAME, false);
    if (!tests.empty())
    {
        for (vector<string>::const_iterator iStr = tests.begin();
            iStr != tests.end();
            ++iStr)
        {
            string test = *iStr;
            PtUtil::TrimString(test);

            cmatch match;
            if (!regex_match(test.c_str(), match, REGEXP_TEST_DEF))
            {
                ostringstream msg;
                msg << "Configuration setting \"" << SETTING_NAME << "\", value \""
                    << test << "\" is invalid";
                ThrowError(msg.str());
            }

            UserTest userTest;
            userTest.description = match.str(1);

            // Convert the channel string to a value
            istringstream issChan(match.str(2));
            uint16 chan = 0; // Using uint16 to avoid interpretation as char
            if (!(issChan >> chan) || !issChan.eof() ||
                chan > CCdaDevice::HOST_COMMS_CHANNEL_MAX)
            {
                ostringstream msg;
                msg << "Configuration setting \"" << SETTING_NAME << "\", value \""
                    << test << "\" msg_channel is invalid (must be decimal, <= "
                    << static_cast<uint16>(CCdaDevice::HOST_COMMS_CHANNEL_MAX) << ")";
                ThrowError(msg.str());
            }

            userTest.channel = static_cast<uint8>(chan);

            // Split the message string
            vector<string> values = PtUtil::SplitString(match.str(3).c_str(), " ");

            // Remove any empty entries (e.g. from accidental double separators between values)
            auto EmptyString = [](string const& aStr) { return aStr.empty(); };
            values.erase(remove_if(values.begin(), values.end(), EmptyString), values.end());

            // Convert the value strings into a vector of uint16 values
            for (vector<string>::const_iterator iValStr = values.begin();
                iValStr != values.end();
                ++iValStr)
            {
                istringstream iss(*iValStr);
                uint16 val = 0;
                if (iss.str().find('-') != string::npos ||
                    !(iss >> hex >> val) || !iss.eof())
                {
                    ostringstream msg;
                    msg << "Configuration setting \"" << SETTING_NAME << "\", value \""
                        << test << "\" msg_data value \"" << *iValStr 
                        << "\" is not a valid unsigned 16-bit hex value";
                    ThrowError(msg.str());
                }

                userTest.message.push_back(val);
            }

            mTests.push_back(userTest);
        }
    }
}
