//**************************************************************************************************
//
//  CdaDevTests.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA device test class declarations, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CDA_DEV_TESTS_H
#define CDA_DEV_TESTS_H

#include "Test.h"
#include "common\types.h"
#include "PtSetup.h"
#include "PtPsStoreKey.h"
#include "CdaDevice.h"
#include <string>
#include <vector>

class CSpectrumAnalyserInstrument;
class CPtBdAddrMgr;

///
/// CDA device test factory
///
class CCdaDevTestFactory
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
    CCdaDevTestFactory();

    ///
    /// The supported test names.
    ///
    static const std::vector<std::string> mSupportedTests;
};


///
/// CDA device test base class
///
class CCdaDevTest : public CTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CCdaDevTest() {};

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaDevTest(const std::string& aName, const CStation& aStation, CCdaDevice& aDut);

    ///
    /// The CDA Device object.
    ///
    CCdaDevice& mCdaDevice;
};


///
/// CDA RF test base class
///
class CCdaRfTest : public CCdaDevTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CCdaRfTest() {};

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaRfTest(const std::string& aName, const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Gets the frequency for a given Bluetooth (BR/EDR) channel.
    /// @param[in] aChannel BR/EDR channel number.
    /// @return Frequency in MHz.
    ///
    uint16 GetBtFreqMhz(uint16 aChannel);

    ///
    /// The spectrum analyser object.
    ///
    CSpectrumAnalyserInstrument* mpSpecAn;

    ///
    /// The BT RF channel to use.
    ///
    uint16 mChannel;
};


///
/// Xtal trim test class
///
class CCdaXtalTrimTest : public CCdaRfTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaXtalTrimTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaXtalTrimTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    static const uint16 MAX_XTAL_LCAP = 31; //!< Maximum XTAL load capacitance value
    static const uint16 MIN_XTAL_LCAP = 0; //!< Minimum XTAL load capacitance value

    static const int16 MAX_XTAL_TRIM = 15; //!< Maximum XTAL trim value
    static const int16 MIN_XTAL_TRIM = -16; //!< Minimum XTAL trim value

    ///
    /// Calculates Xtal trim values using an incremental search.
    /// @param[in] aFreqMHz Frequency used for the trim process in MHz.
    /// @param[out] aLoadCap Load capacitance value (coarse trim).
    /// @param[out] aTrim Trim value (fine trim).
    /// @return true if suitable Xtal trim values are found, false otherwise.
    /// @throws CPtException.
    ///
    bool CalcXtalTrimIncremental(uint32 aFreqMHz, uint16& aLoadCap, int16& aTrim);

    ///
    /// Calculates Xtal trim values using a binary-chop search.
    /// @param[in] aFreqMHz Frequency used for the trim process in MHz.
    /// @param[out] aLoadCap Load capacitance value (coarse trim).
    /// @param[out] aTrim Trim value (fine trim).
    /// @return true if suitable Xtal trim values are found, false otherwise.
    /// @throws CPtException.
    ///
    bool CalcXtalTrimBinChop(uint32 aFreqMHz, uint16& aLoadCap, int16& aTrim);

    ///
    /// Reads the frequency offset using a spectrum analyser.
    /// @param[in] aNomFreqMHz Nominal (expected) frequency in MHz.
    /// @return Frequency offset in Hz.
    /// @throws CPtException.
    ///
    int32 ReadFreqOffsetHz(uint32 aNomFreqMHz);

    ///
    /// Search mode, incremental search used if true,
    /// binary chop search otherwise.
    ///
    bool mIncrementalSearch;

    ///
    /// Coarse trim margin in Hertz.
    ///
    uint16 mTrimMarginCoarseHz;

    ///
    /// Fine trim margin in Hertz.
    ///
    uint16 mTrimMarginFineHz;
};


///
/// RF power test class
///
class CCdaRfPowerTest : public CCdaRfTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaRfPowerTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaRfPowerTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// The expected output power in dBm
    ///
    float64 mExpPowerDbm;

    ///
    /// The output power margin in dBm
    ///
    float64 mPowerMarginDbm;
};


