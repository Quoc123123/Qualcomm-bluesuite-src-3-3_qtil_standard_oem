//**************************************************************************************************
//
//  CdaDtsDevice.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA DTS device class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CDA_DTS_DEVICE_H
#define CDA_DTS_DEVICE_H

#include "common/types.h"
#include "Dut.h"
#include <BluetoothAPIs.h>
#include <vector>

class CPtSerialPort;
class CAtMessenger;
class CPtBdAddrRecord;

///
/// Device class for CDA ICs using Device Test Service for control
///
class CCdaDtsDevice : public CDut
{
public:

    static const uint8 AUDIO_INSTANCE_MAX = 3; //!< The maximum audio instance value
    static const uint8 AUDIO_TONE_MAX = 119; //!< The maximum audio tone value
    static const uint8 LED_ID_MAX = 7; //!< The maximum LED ID value

    ///
    /// Supported audio device identifiers.
    /// Corresponds to device enum audio_hardware (audio_if.h).
    ///
    enum class AudioHardware
    {
        PCM,
        I2S,
        SPDIF,
        CODEC,
        DIGITAL_MIC,
        FM,
        MAX = FM
    };

    ///
    /// Supported audio channel identifiers.
    /// Corresponds to device enum audio_channel (audio_if.h).
    ///
    enum class AudioChannel
    {
        LEFT,
        RIGHT,
        LEFT_AND_RIGHT,
        MAX = LEFT_AND_RIGHT
    };

    ///
    /// Supported touch action types.
    /// Corresponds to the touch_type field in +TOUCH indications (apart from ANY)
    ///
    enum class TouchType
    {
        UNKNOWN,
        TOUCH,
        SLIDE,
        HAND_COVER,
        ANY
    };

    ///
    /// Constructor.
    /// @param[in] aSetup Production test setup object.
    ///
    explicit CCdaDtsDevice(const CPtSetup& aSetup);

    ///
    /// Destructor.
    ///
    virtual ~CCdaDtsDevice();

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
    /// @param[in] aMask Specifies the LEDs to enable (1) / disable (0).
    /// Note that a value of zero turns off all LEDs and re-enables any automatic
    /// LED pattern configured for the application.
    /// @throws CDutException.
    ///
    void LedSet(uint8 aMask);

    ///
    /// Starts generation of an audio tone.
    /// @param[in] aHardware The AudioHardware to play the tone through.
    /// @param[in] aInstance The instance ID of the audio hardware.
    /// @param[in] aChannel The AudioChannel.
    /// @param[in] aTone The tone (0-119, where 0 is C0 (low) and 119 is B9 (high)).
    /// @throws CDutException.
    ///
    void AudioToneStart(AudioHardware aHardware, uint8 aInstance,
        AudioChannel aChannel, uint8 aTone);

    ///
    /// Stops an audio tone.
    /// @throws CDutException.
    ///
    void AudioToneStop();

    ///
    /// Connects an audio loop path from a mic to an output.
    /// @param[in] aMic The microphone number, from 1 - n, where n is device dependent.
    ///   (See device file microphones.h, microphone_number_t enum).
    /// @param[in] aOutHardware The AudioHardware to play the tone through.
    /// @param[in] aOutInstance The instance ID of the audio hardware.
    /// @param[in] aOutChannel The AudioChannel.
    /// @throws CDutException.
    ///
    void AudioLoopbackStart(uint8 aMic, AudioHardware aOutHardware,
        uint8 aOutInstance, AudioChannel aOutChannel);

    ///
    /// Disconnects an audio loop path.
    /// @throws CDutException.
    ///
    void AudioLoopbackStop();

    ///
    /// Waits for a touch event within the given timeout.
    /// @param[in] aTimeoutS Timeout in seconds.
    /// @param[in] aType Touch type to wait for.
    /// @return true if a touch is detected, false otherwise.
    /// @throws CDutException.
    ///
    bool DetectTouch(uint16 aTimeoutS, TouchType aType);

    ///
    /// Runs a proximity test, waiting for a transition within the given timeout.
    /// @param[in] aTimeoutS Timeout in seconds.
    /// @return true if the test passes, false otherwise.
    /// @throws CDutException.
    ///
    bool TestProximity(uint16 aTimeoutS);

