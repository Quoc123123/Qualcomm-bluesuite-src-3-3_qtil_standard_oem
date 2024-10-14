//**************************************************************************************************
//
//  ChargerDevice.cpp
//
//  Copyright (c) 2020-2023 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  ChargerDevice device class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ChargerDevice.h"
#include "PtSetup.h"
#include "PtUtil.h"
#include "PtSerialNum.h"
#include "PtTimer.h"
#include "AtMessenger.h"
#include <PtSerialPort.h>

#include <sstream>
#include <regex>
#include <process.h>
#include <chrono>
#include <thread>
#include <sys/stat.h>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

const char* const CChargerDevice::AT_NEWLINE = "\r\n";

////////////////////////////////////////////////////////////////////////////////

CChargerDevice::CChargerDevice(const CPtSetup& aSetup)
    : CDut(aSetup), mBaudRate(0), mpSerialPort(NULL), mpAtMessenger(NULL),
      mSerNumVal(0), mFwBurnManualReset(false), mNoIdRead(false),
      mSupportsFastComms(false)
{
    static const string FW_BURN_TOOL_SETTING = "FwBurnTool";
    static const string FW_BURN_ARGS_SETTING = "FwBurnArgs";
    static const string FW_BURN_EXIT_CODES_SETTING = "FwBurnPassExitCodes";
    static const string FW_BURN_MANUAL_RESET_SETTING = "FwBurnManualReset";
    static const string NO_ID_READ_SETTING = "NoIdRead";

    // Burning firmware is an optional step
    mFwBurnTool = mSetup.GetValue(FW_BURN_TOOL_SETTING, false);

    if (!mFwBurnTool.empty())
    {
        // Check exists and is executable
        struct stat fileStat;
        if (stat(mFwBurnTool.c_str(), &fileStat) != 0 || !(fileStat.st_mode & S_IEXEC))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << FW_BURN_TOOL_SETTING
                << "\" file path \"" << mFwBurnTool << "\" is not an executable file";
            throw CDutException(msg.str());
        }

        // Get the arguments 
        mFwBurnArgs = mSetup.GetValueList(FW_BURN_ARGS_SETTING, true);
            
        // Get the success exit codes
        vector<string> exitCodes = mSetup.GetValueList(FW_BURN_EXIT_CODES_SETTING, true);
        for (string exitCode : exitCodes)
        {
            int val;
            istringstream istr(exitCode);
            if (!(istr >> val) || !istr.eof())
            {
                ostringstream msg;
                msg << "Configuration setting \"" << FW_BURN_EXIT_CODES_SETTING
                    << "\" contains invalid value \"" << exitCode << "\"";
                throw CDutException(msg.str());
            }

            mFwBurnPassExitCodes.push_back(val);
        }

        // Get the manual reset flag
        mFwBurnManualReset = mSetup.GetValueNum<bool>(FW_BURN_MANUAL_RESET_SETTING);
    }

    // COM port settings
    mComPort = mSetup.GetValue("Transport", true);
    mBaudRate = mSetup.GetValueNum<uint32>("UartBaudRate");

    mNoIdRead = mSetup.GetValueNum<bool>(NO_ID_READ_SETTING);
}

////////////////////////////////////////////////////////////////////////////////

