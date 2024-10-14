//**************************************************************************************************
//
//  CdaDtsDevice.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA DTS device class definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "CdaDtsDevice.h"
#include "PtSetup.h"
#include "..\PtUtil.h"
#include "PtBdAddrMgr.h"
#include "PtSerialNum.h"
#include "AtMessenger.h"
#include <PtSerialPort.h>

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <chrono>
#include <thread>
#include <openssl/cmac.h>
#include <openssl/applink.c>
#include <SetupAPI.h>
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDtsDevice::AT_NEWLINE = "\r\n";
const char* const CCdaDtsDevice::BD_ADDR_REC_FILE_SETTING = "BdAddrRecordFile";

////////////////////////////////////////////////////////////////////////////////

CCdaDtsDevice::CCdaDtsDevice(const CPtSetup& aSetup)
    : CDut(aSetup), mpSerialPort(NULL), mpAtMessenger(NULL),
      mpBdAddrRecord(NULL), mAuthKey{0}, mBtInquiryTimeoutSeconds(0),
      mPrePairedDutsAllowed(false), mPrePairedDevice(false), mPaired(false),
      mInquiryOnPairingFail(false), mDisableDtsOnPass(true)
{
    static const uint16 MIN_INQ_TIMEOUT_SECONDS = 1;
    static const uint16 MAX_INQ_TIMEOUT_SECONDS = 60;
    static const string INQUIRY_TIMEOUT_KEY_SETTING = "BtInquiryTimeoutSeconds";
    static const string AUTH_KEY_SETTING = "DtsAuthKey";
    static const string ALLOW_PREPAIRED_DUTS_SETTING = "AllowPrePairedDuts";
    static const string INQUIRY_ON_PAIRING_FAIL_SETTING = "InquiryOnPairingFail";
    static const string DISABLE_DTS_ON_PASS_SETTING = "DisableDtsOnPass";

    // Get and validate the authentication key
    vector<string> authKeyOctets = mSetup.GetValueList(AUTH_KEY_SETTING, true);
    if (authKeyOctets.size() != AUTH_KEY_NUM_OCTETS)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << AUTH_KEY_SETTING
            << "\" should be a list of " << AUTH_KEY_NUM_OCTETS << " hex octets";
        throw CDutException(msg.str());
    }

    for (size_t index = 0; index < authKeyOctets.size(); ++index)
    {
        string octetStr(authKeyOctets.at(index));
        PtUtil::TrimString(octetStr);
        // Remove prefix if present
        if (octetStr.size() > 2 && octetStr.substr(0, 2) == "0x")
        {
            octetStr.erase(0, 2);
        }

        istringstream iStr(octetStr);
        uint16 val; // Using uint16 to avoid interpretation as char
        if (!(iStr >> hex >> val) || !iStr.eof() || val > 0xFF)
        {
            ostringstream msg;
            msg << "Configuration setting \"" << AUTH_KEY_SETTING << "\", value \""
                << octetStr << "\" is invalid (must be a hex octet)";
            throw CDutException(msg.str());
        }

        mAuthKey[index] = static_cast<uint8>(val);
    }

    // Get the BDADDR record file path
    // SN->BDADDR mapping file is not mandatory, as user could supply the address
    string recFile = mSetup.GetValue(BD_ADDR_REC_FILE_SETTING, false);
    if (!recFile.empty())
    {
        mpBdAddrRecord = new CPtBdAddrRecord(recFile, true);
    }

    // Get and validate the inquiry timeout
    mBtInquiryTimeoutSeconds = mSetup.GetValueNum<uint16>(INQUIRY_TIMEOUT_KEY_SETTING);
    if (mBtInquiryTimeoutSeconds < MIN_INQ_TIMEOUT_SECONDS || mBtInquiryTimeoutSeconds > MAX_INQ_TIMEOUT_SECONDS)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << INQUIRY_TIMEOUT_KEY_SETTING << "\", value \""
            << mBtInquiryTimeoutSeconds << "\" is out of range (must be between "
            << MIN_INQ_TIMEOUT_SECONDS << " and " << MAX_INQ_TIMEOUT_SECONDS << ")";
        throw CDutException(msg.str());
    }

    // Get whether pre-paired DUTs are allowed.
    mPrePairedDutsAllowed = mSetup.GetValueNum<bool>(ALLOW_PREPAIRED_DUTS_SETTING);

    // Get whether an inquiry and report of discovered devices should be performed if pairing fails.
    mInquiryOnPairingFail = mSetup.GetValueNum<bool>(INQUIRY_ON_PAIRING_FAIL_SETTING);

    // Get whether to disable DTS on pass.
    mDisableDtsOnPass = mSetup.GetValueNum<bool>(DISABLE_DTS_ON_PASS_SETTING);
}

////////////////////////////////////////////////////////////////////////////////

