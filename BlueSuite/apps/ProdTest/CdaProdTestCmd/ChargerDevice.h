//**************************************************************************************************
//
//  ChargerDevice.h
//
//  Copyright (c) 2020-2023 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Charger device class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef CHARGER_DEVICE_H
#define CHARGER_DEVICE_H

#include "common/types.h"
#include "Dut.h"
#include <vector>

class CPtSerialPort;
class CAtMessenger;

///
/// Device class for charger case ICs
///
class CChargerDevice : public CDut
{
public:
    ///
    /// Supported LED modes.
    ///
    enum class LedMode
    {
        OFF = 0,
        RED = 1,
        GREEN = 2,
        AMBER = 3,
        BLUE = 4,
        MAGENTA = 5,
        CYAN = 6,
        WHITE = 7
    };

    ///
    /// Supported regulator levels.
    ///
    enum class RegLevel
    {
        HIGH = 0, // Approx. 5V
        LOW = 1,  // Approx. 4V
        RESET = 2 // < 500mV
    };

    ///
    /// Supported charger modes.
    ///
    enum class ChargerMode
    {
        MODE_100_MA = 0,
        MODE_500_MA = 1,
        MODE_I_LIM = 2,
        MODE_STANDBY = 3
    };

    ///
    /// Supported low power modes.
    ///
    enum class LowPowerMode
    {
        STANDBY = 1,
        STOP = 2,
    };

    ///
    /// Earbud selection.
    ///
    enum class EarbudSel
    {
        LEFT,
        RIGHT
    };

    ///
    /// GPIO functions.
    ///
    enum class GpioFunction
    {
        DIGITAL_INPUT,
        DIGITAL_INPUT_PD, // Input with pull-down
        DIGITAL_OUTPUT,
        ANALOGUE,
        ALTERNATIVE
    };

    ///
    /// GPIO states.
    ///
    enum class GpioState
    {
        UNKNOWN, // E.g. if the GPIO mode is analogue, or alternative-function
        LOW,
        HIGH
    };

    ///
    /// Constructor.
    /// @param[in] aSetup Production test setup object.
    ///
    explicit CChargerDevice(const CPtSetup& aSetup);

    ///
    /// Destructor.
    ///
    virtual ~CChargerDevice();

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
    /// Validates and sets the serial number.
    /// Note that this does not write the serial number to the device.
    /// @param[in] apSerialNum The serial number.
    ///
    void SetSerialNum(const CPtSerialNum* apSerialNum) override;

    ///
    /// Gets the serial number.
    /// Note that this does not read the serial number from the device,
    /// rather it is the serial number set using SetSerialNum.
    /// @return Serial number.
    /// @see SetSerialNum.
    ///
    uint64 GetSerialNumber() const { return mSerNumVal; };

    ///
    /// Reads a configuration value from the device.
    /// @param[in] aName The key name.
    /// @return The key value.
    /// @throws CDutException.
    ///
    std::string ConfigReadValue(const std::string& aName);

    ///
    /// Writes a configuration value to the device.
    /// @param[in] aName The key name.
    /// @param[in] aValue The key value.
    /// @throws CDutException.
    ///
    void ConfigWriteValue(const std::string& aName, const std::string& aValue);

    ///
    /// Sets a LED test mode.
    /// @param[in] aMode LED mode.
    /// @throws CDutException.
    ///
    void LedSet(LedMode aMode);

    ///
    /// Set the voltage regulator.
    /// @param[in] aEnable true to enable, false to disable.
    /// @param[in] aLevel The level to set when enabled.
    /// @throws CDutException.
    ///
    void VregSet(bool aEnable, RegLevel aLevel);

    ///
    /// Enable / disable the case comms pull-up used for fast case comms hardware.
    /// @param[in] aEnable true to enable, false to disable.
    /// @throws CDutException.
    ///
    void CaseCommsPullUpSet(bool aEnable);

    ///
    /// Gets the battery voltage.
    /// @return Battery voltage in milli-volts.
    /// @throws CDutException.
    ///
    uint16 BattVoltsGet();

    ///
    /// Gets the status of the earbuds.
    /// @return Comma-separated string of the earbud status: "<left>,<right>",
    /// where the status is either: "Failed" - if no response from earbud, or
    /// the earbud battery charge percentage (0 - 100).
    /// @throws CDutException.
    ///
    std::string EarbudStatus();

    ///
    /// Gets the Bluetooth addresses of an earbud.
    /// @param[in] aEarbud Earbud selection.
    /// @return The earbud BT address as a string of 12 hexadecimal digits (no prefix).
    /// @throws CDutException.
    ///
    std::string EarbudBtAddress(EarbudSel aEarbud);

    ///
    /// Gets the current sense ADC values.
    /// @return ADC values (Left and Right).
    /// @throws CDutException.
    ///
    std::vector<uint16> ReadCurrentSense();

    ///
    /// Gets whether the charger lid is detected as open or closed.
    /// @return true if the lid is detected as open, false if closed.
    /// @throws CDutException.
    ///
    bool LidOpen();

    ///
    /// Gets whether the charger is detected.
    /// @return true if the charger is detected, false otherwise.
    /// @throws CDutException.
    ///
    bool ChargerConnected();