///
/// PIO test base class
///
class CCdaPioTest : public CCdaDevTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CCdaPioTest() {};

protected:

    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    /// @param[in] aSettingName The name of the setting which defines the PIO tests.
    ///
    CCdaPioTest(const std::string& aName, const CStation& aStation,
        CCdaDevice& aDut, const std::string& aSettingName);

    ///
    /// PIO test data
    ///
    struct PioTest
    {
        PioTest() : pio(0), onState(false), firstState(false) {};

        uint8 pio;                  //!< PIO number to test
        bool onState;               //!< PIO state corresponding to on/pressed (false = low, true = high)
        std::string description;    //!< Description of the PIO
        bool firstState;            //!< First state to test (false = off/released, true = on/pressed)
    };

    ///
    /// PIO tests
    ///
    std::vector<PioTest> mTests;
};


///
/// LED test class
///
class CCdaLedTest : public CCdaPioTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaLedTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaLedTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Buttons test class
///
class CCdaButtonsTest : public CCdaPioTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaButtonsTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaButtonsTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Charger test class
///
class CCdaChargerTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaChargerTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaChargerTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Audio tone test class
///
class CCdaAudioToneTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaAudioToneTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaAudioToneTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Audio loop test class
///
class CCdaAudioLoopTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaAudioLoopTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaAudioLoopTest() {};

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
    /// "<desc>:<in_device>:<in_iface>:<in_channel>:<in_mic_pio>:<in_mic_en>:<out_device>:<out_iface>:<out_channel>"
    /// desc is the descriptive string for the loop, the other values should all be uint16s.
    /// in_mic_pio is the PIO to use to enable a mic. For digital microphones, the given PIO is always used. For
    ///   analogue microphones, 0 is treated as a special value causing mic bias to be enabled, no PIO used. A value
    ///   >0 means no mic bias set, PIO driven. This does mean that PIO 0 can't be used to enable an analogue mic.
    /// in_mic_en is the level to set to enable a mic (0 = active low, 1 = active high).
    /// See device file accmd_prim.h for valid device (ACCMD_STREAM_DEVICE), interface
    /// (ACCMD_AUDIO_INSTANCE), and channel (ACCMD_AUDIO_CHANNEL) values.
    /// E.g.to test 2 analogue and 2 digital mics through 1 speaker:
    /// "amic1-spkr:3:0:0:0:0:3:0:0, amic2-spkr:3:0:1:0:0:3:0:0, dmic1-spkr:6:1:0:19:0:3:0:0, dmic2-spkr:6:1:1:19:0:3:0:0".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// Loop test data
    ///
    struct LoopTest
    {
        LoopTest() : inDevice(CCdaDevice::AudioDevice::CODEC),
                     outDevice(CCdaDevice::AudioDevice::CODEC),
                     inIface(0),
                     outIface(0),
                     inChannel(CCdaDevice::AudioChannel::LEFT),
                     outChannel(CCdaDevice::AudioChannel::LEFT),
                     micPio(0),
                     micPioActive(0) {};

        std::string desc;                       //!< Description of the loop
        CCdaDevice::AudioDevice inDevice;       //!< Input device
        CCdaDevice::AudioDevice outDevice;      //!< Output device
        uint16 inIface;                         //!< Input interface
        uint16 outIface;                        //!< Output interface
        CCdaDevice::AudioChannel inChannel;     //!< Input channel
        CCdaDevice::AudioChannel outChannel;    //!< Output channel
        uint16 micPio;                          //!< MIC enable PIO
        uint16 micPioActive;                    //!< Active state for the MIC enable PIO (0/1)
    };

    ///
    /// Loop tests
    ///
    std::vector<LoopTest> mTests;
};


///
/// Set Bluetooth device address test class
///
class CCdaSetBdAddrTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaSetBdAddrTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaSetBdAddrTest();

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Address source - if true, address provided by
    /// user, otherwise pulled from an address file.
    ///
    bool mUserProvidedAddress;
    
    ///
    /// Bluetooth address manager.
    ///
    CPtBdAddrMgr* mpBdAddrMgr;

    ///
    /// Bluetooth address;
    ///
    std::string mBdAddr;
};


