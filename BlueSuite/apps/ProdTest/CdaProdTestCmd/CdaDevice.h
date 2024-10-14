//**************************************************************************************************
//
//  CdaDevice.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA device class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CDA_DEVICE_H
#define CDA_DEVICE_H

#include "common/types.h"
#include "Dut.h"
#include <vector>
#include <map>

///
/// Device class for CDA ICs
///
class CCdaDevice : public CDut
{
public:

    ///
    /// Audio device identifiers we need to know about.
    /// See device file accmd_prim.h for valid device (ACCMD_STREAM_DEVICE) values.
    ///
    enum class AudioDevice
    {
        CODEC = 3,
        DIGITAL_MIC = 6
    };

    ///
    /// Audio channel identifiers we need to know about.
    /// See device file accmd_prim.h for channel (ACCMD_AUDIO_CHANNEL) values.
    ///
    enum class AudioChannel
    {
        LEFT,
        RIGHT,
        LEFT_AND_RIGHT
    };

    static const char* const APP_DISABLE_SETTING_NAME;     //!< The setting name used to switch on/off disabling the device app
    static const uint8 HOST_COMMS_CHANNEL_MAX = 127;       //!< The maximum channel value for host comms messages
    static const uint8 HOST_COMMS_WRITE_MSG_LEN_MIN = 1;   //!< The minimum length of data written to a device using host comms
    static const uint8 HOST_COMMS_WRITE_MSG_LEN_MAX = 80;  //!< The maximum length of data written to a device using host comms

    ///
    /// Transports map type mapping port strings (e.g. "USBDBG(100)")
    /// to transport options strings (e.g. "SPITRANS=USBDBG SPIPORT=1").
    ///
    typedef std::map< std::string, std::string > TransportsMap;

    ///
    /// Gets the available transports.
    /// @param[out] aTransports The available transports.
    /// @throws CDutException.
    ///
    static void GetAvailableTransports(TransportsMap& aTransports);

    ///
    /// Constructor.
    /// @param[in] aSetup Production test setup object.
    ///
    explicit CCdaDevice(const CPtSetup& aSetup);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDevice();

    ///
    /// Connects to the device.
    /// @throws CDutException.
    ///
    void Connect() override;

    ///
    /// Disconnects from the device.
    ///
    void Disconnect() override;

    ///
    /// Resets the device.
    /// @param[in] aMode The reset mode.
    /// @throws CDutException.
    ///
    void Reset(ResetMode aMode) override;

    ///
    /// Disables the application running on the device.
    /// @throws CDutException.
    ///
    void DisableApp();

    ///
    /// Performs any necessary actions prior to running tests.
    /// @throws CDutException.
    ///
    void PreTestActions() override;

    ///
    /// Performs any necessary actions after all tests have been run.
    /// @param[in] aTestsPassed true if all tests passed, false otherwise.
    /// @throws CDutException.
    ///
    void PostTestActions(bool aTestsPassed) override;

    ///
    /// Validates and sets the device (BT) address.
    /// @param[in] aBdAddress The Bluetooth address to set as a string of
    ///   12 hexadecimal characters (optional "0x" prefix).
    ///
    void SetAddress(const std::string& aBtAddress) override;

    ///
    /// Indicates if the device supports the QHCI radio APIs.
    /// @return true if the QHCI based radio APIs are supported, false
    /// otherwise (radiotest APIs supported instead).
    ///
    bool UsesNewRadioApis() const { return mUsesQHciRadioApis; };

    ///
    /// Starts carrier wave transmission.
    /// @param[in] aChannel Bluetooth channel number to transmit on.
    /// @param[in] aFreqMHz The frequency in MHz to transmit on.
    /// @throws CDutException.
    ///
    void RadioTxCwStart(uint16 aChannel, uint16 aFreqMhz);

    ///
    /// Stops any running radio test mode.
    /// @throws CDutException.
    ///
    void RadioStop();

    ///
    /// Sets the Xtal load capacitance (coarse trim) value.
    /// Takes effect immediately.
    /// @param[in] aLoadCapVal Load capacitance value (0 to 31).
    /// @throws CDutException.
    ///
    void XtalSetLoadCap(uint16 aLoadCapVal);

    ///
    /// Sets the Xtal (fine) trim value.
    /// Takes effect immediately.
    /// @param[in] aTrimVal Load capacitance value (-16 to 15).
    /// @throws CDutException.
    ///
    void XtalSetTrim(int16 aTrimVal);