    ///
    /// Set the battery charger.
    /// @param[in] aEnable true to enable, false to disable.
    /// @param[in] aMode The mode to set when enabled.
    /// @throws CDutException.
    ///
    void ChargerSet(bool aEnable, ChargerMode aMode);

    ///
    /// Set the low power mode.
    /// @param[in] aMode The mode to set.
    /// @throws CDutException.
    ///
    void LowPowerSet(LowPowerMode aMode);

    ///
    /// Gets the voltage of the thermistor potential divider.
    /// @return The voltage of the thermistor potential divider in milli-volts.
    /// @throws CDutException.
    ///
    uint16 ThermistorVoltsGet();

    ///
    /// Enter / exit test mode.
    /// @param[in] aEnable true to enter test mode, false to exit.
    /// @throws CDutException.
    ///
    void TestMode(bool aEnable);

    ///
    /// Sets shipping mode on the device (ready to ship).
    /// @throws CDutException.
    ///
    void SetShippingMode();

    ///
    /// Performs a loopback test of the case-bud comms TX/RX components.
    /// @param[in] aState The state to test (true = high, false = low).
    /// @return true if the test passes, false otherwise.
    /// @throws CDutException.
    ///
    bool TestBudCommsTxRxLoop(bool aState);

    ///
    /// Get the state and function of a GPIO.
    /// @param[in] aGpio The GPIO of interest, in the format <port><pin>, e.g. "A12",
    ///   where the port can be A,B or C, and the pin number is 0-15.
    /// @param[out] aFunction The GPIO function. Note that DIGITAL_INPUT will be returned
    ///   for GPIOs set to both DIGITAL_INPUT and DIGITAL_INPUT_PD.
    /// @return The GPIO state. Note that if the GPIO function is ANALOGUE or ALTERNATIVE,
    ///   the state will be returned as UNKNOWN.
    /// @throws CDutException.
    ///
    GpioState GetGpio(const std::string& aGpio, GpioFunction& aFunction);

    ///
    /// Gets whether the charger supports fast case-bud comms.
    /// @return true if fast comms is supported, false otherwise.
    ///
    bool SupportsFastComms() const { return mSupportsFastComms; };

    ///
    /// Validates and converts a GPIO name into a port letter and pin number.
    /// The name must be in the format <port><pin>, e.g. "A12".
    /// @param[in] aGpio The GPIO name.
    /// @param[out] aPort The port specifier (A, B or C).
    /// @return The pin index (0-15).
    /// @throws CDutException.
    /// 
    static uint8 ConvertGpioName(const std::string& aGpio, char& aPort);

private:

    ///
    /// The character(s) used to indicate end of message for AT command processing.
    ///
    static const char* const AT_NEWLINE;

    ///
    /// Time allowed for device boot up after reset.
    ///
    static const uint32 DEVICE_BOOT_TIME_MS = 3000;

    ///
    /// Standard device read timeout in milliseconds.
    ///
    static const uint32 STD_READ_TIMEOUT_MS = 3000;

    ///
    /// The number of pins on a GPIO port.
    ///
    static const size_t NUM_GPIO_PORT_PINS = 16;

    ///
    /// Burns firmware image to the DUT (spawns separate tool).
    /// @throws CDutException.
    ///
    void BurnFw();

    ///
    /// Format a string as needed to pass as a command-line argument,
    /// which means if it isn't double quoted, and it contains spaces,
    /// wrap it in double quotes.
    /// @param[in] aArg Command-line argument.
    /// @return Formated argument.
    ///
    std::string FormatCmdArg(const std::string& aArg);

    ///
    /// Connects to the device's COM port
    /// @throws CDutException.
    ///
    void ConnectComPort();

    ///
    /// Disconnects from the device's COM port
    /// @throws CDutException.
    ///
    void DisconnectComPort();

    ///
    /// Identifies the device.
    /// @return The device information.
    ///
    std::string Identify();

    ///
    /// Path of tool used to perform firmware image burn.
    ///
    std::string mFwBurnTool;

    ///
    /// Arguments to pass to the firmware image burning tool.
    ///
    std::vector<std::string> mFwBurnArgs;

    ///
    /// Exit codes indicating successful firmware image burn.
    ///
    std::vector<int> mFwBurnPassExitCodes;

    ///
    /// Whether a manual reset is required after firmware burn.
    ///
    bool mFwBurnManualReset;

    ///
    /// Whether the DUT ID is to be read and shown after connecting.
    ///
    bool mNoIdRead;

    ///
    /// COM port to use.
    ///
    std::string mComPort;

    ///
    /// Baud rate to use for the COM port.
    ///
    uint32 mBaudRate;

    ///
    /// The serial port object.
    ///
    CPtSerialPort* mpSerialPort;

    ///
    /// The AT messenger object.
    ///
    CAtMessenger* mpAtMessenger;

    ///
    /// The device serial number.
    ///
    uint64 mSerNumVal;

    ///
    /// Whether the device supports fast earbud comms.
    ///
    bool mSupportsFastComms;
};

#endif // CHARGER_DEVICE_H