///
/// Set Bluetooth device name test class
///
class CCdaSetBdNameTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaSetBdNameTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaSetBdNameTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    ///
    /// Bluetooth device name;
    ///
    std::string mBdDeviceName;
};


///
/// Set device serial number test class
///
class CCdaSetSnTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test
                                   
    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaSetSnTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaSetSnTest() {};

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
class CCdaCheckSnTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaCheckSnTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaCheckSnTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;
};


///
/// Set production test mode test class
///
class CCdaSetProdTestModeTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaSetProdTestModeTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaSetProdTestModeTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:
    
    ///
    /// Production test PS key ID
    ///
    uint16 mProdTestPsKeyId;
};


///
/// Configuration merge test class
///
class CCdaCfgMergeTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaCfgMergeTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaCfgMergeTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the PS store keys to merge from settings.
    /// The format of the setting string is <setting>,...,<setting>,
    /// For audio keys the format for a setting is: audio:<id>=[val0Lsb val0Msb val1Lsb val1Msb ...]
    /// For apps keys the format for a setting is: apps:<name|id>=[val0Lsb val0Msb val1Lsb val1Msb ...]
    /// E.g.: "apps:USR3=[01 00],audio:0x000080=[01 10 01 00 01 00 07 F8 E4 01]".
    /// @throws CPtException.
    ///
    void GetPsKeys();

    ///
    /// Logs a PS store key write.
    /// @param[in] aKey The PS key.
    ///
    void LogPsKeyWrite(const CPtPsStoreKey& aKey);

    ///
    /// Configuration merge file (HTF) path.
    ///
    std::string mMergeFile;

    ///
    /// PS keys to merge.
    ///
    std::vector<CPtPsStoreKey> mPsKeys;
};


///
/// I2C test base class
///
class CCdaI2CTest : public CCdaDevTest
{
public:
    ///
    /// Destructor.
    ///
    virtual ~CCdaI2CTest() {};

protected:
    ///
    /// Constructor.
    /// @param[in] aName The test name.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaI2CTest(const std::string& aName, const CStation& aStation, CCdaDevice& aDut);

    static const std::string BUSES_SETTING_NAME;

    ///
    /// I2C bus configuration
    ///
    struct I2cBus
    {
        I2cBus() : pioScl(0), pioSda(0), clockKhz(0) {};

        uint16 pioScl; //!< PIO used for I2C SCL (clock).
        uint16 pioSda; //!< PIO used for I2C SDA (data).
        uint16 clockKhz; //!< I2C clock speed in kHz.
    };

    ///
    /// I2C buses
    ///
    std::vector<I2cBus> mBuses;

    ///
    /// I2C connectivity test data
    ///
    struct I2cConnTest
    {
        I2cConnTest() : busIndex(0), devAddr(0) {};

        std::string name; //!< The name of the device / test.
        uint8 busIndex; //!< The index (0 based) of the I2C bus the device is attached to.
        uint16 devAddr; //!< The I2C address of the device.
        std::vector<uint8> write; //!< The octets to write to the device.
        std::vector<uint8> read;  //!< The octets expected to be read back from the device.
    };

    ///
    /// Gets the I2C bus configurations from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<pio_scl>:<pio_sda>:<clock_khz>"
    /// pio_scl is the PIO used for I2C SCL (clock).
    /// pio_sda is the PIO used for I2C SDA (data).
    /// clock_khz is the I2C clock speed in kHz.
    /// For example "18:20:400, 17:19:100".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetBuses();

    ///
    /// Run the sensor test
    /// @param[in] aTest The test data.
    /// @return The result of the test (read data matches expected).
    /// @throws CPtException.
    ///
    bool TestSensor(const I2cConnTest& aTest);
};


