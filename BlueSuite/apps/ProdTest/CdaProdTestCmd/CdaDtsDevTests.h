//**************************************************************************************************
//
//  CdaDtsDevTests.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA DTS device test class declarations, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CDA_DTS_DEV_TESTS_H
#define CDA_DTS_DEV_TESTS_H

#include "Test.h"
#include "common\types.h"
#include "CdaDtsDevice.h"
#include "PtPsStoreKey.h"
#include <string>
#include <vector>
#include <map>

class CSpectrumAnalyserInstrument;

///
/// CDA DTS device test factory
///
class CCdaDtsDevTestFactory
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
    CCdaDtsDevTestFactory();

    ///
    /// The supported test names.
    ///
    static const std::vector<std::string> mSupportedTests;
};


///
/// CDA DTS device test base class
///
class CCdaDtsDevTest : public CTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsDevTest() {};

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsDevTest(const std::string& aName, const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Gets a test timeout setting.
    /// @param[in] aSettingName The name of the timeout setting.
    /// @return Test timeout in seconds.
    /// @throws CPtException.
    ///
    uint16 GetTestTimeoutS(const std::string& aSettingName);

    ///
    /// The CDA Device object.
    ///
    CCdaDtsDevice& mCdaDtsDevice;
};


///
/// LED test class
///
class CCdaDtsLedTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsLedTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsLedTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the LED tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<LED ID>:<description>", where the ID must be between 0 and 7.
    /// For example "0:Green,1:Red".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// LED test data
    ///
    struct LedTest
    {
        LedTest() : id(0) {};

        uint8 id; //!< LED id
        std::string description; //!< Description of the LED
    };

    ///
    /// LED tests
    ///
    std::vector<LedTest> mTests;
};


///
/// Audio tone test class
///
class CCdaDtsAudioToneTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsAudioToneTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsAudioToneTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the audio tone tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<channel>:<tone>"
    /// channel indicates the channel to play the tone on, where 0 = Channel A (L),
    /// 1 = Channel B (R), and 2 = Channels A & B(L & R).
    /// tone is a value between 0 and 119 indicating the note to play, where
    /// 0 is C0 (low) and 119 is B9 (high) (see device file ringtone_notes.h).
    /// For example to test L & R separately with a C6 tone: "72:0,72:1".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// Tone test data
    ///
    struct ToneTest
    {
        ToneTest() : channel(CCdaDtsDevice::AudioChannel::LEFT), tone(0) {};

        CCdaDtsDevice::AudioChannel channel; //!< Channel to test on
        uint8 tone; //!< Tone to use for the test
    };

    ///
    /// Tone tests
    ///
    std::vector<ToneTest> mTests;
};


///
/// Audio loop test class
///
class CCdaDtsAudioLoopTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsAudioLoopTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsAudioLoopTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the audio loop tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<desc>:<mic>:<out_hardware>:<out_instance>:<out_channel>"
    /// desc is the descriptive string for the loop.
    /// mic indicates the microphone number for input (1-n, where n is device dependent).
    ///   (see device file microphones.h, microphone_number_t enum).
    /// out_hardware indicates the output hardware to use.
    /// out_instance indicates the output instance to use
    ///   (see device enum audio_instance (audio_if.h)).
    /// out_channel indicates the output channel.
    /// E.g.to test 4 mics through 1 speaker:
    /// "mic1-spkr:1:3:0:0, mic2-spkr:2:3:0:0, mic3-spkr:3:3:0:0, mic4-spkr:4:3:0:0".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// Loop test data
    ///
    struct LoopTest
    {
        LoopTest() : mic(0), outHardware(CCdaDtsDevice::AudioHardware::CODEC),
            outInstance(0), outChannel(CCdaDtsDevice::AudioChannel::LEFT) {};

        std::string desc; //!< Description of the loop
        uint8 mic; //!< Mic ID
        CCdaDtsDevice::AudioHardware outHardware; //!< Output hardware
        uint8 outInstance; //!< Output interface
        CCdaDtsDevice::AudioChannel outChannel; //!< Output channel
    };

    ///
    /// Loop tests
    ///
    std::vector<LoopTest> mTests;
};