    ///
    /// Runs a hall effect sensor test, waiting for a transition within the given timeout.
    /// @param[in] aTimeoutS Timeout in seconds.
    /// @return true if the test passes, false otherwise.
    /// @throws CDutException.
    ///
    bool TestHallEffect(uint16 aTimeoutS);

    ///
    /// Gets the temperature from the device.
    /// @return Temperature in degrees C.
    /// @throws CDutException.
    ///
    int16 GetTemperatureDegC();

    ///
    /// Gets the battery voltage from the device.
    /// @return Battery voltage in milli-volts.
    /// @throws CDutException.
    ///
    uint16 GetBatteryMv();

    ///
    /// Gets the Received Signal Strength Indication (RSSI) from the device.
    /// @return RSSI in dBm.
    /// @throws CDutException.
    ///
    int16 GetRssiDbm();

    ///
    /// Sets the PIO to use to trigger radio test stop.
    /// @param[in] aPio PIO number.
    /// @throws CDutException.
    ///
    void RadioCfgStopPio(uint8 aPio);

    ///
    /// Sets the radio test stop time.
    /// @param[in] aTimeSeconds Time in seconds after which a radio test mode will stop.
    /// @throws CDutException.
    ///
    void RadioCfgStopTime(uint8 aTimeSeconds);

    ///
    /// Configures the device to stop radio tests on a touchpad event.
    /// @throws CDutException.
    ///
    void RadioCfgStopTouch();

    ///
    /// Sets the power level for TX test modes.
    /// @param[in] aPowerLevel Power level to set.
    /// @throws CDutException.
    ///
    void RadioCfgTxPower(uint8 aPowerLevel);

    ///
    /// Starts carrier wave transmission.
    /// NOTE: Causes DTS serial port disconnection due to stopping normal BT operation.
    /// @param[in] aChannel Bluetooth channel number to transmit on.
    /// @throws CDutException.
    ///
    void RadioTxCwStart(uint8 aChannel);

    ///
    /// Sets an application PS store key value.
    /// @param[in] aKeyId The PS key ID.
    ///   The key ID can either be from the PSKEY_* ranges specified in the
    ///   ps_if.h device file, or the "local" IDs (which start at 0) used within
    ///   device application code (see the Device Test Service User Guide).
    /// @param[in] aValue Value to write.
    /// @throws CDutException.
    ///
    void PsSetValue(uint16 aKeyId, std::vector<uint16> aValue);

    ///
    /// Clears an application PS store key.
    /// @param[in] aKeyId The PS key ID.
    ///   The key ID can either be from the PSKEY_* ranges specified in the
    ///   ps_if.h device file, or the "local" IDs (which start at 0) used within
    ///   device application code (see the Device Test Service User Guide).
    /// @throws CDutException.
    ///
    void PsClear(uint16 aKeyId);

private:

    ///
    /// The character(s) used to indicate end of message for AT command processing.
    ///
    static const char* const AT_NEWLINE;

    ///
    /// The setting used for the Bluetooth address record file path.
    ///
    static const char* const BD_ADDR_REC_FILE_SETTING;

    ///
    /// Standard device read timeout in milliseconds.
    ///
    static const uint32 STD_READ_TIMEOUT_MS = 3000;

    ///
    /// Copy constructor.
    ///
    CCdaDtsDevice(const CCdaDtsDevice&);

    ///
    /// Assignment operator.
    ///
    CCdaDtsDevice& operator=(const CCdaDtsDevice&);

    ///
    /// Authenticates with the device DTS service.
    /// @throws CDutException
    ///
    void Authenticate();

    ///
    /// Performs AES-CMAC calculation using OpenSSL.
    /// @param[in] apKey Encryption key.
    /// @param[in] aKeyLen Key length.
    /// @param[in] apMsgIn The message to encrypt.
    /// @param[in] aMsgLen Length of the message (aMsgIn and aMsgOut must be a least this big).
    /// @param[out] apMsgOut The result (encrypted message).
    /// @throws CDutException
    ///
    void CalcAesCmac(const uint8* apKey, size_t aKeyLen, const uint8* apMsgIn, size_t aMsgLen, uint8* apMsgOut);