CCdaDtsDevice::~CCdaDtsDevice()
{
    Disconnect();

    delete mpBdAddrRecord;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::Connect()
{
    ConnectBt();

    ConnectComPort(FindComPort());

    Authenticate();

    wostringstream msg;
    msg << L"Connected to " << mDeviceName << L" device";
    mUi.Write(msg.str(), false);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::Disconnect()
{
    DisconnectComPort();

    // If not paired during connection, don't unpair.
    if (!mPrePairedDevice)
    {
        DisconnectBt();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::Reset(ResetMode /*aMode*/)
{
    throw CDutException("Reset not implemented");
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::PreTestActions()
{
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::PostTestActions(bool aTestsPassed)
{
    if (aTestsPassed && mDisableDtsOnPass)
    {
        // Nothing in the response we care about other than OK
        (void)mpAtMessenger->Transact("AT+DTSSETTESTMODE=0", STD_READ_TIMEOUT_MS);
    }

    // Reboot the device
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact("AT+DTSENDTESTING=1", STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::SetAddress(const std::string& aBtAddress)
{
    if (aBtAddress.empty())
    {
        // Need to get the address from a mapping file (SN->BDADDR)
        if (mpSerialNumber == NULL)
        {
            // Shouldn't ever happen
            throw CDutException("Serial number must be set in order to lookup BT address");
        }

        if (mpBdAddrRecord == NULL)
        {
            std::ostringstream msg;
            msg << "Bluetooth address record file is undefined with no address provided. Check the \""
                << BD_ADDR_REC_FILE_SETTING << "\" setting, or provide the address via the command-line";
            throw CDutException(msg.str());
        }

        mDeviceAddress = mpBdAddrRecord->GetAddress(mpSerialNumber->ToString());
        if (mDeviceAddress.empty())
        {
            std::ostringstream msg;
            msg << "Could not find BT address for DUT SN \"" << mpSerialNumber->ToString()
                << "\" in record file \"" << mpBdAddrRecord->GetRecordFile() << "\"";
            throw CDutException(msg.str());
        }
        else
        {
            std::ostringstream msg;
            msg << "Found BT address \"" << mDeviceAddress << "\"for DUT SN \""
                << mpSerialNumber->ToString() << "\" in record file";
            mUi.Write(msg.str(), false);
        }
    }
    else
    {
        mDeviceAddress = CPtBdAddrMgr::CheckBdAddr(aBtAddress);
    }

    // Also set local structure for BT API
    istringstream istr(mDeviceAddress);
    istr >> hex >> mBdAddress.ullLong;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::LedSet(uint8 aMask)
{
    ostringstream msg;
    msg << "AT+TESTLED=" << static_cast<uint16>(aMask);

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::AudioToneStart(AudioHardware aHardware, uint8 aInstance,
                                   AudioChannel aChannel, uint8 aTone)
{
    // Will play for max duration, to be stopped with AudioToneStop.
    static const uint16 MAX_DURATION_MS = 0xFFFF;

    // Should be validated by caller, but checking to be sure before using it in
    // the speaker mask.
    if (aInstance > AUDIO_INSTANCE_MAX)
    {
        ostringstream msg;
        msg << "Audio instance parameter is invalid (must be between 0 and "
            << AUDIO_INSTANCE_MAX << ")";
        throw CDutException(msg.str());
    }

    // The parameters are encoded as a bit mask: HHHIICC:
    // 3 bits for the hardware (H)
    // 2 bits for the instance (I)
    // 2 bits for the channel (C)
    uint8 speaker = static_cast<uint8>(static_cast<uint8>(aChannel) | 
        (aInstance << 2) | 
        (static_cast<uint8>(aHardware) << 4));

    ostringstream msg;
    msg << "AT+AUDIOPLAYTONE=" << static_cast<uint16>(aTone) << ","
        << static_cast<uint16>(speaker) << "," << MAX_DURATION_MS;

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::AudioToneStop()
{
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact("AT+AUDIOPLAYTONE=0,0,0", STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::AudioLoopbackStart(uint8 aMic, AudioHardware aOutHardware,
    uint8 aOutInstance, AudioChannel aOutChannel)
{
    static const uint16 SAMPLE_RATE_HZ = 44100;

    // The parameters are encoded as a bit mask: HHHIICC:
    // 3 bits for the hardware (H)
    // 2 bits for the instance (I)
    // 2 bits for the channel (C)
    uint8 speaker = static_cast<uint8>(
        static_cast<uint8>(aOutChannel) |
        (aOutInstance << 2) |
        (static_cast<uint8>(aOutHardware) << 4));

    ostringstream msg;
    msg << "AT+LOOPBACKSTART=" << static_cast<uint16>(aMic) << ","
        << static_cast<uint16>(speaker) << "," << SAMPLE_RATE_HZ;

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::AudioLoopbackStop()
{
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact("AT+LOOPBACKSTOP", STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsDevice::DetectTouch(uint16 aTimeoutS, TouchType aType)
{
    static const regex REGEXP_TOUCH_REPORT("\\+TOUCH:(\\d+),(\\d+)");

    bool touchDetected = false;

    // Special case for ANY touch action detection to speed up a no touch
    // detection test - uses stop_after_report mode.
    if (aType == TouchType::ANY)
    {
        ostringstream msg;
        msg << "AT+TESTTOUCH=" << aTimeoutS << ",1";

        vector<string> resp = mpAtMessenger->Transact(msg.str(),
            ((aTimeoutS * 1000) + STD_READ_TIMEOUT_MS));

        if (!resp.empty())
        {
            StripRespPrefix(resp, "+TOUCH");

            touchDetected = true;
        }
    }
    else
    {
        // Not using stop_after_report mode, because it's difficult to repeatably
        // perform an action without it sometimes being misidentified, e.g. touch
        // detected as a swipe or hand cover. So we wait for a report for the action
        // type we're looking for within the timeout, ignoring other reports.
        // Using a fixed timeout for the command as we will stop it ourselves before
        // it ends (70 seconds is greater than the max 60 seconds allowed for the
        // user configured test timeout).
        mpAtMessenger->Write("AT+TESTTOUCH=70,0");

        // Read until a report arrives for the touch action we're looking for, or timeout
        time_point<steady_clock> startTime = steady_clock::now();
        do
        {
            string readStr = mpAtMessenger->Read(64, true);
            if (!readStr.empty())
            {
                vector<string> lines = PtUtil::SplitString(readStr.c_str(), AT_NEWLINE);

                for (string line : lines)
                {
                    if (!line.empty())
                    {
                        cmatch match;
                        if (!regex_match(line.c_str(), match, REGEXP_TOUCH_REPORT))
                        {
                            ostringstream msg;
                            msg << "Invalid device response \"" << line
                                << "\", received in touch test mode";
                            throw CDutException(msg.str());
                        }

                        uint16 actionTypeVal;
                        istringstream istr(match.str(1));
                        if (!(istr >> actionTypeVal) || !istr.eof())
                        {
                            ostringstream msg;
                            msg << "Reported touch action \"" << match.str(1) << "\" is invalid";
                            throw CDutException(msg.str());
                        }

                        if (static_cast<uint16>(aType) == actionTypeVal)
                        {
                            touchDetected = true;
                        }
                    }
                }
            }

            if (!touchDetected)
            {
                this_thread::sleep_for(milliseconds(100));
            }
        } while (!touchDetected &&
            duration_cast<milliseconds>(steady_clock::now() - startTime) < milliseconds(aTimeoutS * 1000));

        // End the touch test
        (void)mpAtMessenger->Transact("AT+TESTTOUCH=0,1", STD_READ_TIMEOUT_MS);
    }

    return touchDetected;
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsDevice::TestProximity(uint16 aTimeoutS)
{
    // Using timeout and stop_after_report mode
    ostringstream msg;
    msg << "AT+TESTPROXIMITY=" << aTimeoutS << ",1";

    vector<string> resp = mpAtMessenger->Transact(msg.str(),
        ((aTimeoutS * 1000) + STD_READ_TIMEOUT_MS));

    StripRespPrefix(resp, "+PROXIMITY");

    bool changeDetected = false;
    // Should be 2 differing indications. If there's only
    // 1 indication, it's a test fail, as a change wasn't detected
    if (resp.size() == 2)
    {
        // Sanity check, device shouldn't write 2nd response unless there's a change
        if (resp.at(0) == resp.at(1))
        {
            throw CDutException("Device response indicates no proximity status change");
        }

        changeDetected = true;
    }

    return changeDetected;
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsDevice::TestHallEffect(uint16 aTimeoutS)
{
    // Using timeout and stop_after_report mode
    ostringstream msg;
    msg << "AT+TESTHALLEFFECT=" << aTimeoutS << ",1";

    vector<string> resp = mpAtMessenger->Transact(msg.str(),
        ((aTimeoutS * 1000) + STD_READ_TIMEOUT_MS));

    StripRespPrefix(resp, "+HALLEFFECT");

    bool changeDetected = false;
    // Should be 2 differing indications. If there's only
    // 1 indication, it's a test fail, as a change wasn't detected
    if (resp.size() == 2)
    {
        // Sanity check, device shouldn't write 2nd response unless there's a change
        if (resp.at(0) == resp.at(1))
        {
            throw CDutException("Device response indicates no hall effect sensor change");
        }

        changeDetected = true;
    }

    return changeDetected;
}

////////////////////////////////////////////////////////////////////////////////

int16 CCdaDtsDevice::GetTemperatureDegC()
{
    vector<string> resp = mpAtMessenger->Transact("AT+TEMPERATURE?", STD_READ_TIMEOUT_MS);

    StripRespPrefix(resp, "+TEMPERATURE");

    if (resp.empty())
    {
        throw CDutException("No temperature value reported by device");
    }

    int16 tempC;
    istringstream istr(resp.at(0));
    if (!(istr >> tempC) || !istr.eof())
    {
        ostringstream msg;
        msg << "Reported temperature value \"" << resp.at(0) << "\" is invalid";
        throw CDutException(msg.str());
    }

    return tempC;
}

////////////////////////////////////////////////////////////////////////////////

uint16 CCdaDtsDevice::GetBatteryMv()
{
    vector<string> resp = mpAtMessenger->Transact("AT+BATTERYLEVEL?", STD_READ_TIMEOUT_MS);

    StripRespPrefix(resp, "+BATTERYLEVEL");

    if (resp.empty())
    {
        throw CDutException("No battery level reported by device");
    }

    uint16 battMv;
    istringstream istr(resp.at(0));
    if (!(istr >> battMv) || !istr.eof())
    {
        ostringstream msg;
        msg << "Reported battery value \"" << resp.at(0) << "\" is invalid";
        throw CDutException(msg.str());
    }

    return battMv;
}

////////////////////////////////////////////////////////////////////////////////

int16 CCdaDtsDevice::GetRssiDbm()
{
    vector<string> resp = mpAtMessenger->Transact("AT+RSSIREAD", STD_READ_TIMEOUT_MS);

    StripRespPrefix(resp, "+RSSIREAD");

    if (resp.empty())
    {
        throw CDutException("No RSSI report received from device");
    }
    else if (resp.size() > 1)
    {
        throw CDutException("Can't determine RSSI because device has >1 active connection");
    }

    vector<string> values = PtUtil::SplitString(resp.at(0).c_str(), ",");
    if (values.empty() || values.size() > 2)
    {
        ostringstream msg;
        msg << "Device RSSI report values \"" << resp.at(0)
            << "\" are invalid, expecting \"<address>,<rssi>\"";
        throw CDutException(msg.str());
    }

    int16 rssiDbm;
    istringstream istr(values.at(1));
    if (!(istr >> rssiDbm) || !istr.eof())
    {
        ostringstream msg;
        msg << "Reported RSSI \"" << values.at(1) << "\" is invalid";
        throw CDutException(msg.str());
    }

    return rssiDbm;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::RadioCfgStopPio(uint8 aPio)
{
    ostringstream msg;
    msg << "AT+RFTESTCFGSTOPPIO=" << static_cast<uint16>(aPio);
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::RadioCfgStopTime(uint8 aTimeSeconds)
{
    // Use reboot - without that currently can't reconnect after RF test stops.
    ostringstream msg;
    msg << "AT+RFTESTCFGSTOPTIME=1," << (aTimeSeconds * 1000);
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::RadioCfgStopTouch()
{
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact("AT+RFTESTCFGSTOPTOUCH", STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::RadioCfgTxPower(uint8 aPowerLevel)
{
    ostringstream msg;
    msg << "AT+RFTESTCFGPOWER=" << static_cast<uint16>(aPowerLevel);
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::RadioTxCwStart(uint8 aChannel)
{
    ostringstream msg;
    msg << "AT+RFTESTCARRIER=" << static_cast<uint16>(aChannel);
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);

    // Connection will drop, disconnect COM port to keep things in sync
    DisconnectComPort();
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::PsSetValue(uint16 aKeyId, std::vector<uint16> aValue)
{
    ostringstream cmd;
    cmd << "AT+PSKEYSET=" << aKeyId << ",";
    for (std::vector<uint16>::const_iterator iVal = aValue.begin();
        iVal != aValue.end();
        ++iVal)
    {
        cmd << hex << uppercase << setfill('0') << setw(4) << *iVal;
        if (iVal < (aValue.end() - 1))
        {
            cmd << ' ';
        }
    }

    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(cmd.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::PsClear(uint16 aKeyId)
{
    ostringstream cmd;
    cmd << "AT+PSKEYCLEAR=" << aKeyId;
    // Nothing in the response we care about other than OK
    (void)mpAtMessenger->Transact(cmd.str(), STD_READ_TIMEOUT_MS);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::Authenticate()
{
    static const string START_CMD("AUTHSTART");
    static const string RESP_CMD("AUTHRESP");
    static const size_t AUTH_CHALLENGE_NUM_BYTES = 16;

    vector<string> resp = mpAtMessenger->Transact("AT+" + START_CMD, STD_READ_TIMEOUT_MS);

    StripRespPrefix(resp, "+" + START_CMD);
    if (resp.empty())
    {
        throw CDutException("Device authentication start response not received");
    }
    
    // Check the nonce length
    const string& nonceStr = resp.at(0);
    if (nonceStr.size() != AUTH_CHALLENGE_NUM_BYTES * 2)
    {
        ostringstream msg;
        msg << "DTS authentication start response (nonce) \"" << nonceStr
            << "\" is not the correct length (" << AUTH_CHALLENGE_NUM_BYTES * 2 << ")";
        throw CDutException(msg.str());
    }

    // Check nonce chars are all valid hex digits
    if (!std::all_of(nonceStr.begin(), nonceStr.end(), ::isxdigit))
    {
        ostringstream msg;
        msg << "DTS authentication start nonceStr \"" << nonceStr
            << "\" contains invalid hex digits";
        throw CDutException(msg.str());
    }

    // Convert the string of hex bytes to an array of values
    uint8 nonce[AUTH_CHALLENGE_NUM_BYTES];
    stringstream converter;
    for (size_t index = 0; index < AUTH_CHALLENGE_NUM_BYTES; ++index)
    {
        converter << nonceStr.substr(index * 2, 2);
        uint16 val; // uint16 to avoid interpretation as char
        converter >> hex >> val;
        nonce[index] = static_cast<uint8>(val);
        converter.clear();
    }

    // Do the AES-CMAC calculation
    uint8 result[AUTH_CHALLENGE_NUM_BYTES] = { 0 };
    CalcAesCmac(mAuthKey, AUTH_KEY_NUM_OCTETS, nonce, AUTH_CHALLENGE_NUM_BYTES, result);

    // Convert the AES-CMAC result into a string of hex digits
    ostringstream respCode;
    for (size_t i = 0; i < AUTH_CHALLENGE_NUM_BYTES; ++i)
    {
        respCode << hex << uppercase << setfill('0') << setw(2) << (uint16)result[i];
    }

    try
    {
        // Send the response - nothing in the reply we're interested in other than "OK".
        ostringstream msg;
        msg << "AT+" << RESP_CMD << ':' << respCode.str();
        (void)mpAtMessenger->Transact(msg.str(), STD_READ_TIMEOUT_MS);
    }
    catch(CAtMsgrCmdErrorException&)
    {
        // Provide a more user friendly error in this case
        throw CDutException("DTS authentication failed. Check that the key specified in the setup is correct for the DUT");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::CalcAesCmac(const uint8* apKey, size_t aKeyLen,
    const uint8* apMsgIn, size_t aMsgLen, uint8* apMsgOut)
{
    size_t resultLen;
    CMAC_CTX *pCtx = CMAC_CTX_new();
    if (pCtx == NULL)
    {
        throw CDutException("DTS authentication failed: Couldn't create OpenSSL CMAC_CTX object");
    }

    if (CMAC_Init(pCtx, apKey, aKeyLen, EVP_aes_128_cbc(), NULL) == 0)
    {
        throw CDutException("DTS authentication failed: OpenSSL CMAC_Init failed");
    }

    if (CMAC_Update(pCtx, apMsgIn, aMsgLen) == 0)
    {
        throw CDutException("DTS authentication failed: OpenSSL CMAC_Update failed");
    }

    if (CMAC_Final(pCtx, apMsgOut, &resultLen) == 0)
    {
        throw CDutException("DTS authentication failed: OpenSSL CMAC_Final failed");
    }
    else if (resultLen != aMsgLen)
    {
        throw CDutException("DTS authentication failed: OpenSSL CMAC_Final output length does not match input");
    }

    CMAC_CTX_free(pCtx);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::ConnectComPort(uint16 aPortNum)
{
    ostringstream portNameStr;
    portNameStr << "\\\\.\\COM" << aPortNum;

    delete mpSerialPort;
    // Baud rate isn't relevant for SPP link, so using fixed value
    mpSerialPort = new CPtSerialPort(portNameStr.str(), 115200);

    delete mpAtMessenger;
    mpAtMessenger = new CAtMessenger(*mpSerialPort, AT_NEWLINE, false);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::DisconnectComPort()
{
    delete mpAtMessenger;
    mpAtMessenger = NULL;

    delete mpSerialPort;
    mpSerialPort = NULL;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::StripRespPrefix(std::vector<std::string>& aResponses, const std::string& aPrefix)
{
    for (string& resp : aResponses)
    {
        vector<string> parts = PtUtil::SplitString(resp.c_str(), ":");
        if (parts.empty() || parts.size() > 2)
        {
            ostringstream msg;
            msg << "Device response line \"" << resp
                << "\" format is invalid, expecting \"" << aPrefix 
                << ":\" followed by response data";
            throw CDutException(msg.str());
        }

        if (parts.at(0) != aPrefix)
        {
            ostringstream msg;
            msg << "Device response line \"" << resp
                << "\" does not contain expected \"" << aPrefix << "\" prefix";
            throw CDutException(msg.str());
        }

        resp = parts.at(1);
    }
}

////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI CCdaDtsDevice::BtAuthCallback(LPVOID apParam,
    PBLUETOOTH_AUTHENTICATION_CALLBACK_PARAMS apAuthCallbackParams)
{
    BLUETOOTH_AUTHENTICATE_RESPONSE authResp;
    authResp.authMethod = apAuthCallbackParams->authenticationMethod;
    authResp.bthAddressRemote = apAuthCallbackParams->deviceInfo.Address;
    authResp.negativeResponse = FALSE;
    // Responding with numerical value indicating using "Just Works" pairing
    authResp.numericCompInfo.NumericValue = 1;

    AuthContext* pContext = static_cast<AuthContext*>(apParam);

    // Send authentication response
    DWORD res = BluetoothSendAuthenticationResponseEx(NULL, &authResp);
    if (res != ERROR_SUCCESS)
    {
        // Can't throw from here are this is a C callback, and it runs on it's own thread to complete
        // authentication, so just logging the issue.
        if (res == ERROR_CANCELLED)
        {
            pContext->mErrMsg = "BT authentication failed: Device denied passkey response or a communication problem occurred";
        }
        else if (res == E_FAIL)
        {
            pContext->mErrMsg = "BT authentication failed: Device returned a failure code";
        }
        else
        {
            ostringstream msg;
            msg << "BT authentication failed (" << PtUtil::GetLastWinErrorMessage() << ")";
            pContext->mErrMsg = msg.str();
        }
    }

    // Not much we can do in this callback if this fails, thread waiting will timeout.
    (void)SetEvent(pContext->mCompleteEvent);

    return (res == ERROR_SUCCESS ? TRUE : FALSE); // This value is ignored
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDtsDevice::FindDevice(bool aPairedOnly)
{
    static const float64 BT_INQ_TIMEOUT_DIVISOR = 1.28;
    const float64 tmoMultiplier = ceil(mBtInquiryTimeoutSeconds / BT_INQ_TIMEOUT_DIVISOR);

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams =
    {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        TRUE,  // Authenticated devices
        (aPairedOnly ? FALSE : TRUE),  // Remembered devices
        (aPairedOnly ? FALSE : TRUE),  // Unknown devices
        TRUE,  // Connected devices
        (aPairedOnly ? FALSE : TRUE),  // Issue inquiry
        static_cast<UCHAR>(tmoMultiplier),  // Timeout multiplier for the inquiry, in units of 1.28 seconds
        NULL  // Radio handle: NULL = Use all
    };

    BLUETOOTH_DEVICE_INFO devInfo;
    ZeroMemory(&devInfo, sizeof(BLUETOOTH_DEVICE_INFO));
    devInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

    bool devFound = false;

    // Start the search
    HBLUETOOTH_DEVICE_FIND hBtDevFind = BluetoothFindFirstDevice(&searchParams, &devInfo);
    if (hBtDevFind != NULL)
    {
        try
        {
            do
            {
                // Is this is device we are looking for?
                if (devInfo.Address.ullLong == mBdAddress.ullLong)
                {
                    devFound = true;
                }

                // Print the details of the device we're looking for, or all devices when doing an inquiry
                if (!aPairedOnly || devFound)
                {
                    // Show info for device found
                    wostringstream devMsg;
                    devMsg << L"Found device:" << endl;
                    devMsg << L"\tInstance Name: " << devInfo.szName << endl;
                    devMsg << L"\tAddress: 0x" << hex << setw(12) << setfill(L'0') << devInfo.Address.ullLong << endl;
                    devMsg << L"\tConnected: " << (devInfo.fConnected == FALSE ? L"false" : L"true") << endl;
                    devMsg << L"\tAuthenticated: " << (devInfo.fAuthenticated == FALSE ? L"false" : L"true") << endl;
                    devMsg << L"\tRemembered: " << (devInfo.fRemembered == FALSE ? L"false" : L"true");
                    mUi.Write(devMsg.str(), false);
                }
            } while (!devFound && BluetoothFindNextDevice(hBtDevFind, &devInfo));
        }
        catch (...)
        {
            // Close the find handle
            //
            if (BluetoothFindDeviceClose(hBtDevFind) == FALSE)
            {
                ostringstream failMsg;
                failMsg << "BluetoothFindDeviceClose failed after connection attempt ("
                    << PtUtil::GetLastWinErrorMessage() << ")";
                mUi.Write(failMsg.str(), true);
            }

            throw;
        }

        // Close the find handle
        if (BluetoothFindDeviceClose(hBtDevFind) == FALSE)
        {
            ostringstream failMsg;
            failMsg << "BluetoothFindDeviceClose failed after connection attempt ("
                << PtUtil::GetLastWinErrorMessage() << ")";
            throw CDutException(failMsg.str());
        }
    }

    if (devFound)
    {
        // Save the name for later reporting
        mDeviceName = devInfo.szName;
    }

    return devFound;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::PairDevice(BLUETOOTH_DEVICE_INFO aDevInfo)
{
    static const DWORD AUTH_TIMEOUT_MS = 10000;

    HBLUETOOTH_AUTHENTICATION_REGISTRATION hRegHandle = 0;

    // Register the callback to intercept authentication (avoid user input)
    // Use the context to wait for completion and allow messages to be logged
    AuthContext authContext;
    DWORD errorCode = BluetoothRegisterForAuthenticationEx(&aDevInfo, &hRegHandle,
        (PFN_AUTHENTICATION_CALLBACK_EX)&CCdaDtsDevice::BtAuthCallback, &authContext);
    if (errorCode != ERROR_SUCCESS)
    {
        ostringstream msg;
        msg << "Pairing failed: BluetoothRegisterForAuthenticationEx error = " << errorCode;
        throw CDutException(msg.str());
    }

    // Start pairing
    const time_point<steady_clock> startTime = steady_clock::now();
    errorCode = BluetoothAuthenticateDeviceEx(NULL, NULL, &aDevInfo, NULL, MITMProtectionNotRequired);
    if (errorCode != ERROR_SUCCESS)
    {
        // Unregister the callback for BT authentication
        if (BluetoothUnregisterAuthentication(hRegHandle) == FALSE)
        {
            ostringstream msg;
            msg << "Pairing failed: Failed to unregister authentication callback ("
                << PtUtil::GetLastWinErrorMessage() << ")";
            // Don't throw for the failure to unregister the call back, as we want to
            // throw for the authentication failure. Just log it.
            mUi.Write(msg.str(), true);
        }

        switch (errorCode)
        {
        case(ERROR_CANCELLED):
            throw CDutException("Pairing failed: User abort");
            break;
        case(ERROR_INVALID_PARAMETER):
            throw CDutException("Pairing failed: Invalid parameters");
            break;
        case(ERROR_NO_MORE_ITEMS):
            throw CDutException("Pairing failed: Device already authenticated");
            break;
        case(WAIT_TIMEOUT):
            throw CDutException("Pairing failed: Device not available");
            break;
        default:
            ostringstream msg;
            msg << "Pairing failed: BluetoothAuthenticateDeviceEx returned unexpected return code "
                << errorCode;
            throw CDutException(msg.str());
            break;
        }
    }

    // Wait for authentication callback to complete.
    string errStr;
    DWORD waitRes = WaitForSingleObject(authContext.mCompleteEvent, AUTH_TIMEOUT_MS);
    if (waitRes == WAIT_OBJECT_0 && !authContext.mErrMsg.empty())
    {
        errStr = authContext.mErrMsg;
    }
    else if (waitRes == WAIT_TIMEOUT)
    {
        ostringstream msg;
        msg << "Pairing failed: Timed out waiting for authentication after " << AUTH_TIMEOUT_MS << " ms";
        errStr = msg.str();
    }
    else if (waitRes != WAIT_OBJECT_0)
    {
        ostringstream msg;
        msg << "Pairing failed: WaitForSingleObject returned unexpected value waiting for authentication ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        errStr = msg.str();
    }

    // Unregister the callback for BT authentication
    if (BluetoothUnregisterAuthentication(hRegHandle) == FALSE)
    {
        ostringstream msg;
        msg << "Failed to unregister authentication callback after pairing ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        if (errStr.empty())
        {
            throw CDutException(msg.str());
        }
        else
        {
            // Don't throw for the failure to unregister the call back, as we want to
            // throw for the authentication failure. Just log it.
            mUi.Write(msg.str(), true);
        }
    }

    if (!errStr.empty())
    {
        throw CDutException(errStr);
    }

    const milliseconds durationMs = duration_cast<milliseconds>(steady_clock::now() - startTime);
    ostringstream msg;
    msg << "Device paired successfully (after " << durationMs.count() << " ms)";
    mUi.Write(msg.str(), false);
    
    mPaired = true;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::ConnectBt()
{
    if (!mPaired)
    {
        ostringstream msg;
        msg << "Searching for device address " << mDeviceAddress;
        mUi.Write(msg.str(), false);

        mPrePairedDevice = mPaired = FindDevice(true);

        if (mPrePairedDevice)
        {
            if (mPrePairedDutsAllowed)
            {
                mUi.Write("Device already paired", false);
            }
            else
            {
                throw CDutException("Connection failed: Device already paired, please remove and retry (or update the settings to allow pre-paired DUTs).");
            }
        }
        else
        {
            ostringstream pairMsg;
            pairMsg << "Attempting to pair with device address " << mDeviceAddress;
            mUi.Write(pairMsg.str(), false);

            BLUETOOTH_DEVICE_INFO devInfo;
            ZeroMemory(&devInfo, sizeof(BLUETOOTH_DEVICE_INFO));
            devInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
            devInfo.Address.ullLong = mBdAddress.ullLong;

            try
            {
                PairDevice(devInfo);                
            }
            catch (CDutException &ex)
            {
                bool devFound = false;
                if (mInquiryOnPairingFail)
                {
                    devFound = FindDevice(false);
                }

                ostringstream errMsg;
                errMsg << "Unable to pair (" << ex.what() << ")";
                if (devFound)
                {
                    errMsg << ". Device found";
                }
                throw CDutException(errMsg.str());
            }

            // Request access to the SPP service, which should bring up a virtual COM port
            DWORD ret = BluetoothSetServiceState(NULL, &devInfo, &SerialPortServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE);
            if (ret != ERROR_SUCCESS)
            {
                if (ret == ERROR_INVALID_PARAMETER)
                {
                    throw CDutException("Failed to start SPP: Invalid Parameter");
                }
                else if (ret == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    throw CDutException("Failed to start SPP: Service not found");
                }
                else
                {
                    ostringstream errMsg;
                    errMsg << "Failed to start SPP: Error code = " << ret;
                    throw CDutException(errMsg.str());
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDtsDevice::DisconnectBt()
{
    DWORD res = BluetoothRemoveDevice(&mBdAddress);
    if (res == ERROR_SUCCESS)
    {
        mUi.Write("Device disconnected", false);
    }
    else if (res != ERROR_NOT_FOUND) // OK if not found, already disconnected
    {
        ostringstream msg;
        msg << "ERROR: Disconnection failed with unexpected error code " << res;
        // Don't throw, called from destructor
        mUi.Write(msg.str());
    }

    mPaired = false;
}

////////////////////////////////////////////////////////////////////////////////

uint16 CCdaDtsDevice::FindComPort()
{
    // Device property "Bus reported device description"
    static const wstring DTS_BUS_REPORTED_DEVICE_DESC(L"DTS");
    // This isn't defined in devpkey.h (unlike DEVPKEY_Device_BusReportedDeviceDesc
    // for example), but managed to determine the values by enumerating through
    // all properties using SetupDiGetDevicePropertyKeys.
    static const DEVPROPKEY DEVPKEY_BluetoothAddress =
    { { 0x2BD67D8B, 0x8BEB, 0x48D5,{ 0x87, 0xE0, 0x6C, 0xDA, 0x34, 0x28, 0x04, 0x0A } }, 1 };

    // Need to allow time for the COM port to come up after pairing. During
    // this time the port might show up, but reading the properties could fail
    // until the setup is complete, so need retries within a timeout.
    static const milliseconds TIMEOUT_MS(10000);

    string lastErrorStr;
    uint16 portNum = 0;
    milliseconds durationMs(0);
    const time_point<steady_clock> startTime = steady_clock::now();

    do
    {
        HDEVINFO hDevInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
        if (hDevInfoSet == INVALID_HANDLE_VALUE)
        {
            ostringstream msg;
            msg << "Failed to find COM port: SetupDiGetClassDevs failed ("
                << PtUtil::GetLastWinErrorMessage() << ")";
            throw CDutException(msg.str());
        }

        SP_DEVINFO_DATA devInfoData;
        ZeroMemory(&devInfoData, sizeof(devInfoData));
        devInfoData.cbSize = sizeof(devInfoData);

        try
        {
            for (DWORD index = 0;
                SetupDiEnumDeviceInfo(hDevInfoSet,
                    index,
                    &devInfoData);
                ++index)
            {
                // For BDADDR, "Bluetooth device address": E.g. "00025B00FF00".
                // "Friendly name": E.g. "Standard Serial over Bluetooth link (COM14)"
                WCHAR wStrBuff[1024];
                DEVPROPTYPE propertyType;

                // Query Bluetooth device address.
                // (not an error if not found, port may not be a Bluetooth port).
                if (SetupDiGetDevicePropertyW(hDevInfoSet,
                    &devInfoData,
                    &DEVPKEY_BluetoothAddress,
                    &propertyType,
                    (PBYTE)wStrBuff,
                    sizeof(wStrBuff),
                    NULL,
                    0) != FALSE)
                {
                    wistringstream wiStrBdAddr(wStrBuff);
                    BTH_ADDR bdAddr;
                    wiStrBdAddr >> hex >> bdAddr;

                    if (bdAddr == mBdAddress.ullLong)
                    {
                        // Query "Bus resported device description"
                        if (SetupDiGetDevicePropertyW(hDevInfoSet,
                            &devInfoData,
                            &DEVPKEY_Device_BusReportedDeviceDesc,
                            &propertyType,
                            (PBYTE)wStrBuff,
                            sizeof(wStrBuff),
                            NULL,
                            0) == FALSE)
                        {
                            // Might fail during setup, so don't throw
                            ostringstream msg;
                            msg << "SetupDiGetDevicePropertyW failed to read \"Bus reported device description\" ("
                                << PtUtil::GetLastWinErrorMessage() << ")";
                            lastErrorStr = msg.str();
                        }

                        wstring desc(wStrBuff);
                        if (desc == DTS_BUS_REPORTED_DEVICE_DESC)
                        {
                            // This is the expected port for the device we paired with, so get the COM port number
                            if (SetupDiGetDevicePropertyW(hDevInfoSet,
                                &devInfoData,
                                &DEVPKEY_Device_FriendlyName,
                                &propertyType,
                                (PBYTE)wStrBuff,
                                sizeof(wStrBuff),
                                NULL,
                                0) == FALSE)
                            {
                                // Might fail during setup, so don't throw
                                ostringstream msg;
                                msg << "SetupDiGetDevicePropertyW failed to read \"Friendly name\" ("
                                    << PtUtil::GetLastWinErrorMessage() << ")";
                                lastErrorStr = msg.str();
                            }
                            else
                            {
                                wstring name(wStrBuff);
                                static const wregex REGEXP_FRIENDLY_NAME(L".+\\(COM(\\d+)\\)$");
                                wcmatch match;
                                if (regex_match(name.c_str(), match, REGEXP_FRIENDLY_NAME))
                                {
                                    wistringstream wiStr(match.str(1));
                                    wiStr >> portNum;
                                }
                                else
                                {
                                    // If we read the friendly name from our target DUT, this should always work, so throw.
                                    ostringstream msg;
                                    msg << "Failed to find COM port: Couldn't find port number in friendly name";
                                    throw CDutException(msg.str());
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (...)
        {
            if (SetupDiDestroyDeviceInfoList(hDevInfoSet) == FALSE)
            {
                ostringstream msg;
                msg << "SetupDiDestroyDeviceInfoList failed after attempting to find BT COM port ("
                    << PtUtil::GetLastWinErrorMessage() << ")";
                mUi.Write(msg.str(), false);
            }

            throw;
        }

        if (SetupDiDestroyDeviceInfoList(hDevInfoSet) == FALSE)
        {
            ostringstream msg;
            msg << "SetupDiDestroyDeviceInfoList failed after attempting to find BT COM port ("
                << PtUtil::GetLastWinErrorMessage() << ")";
            throw CDutException(msg.str());
        }

        durationMs = duration_cast<milliseconds>(steady_clock::now() - startTime);
        if (portNum == 0 && durationMs < TIMEOUT_MS)
        {
            this_thread::sleep_for(seconds(1));
        }
    } while (portNum == 0 && durationMs < TIMEOUT_MS);

    if (portNum == 0)
    {
        ostringstream msg;
        msg << "Could not find DTS COM port for device after " << TIMEOUT_MS.count() << " ms";
        if (!lastErrorStr.empty())
        {
            msg << " (error: " << lastErrorStr << ")";
        }
        throw CDutException(msg.str());
    }
    else
    {
        ostringstream msg;
        msg << "Found device DTS port COM" << portNum << " (after " << durationMs.count() << " ms)";
        mUi.Write(msg.str(), false);
    }

    return portNum;
}

////////////////////////////////////////////////////////////////////////////////

CCdaDtsDevice::AuthContext::AuthContext()
    : mCompleteEvent(NULL)
{
    mCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (mCompleteEvent == NULL)
    {
        ostringstream msg;
        msg << "Failed to create event handle ("
            << PtUtil::GetLastWinErrorMessage() << ")";
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

CCdaDtsDevice::AuthContext::~AuthContext()
{
    CloseHandle(mCompleteEvent);
}