    ///
    /// Merges configuration settings into the cache (cache is first populated
    /// from the device if that hasn't happened already).
    /// @param[in] aMergeFile Path of a *.htf file containing settings to merge.
    /// @throws CDutException.
    ///
    void ConfigMerge(const std::string& aMergeFile);

    ///
    /// Sets a configuration value.
    /// The value is written to an internal cache, i.e. not written to the device.
    /// @param[in] aSubSys The name of the subsystem the key belongs to.
    /// @param[in] aKeyName The key name.
    /// @param[in] aValue The key value.
    /// @throws CDutException.
    ///
    void ConfigSetValue(const std::string& aSubSys, const std::string& aKeyName,
        const std::string& aValue);

    ///
    /// Gets a configuration value from the internal configuration cache (populating
    /// the internal cache by reading from the device if this hasn't already been done).
    /// @param[in] aSubSys The name of the subsystem the key belongs to.
    /// @param[in] aKeyName The key name.
    /// @return The key value.
    /// @throws CDutException.
    ///
    std::string ConfigGetValue(const std::string& aSubSys, const std::string& aKeyName);

    ///
    /// Sets a PS store key value (non-audio keys).
    /// @param[in] aKeyId The PS key ID.
    /// @param[in] aValueLen The length of the apValue array (number of 16bit words).
    /// @param[in] apValue Pointer to the key value (array).
    /// @throws CDutException.
    ///
    void PsSetValue(uint16 aKeyId, uint16 aValueLen, const uint16 apValue[]);

    ///
    /// Sets a PS store audio key value.
    /// @param[in] aKeyId The PS key ID.
    /// @param[in] aValueLen The length of the apValue array (number of 16bit words).
    /// @param[in] apValue Pointer to the key value (array).
    /// @throws CDutException.
    ///
    void PsAudioSetValue(uint32 aKeyId, uint16 aValueLen, const uint16 apValue[]);

    ///
    /// Maps IO lines as PIOs or their alternative use.
    /// @param[in] aBank The PIO bank.
    /// @param[in] aMask The lines in the bank to map.
    /// @param[in] aPios The masked lines to assign as PIOs (1 = PIO, 0 = alternative).
    /// @throws CDutException.
    ///
    void PioMap(uint16 aBank, uint32 aMask, uint32 aPios);

    ///
    /// Sets the direction and value of PIOs.
    /// @param[in] aBank The PIO bank.
    /// @param[in] aMask The PIOs to set.
    /// @param[in] aDirection The direction to assign to the masked PIOs (1 = output, 0 = input).
    /// @param[in] aValue The values to set for the masked PIOs.
    /// @throws CDutException.
    ///
    void PioSet(uint16 aBank, uint32 aMask, uint32 aDirection, uint32 aValue);

    ///
    /// Gets the value and direction of PIOs.
    /// @param[in] aBank The PIO bank.
    /// @param[out] aDirection The direction of the PIOs (1 = output, 0 = input).
    /// @return The values of the PIOs.
    /// @throws CDutException.
    ///
    uint32 PioGet(uint16 aBank, uint32& aDirection);

    ///
    /// Gets the battery charging status
    /// @return true if the battery charger is active, false otherwise.
    ///
    bool IsCharging();

    ///
    /// Starts generation of an audio tone.
    /// @param[in] aDevice The audio device ID to play the tone through.
    /// @param[in] aIface The interface ID of the audio device.
    /// @param[in] aChannel The audio channel ID.
    /// See device file accmd_prim.h for valid interface (ACCMD_AUDIO_INSTANCE) values.
    /// @throws CDutException.
    ///
    void AudioToneStart(AudioDevice aDevice, uint16 aIface, AudioChannel aChannel);

    ///
    /// Stops an audio tone.
    /// @throws CDutException.
    ///
    void AudioToneStop();

    ///
    /// Configures microphone bias.
    /// @param[in] aId The mic bias instance to configure.
    /// @param[in] aKey The configuration key, e.g.:
    ///   MIC_BIAS_ENABLE = 0
    ///   MIC_BIAS_VOLTAGE = 1
    /// @param[in] aValue The value to set for the key.
    /// @throws CDutException.
    ///
    void AudioConfigMicBias(uint16 aId, uint16 aKey, uint32 aValue);