CChargerDevice::~CChargerDevice()
{
    Disconnect();
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::Connect()
{
    // Burn the firmware if specified
    if (!mFwBurnTool.empty())
    {
        BurnFw();
    }

    // Should be able to connect via COM port now
    ConnectComPort();
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::Disconnect()
{
    DisconnectComPort();
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::Reset(ResetMode aMode)
{
    mUi.Write("Resetting the DUT", false);
    mpAtMessenger->Write("AT+REBOOT");
    // No reponse as we're rebooting

    DisconnectComPort();

    if (aMode == ResetMode::WAIT)
    {
        // Allow some time for USB COM port to re-enumerate
        this_thread::sleep_for(milliseconds(DEVICE_BOOT_TIME_MS));

        ConnectComPort();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::DisableApp()
{
    TestMode(true);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::PreTestActions()
{
    // Purge the buffers to clear any data sent by the device (which we don't care about).
    mpSerialPort->Purge();

    // Stop normal activity
    DisableApp();

    // Identify the chip

    if (!mNoIdRead)
    {
        ostringstream msg;
        msg << "Connected device details:" << endl << Identify();
        mUi.Write(msg.str(), false);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::PostTestActions(bool /*aTestsPassed*/)
{
    Reset(ResetMode::NOWAIT);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::SetSerialNum(const CPtSerialNum* apSerialNum)
{
    if (apSerialNum == NULL)
    {
        // Should never happen
        throw CDutException("Serial number cannot be NULL");
    }
    
    // Check the type is compatible - as the DUT storage is a uint64, it has to
    // be decimal or hexadecimal, within that range.
    const CPtSerialNum::SnType type = apSerialNum->GetType();
    if (type != CPtSerialNum::SnType::DEC && type != CPtSerialNum::SnType::HEX)
    {
        throw CDutException("Serial number type is incompatible with DUT. Must be decimal or hexadecimal (no prefix), value no larger than 64 bits");
    }

    // Check the value, could be hex or dec, but must fit in a uint64.
    // Save the value so we don't have to do the conversion from string again.
    const string snStr = apSerialNum->ToString();
    istringstream istr(snStr);
    if (type == CPtSerialNum::SnType::HEX)
    {
        istr >> hex;
    }

    if (!(istr >> mSerNumVal) || !istr.eof())
    {
        ostringstream msg;
        msg << "Invalid serial number \"" << snStr
            << "\". Must be set as a string of decimal or hexadecimal characters (no prefix), value no larger than 64 bits";
        throw CDutException(msg.str());
    }

    // Save the serial number
    mpSerialNumber = apSerialNum;
}

////////////////////////////////////////////////////////////////////////////////

std::string CChargerDevice::ConfigReadValue(const std::string& aName)
{
    ostringstream msg;
    msg << "AT+CONFIG=" << aName;
    vector<string> resp = mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("Config value not received from DUT");
    }

    return resp.at(0);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::ConfigWriteValue(const std::string& aName,
    const std::string& aValue)
{
    ostringstream msg;
    msg << "AT+CONFIG=" << aName << "," << aValue;

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::LedSet(LedMode aMode)
{
    ostringstream msg;
    msg << "AT+LED=" << static_cast<uint16>(aMode);

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::VregSet(bool aEnable, RegLevel aLevel)
{
    ostringstream msg;
    msg << "AT+REGULATOR=" << (aEnable ? "1" : "0") << "," << static_cast<uint16>(aLevel);

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::CaseCommsPullUpSet(bool aEnable)
{
    ostringstream msg;
    msg << "AT+PULL=" << (aEnable ? "1" : "0");

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

uint16 CChargerDevice::BattVoltsGet()
{
    static const string CMD = "AT+BATTERY?";

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("Battery status values not received from DUT");
    }

    // Response contains a milli-volts value and percentage, comma-separated,
    // but we only care about the milli-volts value
    vector<string> values = PtUtil::SplitString(resp.at(0), ",");
    if (values.empty())
    {
        ostringstream msg;
        msg << "No value data returned for " << CMD;
        throw CDutException(msg.str());
    }

    uint16 mv;
    istringstream istr(values.at(0));
    if (!(istr >> mv) || !istr.eof())
    {
        ostringstream msg;
        msg << "Could not convert " << CMD << " response \"" << values.at(0) << "\" to value";
        throw CDutException(msg.str());
    }

    return mv;
}

////////////////////////////////////////////////////////////////////////////////

std::string CChargerDevice::EarbudStatus()
{
    static const string CMD = "AT+EBSTATUS";
    static const milliseconds READ_TIMEOUT_MS(10000);

    // Indications expected (after "OK"):
    //   EBSTATUS(L) : <percentage>|"Failed"
    //   EBSTATUS(R) : <percentage>|"Failed"
    static const regex REGEX_EBSTATUS("EBSTATUS\\s*\\(([LR])\\)\\s*\\:\\s*(\\S*)" + string(AT_NEWLINE));

    // The response may contain the earbud status responses
    vector<string> lines = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);

    // Convert to single string, makes subsequent handling easier as more data
    // comes in, which may not be in complete lines.
    string readStr;
    for (const string& line : lines)
    {
        readStr += line + AT_NEWLINE;
    }

    // The response may not have contained the earbud status responses, in which case
    // wait for them.
    string lStatus;
    string rStatus;
    time_point<steady_clock> startTime = steady_clock::now();
    do
    {
        cmatch match;
        while (regex_search(readStr.c_str(), match, REGEX_EBSTATUS))
        {
            if (match.str(1) == "L")
            {
                lStatus = match.str(2);
            }
            else if (match.str(1) == "R")
            {
                rStatus = match.str(2);
            }
            readStr.erase(match.position(0), match.length(0));
        }

        if (lStatus.empty() || rStatus.empty())
        {
            readStr += mpAtMessenger->Read(128, false);
        }
    } while ((lStatus.empty() || rStatus.empty()) &&
        duration_cast<milliseconds>(steady_clock::now() - startTime) < READ_TIMEOUT_MS);

    if (lStatus.empty() || rStatus.empty())
    {
        ostringstream msg;
        msg << "Failed to receive status for ";
        if (lStatus.empty() && rStatus.empty())
        {
            msg << "L & R";
        }
        else if (lStatus.empty())
        {
            msg << "L";
        }
        else
        {
            msg << "R";
        }
        msg << " earbud(s) within timeout (" << READ_TIMEOUT_MS.count() << " ms)";
        throw CDutException(msg.str());
    }

    ostringstream statusStr;
    statusStr << lStatus << "," << rStatus;

    return statusStr.str();
}

////////////////////////////////////////////////////////////////////////////////

std::string CChargerDevice::EarbudBtAddress(EarbudSel aEarbud)
{
    static const milliseconds READ_TIMEOUT_MS(10000);

    const string earbud = (aEarbud == EarbudSel::LEFT ? "L" : "R");

    // The second parameter indicates the information we require, only 0 is
    // currently supported (and this is for the BT address).
    ostringstream cmd;
    cmd << "AT+EBSTATUS=" << earbud << ",0";

    // Indication expected (after "OK", with example BT address):
    //   EBSTATUS (L|R): 0002,5B,00FF0A
    static const regex REGEX_EBSTATUS("EBSTATUS\\s*\\(([LR])\\)\\s*\\:\\s*(\\S*)" + string(AT_NEWLINE));

    // The response may contain the earbud status responses
    vector<string> lines = mpAtMessenger->Transact(cmd.str(), STD_READ_TIMEOUT_MS);

    // Convert to single string, makes subsequent handling easier as more data
    // comes in, which may not be in complete lines.
    string readStr;
    for (const string& line : lines)
    {
        readStr += line + AT_NEWLINE;
    }

    // The response may not have contained the earbud status responses, in which case
    // wait for them.
    string bdAddr;
    time_point<steady_clock> startTime = steady_clock::now();
    do
    {
        cmatch match;
        while (regex_search(readStr.c_str(), match, REGEX_EBSTATUS))
        {
            if (match.str(1) == earbud)
            {
                bdAddr = match.str(2);
            }
            readStr.erase(match.position(0), match.length(0));
        }

        if (bdAddr.empty())
        {
            readStr += mpAtMessenger->Read(128, false);
        }
    } while (bdAddr.empty() &&
        duration_cast<milliseconds>(steady_clock::now() - startTime) < READ_TIMEOUT_MS);

    if (bdAddr.empty())
    {
        ostringstream msg;
        msg << "Failed to receive device address for " << earbud
            << " earbud within timeout (" << READ_TIMEOUT_MS.count() << " ms)";
        throw CDutException(msg.str());
    }
    else
    {
        PtUtil::ToUpper(bdAddr);
        if (bdAddr == "FAILED")
        {
            ostringstream msg;
            msg << "Failed to receive device address for " << earbud << " earbud";
            throw CDutException(msg.str());
        }
    }

    vector<string> bdAddrParts = PtUtil::SplitString(bdAddr, ",");
    if (bdAddrParts.size() != 3)
    {
        ostringstream msg;
        msg << "Bluetooth device address returned for " << earbud
            << " earbud (" << bdAddr << ") is not in the expected 3 part format (NAP, UAP, LAP)";
        throw CDutException(msg.str());
    }

    string addrStr;
    for (const string& part : bdAddrParts)
    {
        addrStr += part;
    }

    if (addrStr.size() != 12 || !all_of(addrStr.begin(), addrStr.end(), isxdigit))
    {
        ostringstream msg;
        msg << "Bluetooth device address returned for " << earbud
            << " earbud (" << bdAddr << ") is not 12 hexadecimal digits";
        throw CDutException(msg.str());
    }

    return addrStr;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<uint16> CChargerDevice::ReadCurrentSense()
{
    static const string CMD = "AT+SENSE?";

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("Current sense values not received from DUT");
    }

    // Response contains ADC readings for each of L and R earbud current sense,
    // comma-separated, left first.
    vector<string> values = PtUtil::SplitString(resp.at(0), ",");
    if (values.size() != 2)
    {
        ostringstream msg;
        msg << "Didn't get the expected 2 values for " << CMD;
        throw CDutException(msg.str());
    }

    vector<uint16> adcValues;
    uint16 adcVal;
    for (const string& valStr : values)
    {
        istringstream istr(valStr);
        if (!(istr >> adcVal) || !istr.eof())
        {
            ostringstream msg;
            msg << "Could not convert " << CMD << " response value \"" << valStr << "\" to value";
            throw CDutException(msg.str());
        }

        adcValues.push_back(adcVal);
    }

    return adcValues;
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerDevice::LidOpen()
{
    static const string CMD = "AT+LID?";

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);

    // 0 = Closed, 1 = Open
    if (resp.empty() || resp.at(0).size() != 1 || 
        (resp.at(0).at(0) != '0' && resp.at(0).at(0) != '1'))
    {
        ostringstream msg;
        msg << "Response for " << CMD << " should be either '0' or '1'";
        throw CDutException(msg.str());
    }

    return (resp.at(0).at(0) == '1' ? true : false);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerDevice::ChargerConnected()
{
    static const string CMD = "AT+CHARGER?";

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);

    // Returns <connected>,<charging>,<mode>. We're only interested in connected.
    // 1 = Connected, 0 = Not connected
    if (resp.empty() || resp.at(0).empty() || 
        (resp.at(0).at(0) != '0' && resp.at(0).at(0) != '1'))
    {
        ostringstream msg;
        msg << "Response for " << CMD << " should be either '0' or '1'";
        throw CDutException(msg.str());
    }

    return (resp.at(0).at(0) == '1' ? true : false);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::ChargerSet(bool aEnable, ChargerMode aMode)
{
    ostringstream msg;
    msg << "AT+CHARGER=" << (aEnable ? "1" : "0") << "," << static_cast<uint16>(aMode);

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::LowPowerSet(LowPowerMode aMode)
{
    ostringstream msg;
    msg << "AT+POWER=" << static_cast<uint16>(aMode);

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

uint16 CChargerDevice::ThermistorVoltsGet()
{
    static const string CMD = "AT+NTC?";

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("NTC value not received from DUT");
    }

    // Response contains a milli-volts value
    uint16 mv;
    istringstream istr(resp.at(0));
    if (!(istr >> mv) || !istr.eof())
    {
        ostringstream msg;
        msg << "Could not convert " << CMD << " response \"" << resp.at(0) << "\" to value";
        throw CDutException(msg.str());
    }

    return mv;
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::BurnFw()
{
    // Timed block
    {
        CPtTimer timer("Firmware Burn");

        // First command-line argument is the tool path
        ostringstream args;
        args << FormatCmdArg(mFwBurnTool);

        // Adding all arguments to one string, as it's easier
        ostringstream msg;
        msg << "Attempting to burn charger firmware image using " << mFwBurnTool;
        for (const string& arg : mFwBurnArgs)
        {
            msg << " " << arg;
            args << " " << FormatCmdArg(arg);
        }
        mUi.Write(msg.str(), false);

        const string argsStr(args.str());
        const char* pArgv[2];
        pArgv[0] = argsStr.c_str();
        pArgv[1] = NULL;

        // Perform the burn
        int ret = static_cast<int>(_spawnv(_P_WAIT, mFwBurnTool.c_str(), pArgv));

        // Check the exit code
        if (find(mFwBurnPassExitCodes.begin(), mFwBurnPassExitCodes.end(), ret) == mFwBurnPassExitCodes.end())
        {
            ostringstream expected;
            for (size_t retIndex = 0; retIndex < mFwBurnPassExitCodes.size(); ++retIndex)
            {
                expected << mFwBurnPassExitCodes.at(retIndex);
                if (retIndex < mFwBurnPassExitCodes.size() - 1)
                {
                    expected << ",";
                }
            }

            ostringstream errMsg;
            errMsg << "Charger firmware burn return value (" << ret
                << ") indicates failure (expecting " << expected.str() << ")";
            throw CDutException(errMsg.str());
        }
    }

    // Manual reset step might be needed if after burning the device isn't running the burned firmware
    if (mFwBurnManualReset)
    {
        mUi.AskHitKey("Please reset the DUT then hit a key");
    }

    mUi.Write("Waiting for DUT boot...");
    this_thread::sleep_for(milliseconds(DEVICE_BOOT_TIME_MS));
}

/////////////////////////////////////////////////////////////////////////////

std::string CChargerDevice::FormatCmdArg(const std::string& aArg)
{
    string quoted = aArg;

    // Already quoted?
    if (aArg.at(0) != '"')
    {
        // Contains spaces, so needs to be quoted?
        if (aArg.find(' ') != string::npos)
        {
            quoted = "\"" + aArg + "\"";
        }
    }

    return quoted;
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::ConnectComPort()
{
    delete mpSerialPort;
    mpSerialPort = new CPtSerialPort(mComPort, mBaudRate);

    delete mpAtMessenger;
    mpAtMessenger = new CAtMessenger(*mpSerialPort, AT_NEWLINE, true);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::DisconnectComPort()
{
    delete mpAtMessenger;
    mpAtMessenger = NULL;

    delete mpSerialPort;
    mpSerialPort = NULL;
}

////////////////////////////////////////////////////////////////////////////////

std::string CChargerDevice::Identify()
{
    static const string CMD = "AT+ID?";
    static const size_t NUM_ID_FIELDS = 4;

    string idStr;

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("ID values not received from DUT");
    }

    // Response contains: "<firmware_variant_string>",<board_number>,<firmware_id>,"<firmware_id_string>"
    // Where the board_number and firmware_id fields are 32 bit unsigned integers,
    // and the other fields are strings (double quoted).
    vector<string> values = PtUtil::SplitString(resp.at(0), ",");
    if (values.size() != NUM_ID_FIELDS)
    {
        ostringstream msg;
        msg << "Unexpected response received for " << CMD << ", expected "
            << NUM_ID_FIELDS << " comma-separated fields";
        throw CDutException(msg.str());
    }
    else
    {
        uint32 boardNum;
        istringstream istrBNum(values.at(1));
        if (!(istrBNum >> boardNum) || !istrBNum.eof())
        {
            ostringstream msg;
            msg << "Could not convert " << CMD << " response field \"" << values.at(1) << "\" to value";
            throw CDutException(msg.str());
        }

        uint32 firmwareId;
        istringstream istrFId(values.at(2));
        if (!(istrFId >> firmwareId) || !istrFId.eof())
        {
            ostringstream msg;
            msg << "Could not convert " << CMD << " response field \"" << values.at(2) << "\" to value";
            throw CDutException(msg.str());
        }

        ostringstream msg;
        msg << "Firmware variant: " << values.at(0) << endl
            << "Board number: " << boardNum << endl
            << "Firmware ID number: " << firmwareId << endl
            << "Firmware ID: " << values.at(3);
        idStr = msg.str();

        if (values.at(0) == "\"ST2\"")
        {
            mSupportsFastComms = true;
        }
    }

    return idStr;
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::TestMode(bool aEnable)
{
    // Enter test mode (stops normal activity)
    ostringstream cmdStr;
    cmdStr << "AT+TEST=" << (aEnable ? '1' : '0');
    // Nothing in the response we care about other than OK
    // Allowing enough time for read of additional data due to earbud message
    // output when not in test mode.
    (void)mpAtMessenger->Transact(cmdStr.str(), 5000);
}

////////////////////////////////////////////////////////////////////////////////

bool CChargerDevice::TestBudCommsTxRxLoop(bool aState)
{
    ostringstream cmdStr;
    cmdStr << "AT+TXTEST=" << (aState ? '1' : '0');

    vector<string> resp = mpAtMessenger->Transact(cmdStr.str(), STD_READ_TIMEOUT_MS);

    // PASS or FAIL expected
    if (resp.empty() || (resp.at(0) != "PASS" && resp.at(0) != "FAIL"))
    {
        ostringstream msg;
        msg << "Response for " << cmdStr.str()
            << " should be either \"PASS\" or \"FAIL\"";
        throw CDutException(msg.str());
    }

    return (resp.at(0) == "PASS" ? true : false);
}

////////////////////////////////////////////////////////////////////////////////

void CChargerDevice::SetShippingMode()
{
    static const string CMD = "AT+SHIP";
    // Allow longer timeout for this one as the earbuds need time to get into
    // shipping mode and respond to the charger.
    static const uint32 READ_TIMEOUT_MS(10000);

    // Indications expected (before "OK"):
    //   Shipping mode (L)"
    //   Shipping mode (R)"
    static const regex REGEX_EBSTATUS("Shipping mode\\s*\\(([LR])\\)\\s*");

    vector<string> read = mpAtMessenger->Transact(CMD, READ_TIMEOUT_MS);
    bool lOk = false;
    bool rOk = false;
    if (!read.empty())
    {
        // Should have status for both earbuds (two lines)
        for (const string& line : read)
        {
            cmatch match;
            if (!regex_match(line.c_str(), match, REGEX_EBSTATUS))
            {
                throw CDutException("Unexepected data received from DUT, expecting \"Shipping mode (L|R)\"");
            }

            if (match.str(1) == "L")
            {
                lOk = true;
            }
            else if (match.str(1) == "R")
            {
                rOk = true;
            }
        }
    }

    if (!lOk || !rOk)
    {
        ostringstream msg;
        msg << "Failed to receive shipping mode confirmation for ";
        if (!lOk && !rOk)
        {
            msg << "L & R";
        }
        else if (!lOk)
        {
            msg << "L";
        }
        else
        {
            msg << "R";
        }
        msg << " earbud(s)";
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

uint8 CChargerDevice::ConvertGpioName(const std::string& aGpio, char& aPort)
{
    uint8 pinIndex = 0;
    bool valid = true;
    
    if (aGpio.size() >= 2)
    {
        aPort = aGpio.at(0);
        uint16 pin; // Using uint16 to avoid read as char
        istringstream istr(aGpio.substr(1));
        if (!(istr >> pin) || !istr.eof() || pin > 0xFF || pin >= NUM_GPIO_PORT_PINS ||
            (aPort != 'A' && aPort != 'a' &&
             aPort != 'B' && aPort != 'b' &&
             aPort != 'C' && aPort != 'c'))
        {
            valid = false;
        }
        else
        {
            pinIndex = static_cast<uint8>(pin);
        }

        // Convert to upper-case to simplify things for caller
        aPort = static_cast<char>(toupper(aPort));
    }
        
    if (!valid)
    {
        ostringstream msg;
        msg << "GPIO specifier is invalid, must start with either A,B or C, followed by a number between 0 and " << NUM_GPIO_PORT_PINS - 1;
        throw CDutException(msg.str());
    }

    return pinIndex;
}

////////////////////////////////////////////////////////////////////////////////

CChargerDevice::GpioState CChargerDevice::GetGpio(const std::string& aGpio, GpioFunction& aFunction)
{
    static const string CMD = "AT+GPIO"; // NOTE: No '?' unlike other querying commands

    // Response is effectively a table looking like:
    // 
    //        0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    // GPIOA i0 i1 o1 i0 an o1 an i0 o0 i0 o1 i1 i1 af af i1
    // GPIOB o0 o1 i1 o1 o1 o1 af af o0 i1 af af i0 i0 o1 o0
    // GPIOC i0 i0 i0 i0 i0 i0 i0 i0 i0 i0 i0 i0 i0 i1 i1 i1
    //
    // Where:
    //   i0 = Input and low
    //   i1 = Input and high
    //   o0 = Output and low
    //   o1 = Ouput and high
    //   an = Analog
    //   af= Alternative function
    //
    // We need to extract the function (and the state if function is not "an" or "af").
    // 
    // Note that while input-with-pulldown can be set, there's no provision to
    // show that vs. a standard input in the information returned.
    //

    char port;
    const uint16 pin = ConvertGpioName(aGpio, port);
    // Get the zero-based index for the line containing the port statuses.
    const size_t rspPortLine = port - 'A' + 1;

    vector<string> resp = mpAtMessenger->Transact(CMD, STD_READ_TIMEOUT_MS);
    if (resp.empty())
    {
        throw CDutException("GPIO status not received from DUT");
    }

    static const size_t EXP_RSP_LINES = 4;
    if (resp.size() != EXP_RSP_LINES)
    {
        ostringstream msg;
        msg << "Unexpected response data returned for " << CMD << ", expected " << EXP_RSP_LINES << " lines";
        throw CDutException(msg.str());
    }

    vector<string> statuses = PtUtil::SplitString(resp.at(rspPortLine), " ");
    static const size_t EXP_LINE_FIELDS = 1 + NUM_GPIO_PORT_PINS; // Port name (e.g. "GPIOA") plus number of pins
    if (statuses.size() != EXP_LINE_FIELDS)
    {
        ostringstream msg;
        msg << "Unexpected response data returned for " << CMD << ", expected " << EXP_LINE_FIELDS 
            << " fields in line " << rspPortLine;
        throw CDutException(msg.str());
    }

    const string status = statuses.at(pin + 1); // Skip past the port name

    static const size_t STATUS_FIELD_SIZE = 2;
    if (status.size() != STATUS_FIELD_SIZE)
    {
        ostringstream msg;
        msg << "Unexpected response data returned for " << CMD << ", expected status field size of " 
            << STATUS_FIELD_SIZE << ", got \"" << status << "\"";
        throw CDutException(msg.str());
    }

    if (status == "an")
    {
        aFunction = GpioFunction::ANALOGUE;
    }
    else if (status == "af")
    {
        aFunction = GpioFunction::ALTERNATIVE;
    }
    else if (status.at(0) == 'i')
    {
        aFunction = GpioFunction::DIGITAL_INPUT;
    }
    else if (status.at(0) == 'o')
    {
        aFunction = GpioFunction::DIGITAL_OUTPUT;
    }

    GpioState state = GpioState::UNKNOWN;
    if (aFunction == GpioFunction::DIGITAL_INPUT || aFunction == GpioFunction::DIGITAL_OUTPUT)
    {
        if (status.at(1) == '0')
        {
            state = GpioState::LOW;
        }
        else if (status.at(1) == '1')
        {
            state = GpioState::HIGH;
        }
        else
        {
            ostringstream msg;
            msg << "Unexpected response data returned for " << CMD
                << ", expected state value of 0 or 1, got " << status.at(1);
            throw CDutException(msg.str());
        }
    }

    return state;
}