///
/// I2C devices test class (e.g. for sensors connected via I2C).
///
class CCdaI2CDevicesTest : public CCdaI2CTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaI2CDevicesTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaI2CDevicesTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the I2C connectivity tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<desc>:<bus_index>:<dev_addr>:[<write_octets>]:[<expected_response_octets>]"
    /// desc is a string describing the test or device being tested.
    /// bus_index is a decimal value specifying the index (0 based) of the I2C bus the 
    /// device is attached to.
    /// dev_addr is a hex octet(no prefix) specifying the I2C address of the device.
    /// write_octets is a space separated list of between 0 and 32 octets
    /// (hex, no prefix) to write.
    /// expected_response_octets is a space separated list of between 1 and 32
    /// octets (hex, no prefix) expected to be read from the device after writing
    /// write_bytes.
    /// For example "Prox sensor:0:1E:[7F]:[11], Cap sense:1:44:[01 00]:[06 00]".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();

    ///
    /// I2C connectivity tests
    ///
    std::vector<I2cConnTest> mTests;
};


///
/// I2C touchpad test class for touchpad sensors connected via I2C and using a
/// reset PIO.
///
class CCdaI2CTouchpadTest : public CCdaI2CTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaI2CTouchpadTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaI2CTouchpadTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// I2C touchpad test data
    ///
    struct I2cTouchTest : public I2cConnTest
    {
        I2cTouchTest() : resetPio(0), resetWaitMs(0) {};

        uint16 resetPio; //!< The PIO used to reset the touchpad
        uint16 resetWaitMs; //!< The reset wait time in milliseconds
    };

    ///
    /// I2C touch tests
    ///
    std::vector<I2cTouchTest> mTests;

    ///
    /// Gets the I2C touchpad tests from settings.
    /// The format is a comma-separated list of entries in the form
    /// "<desc>:<reset_pio>:<reset_wait_ms>:<bus_index>:<dev_addr>:[<write_octets>]:[<expected_response_octets>]"
    /// desc is a string describing the test or device being tested.
    /// reset_pio is the PIO number used for touchpad reset (decimal, maximum 255).
    /// reset_wait_ms is the time to wait for the touchpad to reset in milliseconds (decimal, maximum 65535).
    /// bus_index is a decimal value specifying the index (0 based) of the I2C bus the 
    /// device is attached to.
    /// dev_addr is a hex octet(no prefix) specifying the I2C address of the device.
    /// write_octets is a space separated list of between 0 and 32 octets
    /// (hex, no prefix) to write.
    /// expected_response_octets is a space separated list of between 1 and 32
    /// octets (hex, no prefix) expected to be read from the device after writing
    /// write_bytes.
    /// For example "Touchpad L:18:1000:0:08:[]:[FF 03 01], Touchpad R:19:1000:0:09:[]:[FF 03 01]".
    /// A space following the comma separator is optional.
    /// @throws CPtException.
    ///
    void GetTests();
};


///
/// User defined host-comms (app) driven test class
///
class CCdaUserHcTest : public CCdaDevTest
{
public:
    static const char* const NAME; //!< Name of the test

    ///
    /// Constructor.
    /// @param[in] aStation The production test station object.
    /// @param[in] aDut The device under test object.
    ///
    CCdaUserHcTest(const CStation& aStation, CCdaDevice& aDut);

    ///
    /// Destructor.
    ///
    virtual ~CCdaUserHcTest() {};

    ///
    /// Executes the test.
    /// @return true if the test passed, false otherwise.
    /// @throws CPtException.
    ///
    bool Execute() override;

private:

    ///
    /// Gets the test configuration from settings.
    /// The format is a comma-separated list of entries in the form:
    ///   <description>:<msg_channel>:[<msg_data>]
    /// description provides the user with the description of what's being tested or what to check.
    /// msg_channel Indicates the message channel to use (0-127).
    /// msg_data Space separated list of between 1 and 80 16-bit values (hex, no prefix) to write.
    /// E.g. for two tests: "MIC:1:[0001 0002 0001],RF:1:[0001 0002 0002]".
    /// @throws CPtException.
    ///
    void GetConfig();

    ///
    /// Test data
    ///
    struct UserTest
    {
        UserTest() : channel(0) {};

        std::string description;        //!< The description of the test
        uint8 channel;                  //!< The message channel to use
        std::vector<uint16> message;    //!< The message data to write to the device
    };

    ///
    /// Tests
    ///
    std::vector<UserTest> mTests;
};

#endif // CDA_DEV_TESTS_H