    ///
    /// Connects an audio loop path.
    /// Note that for microphone sources, AudioConfigMicBias may need to be used to
    ///   enable or otherwise configure microphone bias.
    /// @param[in] aSourceDevice The source audio device ID.
    /// @param[in] aSourceIface The interface ID of the source audio device.
    /// @param[in] aSourceChannel The source audio channel ID.
    /// @param[in] aSinkDevice The sink audio device ID.
    /// @param[in] aSinkIface The interface ID of the sink audio device.
    /// @param[in] aSinkChannel The sink audio channel ID.
    /// See device file accmd_prim.h for valid interface (ACCMD_AUDIO_INSTANCE) values.
    /// @throws CDutException.
    ///
    void AudioLoopbackStart(AudioDevice aSourceDevice, uint16 aSourceIface, AudioChannel aSourceChannel,
        AudioDevice aSinkDevice, uint16 aSinkIface, AudioChannel aSinkChannel);

    ///
    /// Disconnects an audio loop path.
    /// @throws CDutException.
    ///
    void AudioLoopbackStop();

    ///
    /// Performs an I2C transfer operation (write and/or read operation).
    /// @param[in] aPioScl The PIO used for I2C SCL (clock).
    /// @param[in] aPioSda The PIO used for I2C SDA (data).
    /// @param[in] aClockKhz The I2C clock speed in kHz.
    /// @param[in] aDevAddr The address of the I2C device.
    /// @param[in] aTxOctets The octets to write to the device (can be empty in the case of a read only operation).
    /// @param[in] aRxOctets The number of octets to read from the device.
    /// @return The octets read.
    /// @throws CDutException.
    ///
    std::vector<uint8> I2CTransfer(uint16 aPioScl, uint16 aPioSda, uint16 aClockKhz,
        uint16 aDevAddr, std::vector<uint8> aTxOctets, uint16 aRxOctets);

    ///
    /// Performs an application (host comms) write operation.
    /// @param[in] aChannel The channel number (0 - 127).
    /// @param[in] aData The data to write.
    /// @throws CDutException.
    ///
    void AppWrite(uint8 aChannel, const std::vector<uint16>& aData);

private:

    ///
    /// Holds audio stream connection data to allow tear down when stopped.
    ///
    class AudioStreamData
    {
    public:
        AudioStreamData() : sourceId(0), sinkId(0), transformId(0), operatorId(0) {};

        uint16 sourceId;
        uint16 sinkId;
        uint16 transformId;
        uint16 operatorId;
    };

    ///
    /// Detect and return the transport string for a single connected device.
    /// Throws if there are no connected devices or more than one.
    /// @return Transport specifier string
    /// @throws CDutException.
    ///
    std::string AutoDetectTransport();

    ///
    /// Initialises the internal configuration data cache (reads from device).
    /// @throws CDutException.
    ///
    void ConfigInit();

    ///
    /// Commits (writes) configuration values set with ConfigSetValue to the device.
    /// @throws CDutException.
    /// @see ConfigSetValue.
    ///
    void ConfigCommitValues();

    ///
    /// Identifies the device.
    /// @return The device name.
    ///
    std::string Identify();

    ///
    /// Burns firmware to the device.
    ///
    void BurnFirmware();

    ///
    /// Path to the firmware image file.
    ///
    std::string mFirmwarePath;

    ///
    /// Time (in milliseconds) to wait after device reset before attempting to open a connection.
    ///
    size_t mResetWaitMs;

    ///
    /// Transport option string.
    ///
    std::string mTransport;

    ///
    /// TestEngine handle.
    ///
    uint32 mTeHandle;

    ///
    /// Indicates if the configuration cache has been initialised.
    ///
    bool mConfigInitialised;

    ///
    /// Indicates if the configuration cache has been updated (i.e. commit to device required).
    ///
    bool mConfigUpdated;

    ///
    /// Connected Audio streams.
    ///
    std::vector<AudioStreamData> mAudioStreams;

    ///
    /// Indicates if the QHCI radio APIs are supported.
    ///
    bool mUsesQHciRadioApis;

    ///
    /// Configuration database path
    ///
    std::string mCfgDbPath;

    ///
    /// Configuration layer to use when writing.
    ///
    uint16 mCfgWriteLayer;

    ///
    /// Whether the app should be disabled before operations are performed.
    ///
    bool mDisableApp;

    ///
    /// Whether the device should be reset when the connection is closed.
    ///
    bool mResetOnClose;
};

#endif // CDA_DEVICE_H