    ///
    /// Connects to the device's COM port
    /// @param[in] aPortNum The port number to open
    /// @throws CDutException.
    ///
    void ConnectComPort(uint16 aPortNum);

    ///
    /// Disconnects from the device's COM port
    /// @throws CDutException.
    ///
    void DisconnectComPort();

    ///
    /// Checks device responses for the given prefix, removing it and the 
    /// expected ':' separator from each response.
    /// @param[in] aResponses Device reponses to strip the prefix from.
    /// @param[in] aPrefix The expected prefix.
    /// @throws CDutException.
    ///
    void StripRespPrefix(std::vector<std::string>& aResponses, const std::string& aPrefix);

    ///
    /// Authentication context data
    ///
    struct AuthContext
    {
    public:
        AuthContext();
        ~AuthContext();

        HANDLE mCompleteEvent; //!< Event used to signal authentication callback completion
        std::string mErrMsg; //!< Error string
    
    private:
        AuthContext(const AuthContext&);
        AuthContext& operator=(const AuthContext&);
    };

    ///
    /// Callback function registered with BT API function BluetoothRegisterForAuthenticationEx.
    /// @param[in] Optional context pointer (can be NULL).
    /// @param[in] Device specific authentication configuration parameters.
    /// @return TRUE on success, FALSE otherwise (though this is ignored by the BT API caller).
    ///
    static BOOL WINAPI BtAuthCallback(LPVOID apParam,
        PBLUETOOTH_AUTHENTICATION_CALLBACK_PARAMS apAuthCallbackParams);

    ///
    /// Attempts to find a Bluetooth device.
    /// @param[in] aPairedOnly If true, search only for paired devices, otherwise search all.
    /// @return true if the device was found, false otherwise.
    /// @throws CPtException.
    ///
    bool FindDevice(bool aPairedOnly);

    ///
    /// Attempts to pair with a Bluetooth device
    /// @param[in] Device information.
    /// @throws CPtException.
    ///
    void PairDevice(BLUETOOTH_DEVICE_INFO aDevInfo);

    ///
    /// Attempts to connect to the device
    /// @throws CPtException.
    ///
    void ConnectBt();

    ///
    /// Attempts to disconnect the device
    /// @throws CPtException.
    ///
    void DisconnectBt();

    ///
    /// Attempts to find the COM port for the SPP connection
    /// @return COM port number
    /// @throws CPtException.
    ///
    uint16 FindComPort();

    ///
    /// The serial port object.
    ///
    CPtSerialPort* mpSerialPort;

    ///
    /// The AT messenger object.
    ///
    CAtMessenger* mpAtMessenger;

    ///
    /// Bluetooth address record.
    ///
    CPtBdAddrRecord* mpBdAddrRecord;

    ///
    /// Device Bluetooth address.
    ///
    BLUETOOTH_ADDRESS_STRUCT mBdAddress;

    ///
    /// Device name.
    ///
    std::wstring mDeviceName;

    ///
    /// The number of octets in the DTS authentication key.
    ///
    static const size_t AUTH_KEY_NUM_OCTETS = 16;
    
    ///
    /// The DTS authentication key.
    ///
    uint8 mAuthKey[AUTH_KEY_NUM_OCTETS];

    ///
    /// The timeout (approximate) to use for BT inquiry.
    ///
    uint16 mBtInquiryTimeoutSeconds;

    ///
    /// Whether the pre-paired devices are allowed.
    ///
    bool mPrePairedDutsAllowed;

    ///
    /// Whether the device was pre-paired.
    ///
    bool mPrePairedDevice;

    ///
    /// Whether the device is paired.
    ///
    bool mPaired;

    ///
    /// Whether an inquiry and display of discovered devices should be performed if pairing fails.
    ///
    bool mInquiryOnPairingFail;

    ///
    /// Whether to disable DTS on pass or not.
    ///
    bool mDisableDtsOnPass;
};

#endif // CDA_DTS_DEVICE_H