///
/// Touch sensor test class
///
class CCdaDtsTouchpadTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsTouchpadTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsTouchpadTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    typedef std::pair<std::string, CCdaDtsDevice::TouchType> TouchActionPair;
    typedef std::map<TouchActionPair::first_type, TouchActionPair::second_type> TouchActionMap;

    ///
    /// Touch actions map, mapping mode strings to enumerated action values.
    ///
    static const TouchActionMap mSupportedActionsMap;

    ///
    /// The touch actions to test
    ///
    std::vector<TouchActionPair> mActions;

    ///
    /// The timeout to use for each test.
    ///
    uint16 mTestTimeoutSeconds;
};


///
/// Proximity sensor test class
///
class CCdaDtsProximityTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsProximityTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsProximityTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// The timeout to use for the test.
    ///
    uint16 mTestTimeoutSeconds;
};


///
/// Hall sensor test class
///
class CCdaDtsHallSensorTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsHallSensorTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsHallSensorTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// The timeout to use for the test.
    ///
    uint16 mTestTimeoutSeconds;
};


///
/// Temperature sensor test class
///
class CCdaDtsTemperatureTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsTemperatureTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsTemperatureTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// The low limit to use for the test.
    ///
    int16 mLowLimit;

    ///
    /// The high limit to use for the test.
    ///
    int16 mHighLimit;
};


///
/// Battery test class
///
class CCdaDtsBatteryTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsBatteryTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsBatteryTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// The low limit to use for the test.
    ///
    uint16 mLowLimit;

    ///
    /// The high limit to use for the test.
    ///
    uint16 mHighLimit;
};


///
/// RSSI test class
///
class CCdaDtsRssiTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsRssiTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsRssiTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// The low limit to use for the test.
    ///
    int16 mLowLimit;

    ///
    /// The high limit to use for the test.
    ///
    int16 mHighLimit;
};


///
/// RF power test class
///
class CCdaDtsRfPowerTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsRfPowerTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsRfPowerTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the frequency for a given Bluetooth (BR/EDR) channel.
    /// @param[in] aChannel BR/EDR channel number.
    /// @return Frequency in MHz.
    ///
    uint16 GetBtFreqMhz(uint16 aChannel);

    ///
    /// Supported RF stop methods.
    ///
    enum class RfStopMethod
    {
        TIME,
        PIO,
        TOUCH
    };

    typedef std::pair<std::string, RfStopMethod> RfStopMethodPair;
    typedef std::map<RfStopMethodPair::first_type, RfStopMethodPair::second_type> RfStopMethodMap;

    ///
    /// RF stop methods map, mapping method strings to enumerated values.
    ///
    static const RfStopMethodMap mSupportedStopMethodsMap;

    ///
    /// Holds details of the RF test is to be stopped.
    /// 
    struct RfStopData
    {
        RfStopData() : method(RfStopMethod::TIME), param(0) {};
        RfStopMethod method;
        uint8 param; // either the time in seconds or PIO ID depending on the method.
        std::string pioDesc;
    } mRfStopData;

    ///
    /// The spectrum analyser object.
    ///
    CSpectrumAnalyserInstrument* mpSpecAn;

    ///
    /// The BT RF channel to use.
    ///
    uint16 mChannel;

    ///
    /// The expected output power in dBm
    ///
    float64 mExpPowerDbm;

    ///
    /// The output power margin in dBm
    ///
    float64 mPowerMarginDbm;

    ///
    /// The reboot wait time in seconds.
    ///
    uint16 mRebootWaitS;
};


///
/// Configuration set test class
///
class CCdaDtsCfgSetTest : public CCdaDtsDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDtsCfgSetTest(const CStation& aStation, CCdaDtsDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsCfgSetTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the PS store keys to set from settings.
    /// The format of the setting string is <setting>,...,<setting>,
    /// The format for a setting is: <name|id>=[val0Lsb val0Msb val1Lsb val1Msb ...]
    /// E.g.: "USR3=[01 00],USR5=[47 20 2E 00]".
    /// The optional "apps:" prefix can be used, e.g.: "apps:USR3=[01 00]".
    /// @throws CPtException.
    ///
    void GetPsKeys();

    ///
    /// Logs a PS store key write.
    /// @param[in] aKey The PS key.
    ///
    void LogPsKeyWrite(const CPtPsStoreKey& aKey);

    ///
    /// PS keys to set.
    ///
    std::vector<CPtPsStoreKey> mPsKeys;
};

#endif // CDA_DTS_DEV_TESTS_H
