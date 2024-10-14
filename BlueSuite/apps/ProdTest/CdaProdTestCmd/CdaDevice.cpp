//**************************************************************************************************
//
//  CdaDevice.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  CDA device class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "CdaDevice.h"
#include "PtSetup.h"
#include "PtUtil.h"
#include "PtBdAddrMgr.h"
#include "PtTimer.h"
#include "hci\TestEngine.h"
#include "spi\TestFlash.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace QTIL;
using namespace std;
using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////

const char* const CCdaDevice::APP_DISABLE_SETTING_NAME = "DisableDutApp";

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::GetAvailableTransports(TransportsMap& aTransports)
{
    uint16 maxLen(256);
    uint16 count(0);
    char* pPortsStr = new char[maxLen]; // The human readable port strings (e.g. "USB TRB (151134)")
    char* pTransStr = new char[maxLen]; // The transport option strings (e.g. "SPITRANS=TRB SPIPORT=1")

    int32 status = teGetAvailableDebugPorts(&maxLen, pPortsStr, pTransStr, &count);
    if (status != TE_OK && maxLen != 0)
    {
        // Not enough space - resize the storage
        pPortsStr = new char[maxLen];
        pTransStr = new char[maxLen];
        status = teGetAvailableDebugPorts(&maxLen, pPortsStr, pTransStr, &count);
    }

    if (status == TE_OK && count > 0)
    {
        // Split up the comma separated strings of ports / transport options
        vector<string> ports = PtUtil::SplitString(pPortsStr, ",");
        vector<string> trans = PtUtil::SplitString(pTransStr, ",");

        for (size_t index = 0; index < ports.size(); ++index)
        {
            aTransports.insert({ ports.at(index), trans.at(index) });
        }
    }

    delete[] pPortsStr;
    delete[] pTransStr;

    if (status != TE_OK)
    {
        throw CDutException("Failed to get available transports");
    }
}

////////////////////////////////////////////////////////////////////////////////

CCdaDevice::CCdaDevice(const CPtSetup& aSetup)
    : CDut(aSetup), mResetWaitMs(0), mTeHandle(TE_INVALID_HANDLE_VALUE),
      mConfigInitialised(false), mConfigUpdated(false),
      mUsesQHciRadioApis(false), mCfgWriteLayer(0), mDisableApp(false),
      mResetOnClose(true)
{
    // Validate the mandatory settings here so that we flag the problem
    // during initialisation rather than when we hit a test that needs it.
    static const string FW_PATH_SETTING = "FwPath";
    static const string DUT_RESET_WAIT_SETTING = "DutResetWaitMs";
    static const string TRANSPORT_SETTING = "Transport";
    static const string CFG_DB_SETTING = "ConfigDb";
    static const string CFG_WRITE_LAYER_SETTING = "ConfigWriteLayer";
    static const uint16 MIN_CFG_LAYER = 6; // Chosen because build defaults may be set in lower layers
    static const uint16 MAX_CFG_LAYER = 15;

    // Optional firmware path
    mFirmwarePath = mSetup.GetValue(FW_PATH_SETTING, false);
    if (!mFirmwarePath.empty())
    {
        if (!PtUtil::FileExists(mFirmwarePath))
        {
            ostringstream msg;
            msg << "Configuration setting \"" << FW_PATH_SETTING
                << "\" specifies file \"" << mFirmwarePath 
                << "\", which does not exist (or cannot be opened)";
            throw CDutException(msg.str());
        }
    }

    // Reset wait time
    mResetWaitMs = mSetup.GetValueNum<size_t>(DUT_RESET_WAIT_SETTING);

    // Transport
    mTransport = mSetup.GetValue(TRANSPORT_SETTING, false);
        
    // Config database
    mCfgDbPath = mSetup.GetValue(CFG_DB_SETTING, true);
    // Error on empty string or if the separator is the last character (no
    // separator should be present if the system version label isn't specified).
    if (mCfgDbPath.empty() || (mCfgDbPath.rfind(':') == mCfgDbPath.size() - 1))
    {
        ostringstream msg;
        msg << "Configuration setting \"" << CFG_DB_SETTING
            << "\" must be a valid file path to a configuration database (*.sdb file). E.g. "
            << "\"hyd.sdb\", or with system version label \"hyd.sdb:QCC512X_CONFIG\"";
        throw CDutException(msg.str());
    }

    // Write layer
    mCfgWriteLayer = mSetup.GetValueNum<uint16>(CFG_WRITE_LAYER_SETTING);
    if (mCfgWriteLayer < MIN_CFG_LAYER || mCfgWriteLayer > MAX_CFG_LAYER)
    {
        ostringstream msg;
        msg << "Configuration setting \"" << CFG_WRITE_LAYER_SETTING
            << "\" is out of range (min = " << MIN_CFG_LAYER
            << ", max = " << MAX_CFG_LAYER << ")";
        throw CDutException(msg.str());
    }

    mDisableApp = mSetup.GetValueNum<bool>(APP_DISABLE_SETTING_NAME);

    mResetOnClose = mSetup.GetValueNum<bool>("DutResetOnClose");
}

////////////////////////////////////////////////////////////////////////////////

CCdaDevice::~CCdaDevice()
{
    Disconnect();
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::Connect()
{
    if (mTransport.empty())
    {
        mTransport = AutoDetectTransport();

        ostringstream msg;
        msg << "Detected device transport \"" << mTransport << "\"";
        mUi.Write(msg.str(), false);
    }

    // Burn firmware first before connecting with TestEngine
    if (!mFirmwarePath.empty())
    {
        BurnFirmware();

        // Device gets reset after the firmware is burned. Wait until it comes
        // back up (and transport re-enumerates in the case of USBDBG).
        ostringstream msg;
        msg << "Waiting " << mResetWaitMs << " ms after DUT reset...";
        mUi.Write(msg.str());

        this_thread::sleep_for(milliseconds(mResetWaitMs));
    }

    mTeHandle = openTestEngineDebugTrans(mTransport.c_str(), 0);

    if (mTeHandle == TE_INVALID_HANDLE_VALUE)
    {
        ostringstream msg;
        msg << "Failed to connect to device using transport \"" << mTransport << "\"";
        throw CDutException(msg.str());
    }
    else
    {
        // Identify the chip
        ostringstream msg;
        msg << "Connected to " << Identify() << " device";
        mUi.Write(msg.str(), false);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::Disconnect()
{
    if (mTeHandle != TE_INVALID_HANDLE_VALUE)
    {
        const uint16 options = (mResetOnClose ? TE_CLOSE_EX_RESET_RELOCK_NO_WAIT : TE_CLOSE_EX_NO_RESET_NO_RELOCK);
        closeTestEngineEx(mTeHandle, options);
        mTeHandle = TE_INVALID_HANDLE_VALUE;
    }

    mConfigInitialised = false;
    mConfigUpdated = false;
    mUsesQHciRadioApis = false;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::Reset(ResetMode aMode)
{
    if (teChipReset(mTeHandle, static_cast<uint32>(aMode)) != TE_OK)
    {
        throw CDutException("teChipReset failed");
    }
    else
    {
        mUi.Write("Device has been reset", false);
    }

    mConfigInitialised = false;
    mConfigUpdated = false;

    if (aMode == ResetMode::WAIT && mDisableApp)
    {
        DisableApp();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::DisableApp()
{
    if (teAppDisable(mTeHandle, 0) != TE_OK)
    {
        throw CDutException("teAppDisable failed");
    }
    else
    {
        // After the device responds to the command to disable the application,
        // it seems it hasn't quite finished, which can cause errors when
        // attempting to use device APIs, e.g. ACCMDs (ref: B-307137).
        this_thread::sleep_for(seconds(1));

        mUi.Write("Device application disabled", false);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PreTestActions()
{
    if (mDisableApp)
    {
        DisableApp();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PostTestActions(bool aTestsPassed)
{
    if (aTestsPassed)
    {
        ConfigCommitValues();
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::SetAddress(const std::string& aBtAddress)
{
    if (!aBtAddress.empty())
    {
        mDeviceAddress = CPtBdAddrMgr::CheckBdAddr(aBtAddress);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::RadioTxCwStart(uint16 aChannel, uint16 aFreqMhz)
{
    if (mUsesQHciRadioApis)
    {
        const uint8 POW_LVL = 6; // Default max power

        // Stop first to ensure the device is ready to start CW
        if (teRadStop(mTeHandle) != TE_OK)
        {
            throw CDutException("teRadStop failed");
        }

        if (teRadTxCwStart(mTeHandle, static_cast<uint8>(aChannel), POW_LVL) != TE_OK)
        {
            throw CDutException("teRadTxCwStart failed");
        }
    }
    else
    {
        const uint16 INTPA = 50;
        const uint16 EXTPA = 255;
        const uint16 MODULATION = 0;

        if (radiotestTxstart(mTeHandle, aFreqMhz, INTPA, EXTPA, MODULATION) != TE_OK)
        {
            throw CDutException("radiotestTxstart failed");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::RadioStop()
{
    if (mUsesQHciRadioApis)
    {
        if (teRadStop(mTeHandle) != TE_OK)
        {
            throw CDutException("teRadStop failed");
        }
    }
    else
    {
        if (radiotestPause(mTeHandle) != TE_OK)
        {
            throw CDutException("radiotestPause failed");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::XtalSetLoadCap(uint16 aLoadCapVal)
{
    if (teMcSetXtalLoadCapacitance(mTeHandle, aLoadCapVal) != TE_OK)
    {
        throw CDutException("teMcSetXtalLoadCapacitance failed");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::XtalSetTrim(int16 aTrimVal)
{
    int16 mibValUnused;
    if (teMcSetXtalFreqTrim(mTeHandle, static_cast<uint16>(aTrimVal),
        &mibValUnused) != TE_OK)
    {
        throw CDutException("teMcSetXtalFreqTrim failed");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::ConfigMerge(const std::string& aMergeFile)
{
    ConfigInit();

    if (teConfigCacheMerge(mTeHandle, aMergeFile.c_str()) != TE_OK)
    {
        ostringstream msg;
        msg << "teConfigCacheMerge failed for file \"" << aMergeFile
            << "\". Check that the file contents are valid for the database and system version label in use";
        throw CDutException(msg.str());
    }

    mConfigUpdated = true;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::ConfigSetValue(const std::string& aSubSys,
                                const std::string& aKeyName,
                                const std::string& aValue)
{
    ConfigInit();

    ostringstream keyStr;
    keyStr << aSubSys << mCfgWriteLayer << ':' << aKeyName;
    if (teConfigCacheWriteItem(mTeHandle, keyStr.str().c_str(), aValue.c_str()) != TE_OK)
    {
        ostringstream msg;
        msg << "teConfigCacheWriteItem failed for key \"" << keyStr.str()
            << "\" and value \"" << aValue
            << "\". Check that the key is valid for the database and system version label in use";
        throw CDutException(msg.str());
    }

    mConfigUpdated = true;
}

////////////////////////////////////////////////////////////////////////////////

std::string CCdaDevice::ConfigGetValue(const std::string& aSubSys,
                                       const std::string& aKeyName)
{
    static const size_t DEFAULT_MAX_VAL_LEN = 128;

    ConfigInit();

    ostringstream keyStr;
    keyStr << aSubSys << ':' << aKeyName;
    char* pReadBuff = new char[DEFAULT_MAX_VAL_LEN];
    uint32 maxLen = DEFAULT_MAX_VAL_LEN;
    int32 teRetVal = teConfigCacheReadItem(mTeHandle, keyStr.str().c_str(), pReadBuff, &maxLen);
    if (teRetVal == TE_ERROR && maxLen > DEFAULT_MAX_VAL_LEN)
    {
        delete[] pReadBuff;
        pReadBuff = new char[maxLen];
        teRetVal = teConfigCacheReadItem(mTeHandle, keyStr.str().c_str(), pReadBuff, &maxLen);
    }

    if (teRetVal != TE_OK)
    {
        delete[] pReadBuff;

        ostringstream msg;
        msg << "teConfigCacheReadItem failed for key \"" << keyStr.str()
            << "\". Check that the key is valid for the database and system version label in use";
        throw CDutException(msg.str());
    }

    string value(pReadBuff);
    delete[] pReadBuff;

    return value;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PsSetValue(uint16 aKeyId, uint16 aValueLen,
                            const uint16 apValue[])
{
    if (tePsWrite(mTeHandle, aKeyId, aValueLen, apValue) != TE_OK)
    {
        ostringstream msg;
        msg << "tePsWrite failed for key ID 0x"
            << hex << setfill('0') << setw(4) << aKeyId;
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PsAudioSetValue(uint32 aKeyId, uint16 aValueLen,
                                 const uint16 apValue[])
{
    if (tePsAudioWrite(mTeHandle, aKeyId, aValueLen, apValue) != TE_OK)
    {
        // Note - audio key IDs are 24bit
        ostringstream msg;
        msg << "tePsAudioWrite failed for key ID 0x"
            << hex << setfill('0') << setw(6) << aKeyId;
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::ConfigCommitValues()
{
    if (mConfigInitialised && mConfigUpdated)
    {
        mUi.Write("Committing configuration values to device", false);

        uint16 unused = 0;
        if (teConfigCacheWrite(mTeHandle, NULL, unused) != TE_OK)
        {
            throw CDutException("teConfigCacheWrite failed");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PioMap(uint16 aBank, uint32 aMask, uint32 aPios)
{
    uint32 errLines = 0;
    if (tePioMap(mTeHandle, aBank, aMask, aPios, &errLines) != TE_OK)
    {
        ostringstream msg;
        msg << "tePioMap failed";
        
        if (errLines != 0)
        {
            msg << ". Lines that could not be mapped as PIOs = "
                << "0x" << hex << setfill('0') << setw(8) << errLines;
        }
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::PioSet(uint16 aBank, uint32 aMask, uint32 aDirection,
                        uint32 aValue)
{
    uint32 errLines = 0;
    if (tePioSet(mTeHandle, aBank, aMask, aDirection, aValue, &errLines) != TE_OK)
    {
        ostringstream msg;
        msg << "tePioSet failed";

        if (errLines != 0)
        {
            msg << ". Lines that could not be set = "
                << "0x" << hex << setfill('0') << setw(8) << errLines;
        }
        throw CDutException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

uint32 CCdaDevice::PioGet(uint16 aBank, uint32& aDirection)
{
    uint32 value;
    if (tePioGet(mTeHandle, aBank, &aDirection, &value) != TE_OK)
    {
        throw CDutException("tePioGet failed");
    }

    return value;
}

////////////////////////////////////////////////////////////////////////////////

bool CCdaDevice::IsCharging()
{
    typedef map<uint16, string> ChargingStatesMap;
    static const ChargingStatesMap chgrStatusMap = {
            {static_cast<uint16>(1), "TRICKLE_CHARGE"},
            {static_cast<uint16>(2), "PRE_CHARGE"},
            {static_cast<uint16>(3), "FAST_CHARGE"},
            {static_cast<uint16>(6), "STANDBY"},
    };

    uint16 status;
    if (teChargerGetStatus(mTeHandle, &status) != TE_OK)
    {
        throw CDutException("teChargerGetStatus failed");
    }

    ChargingStatesMap::const_iterator iState = chgrStatusMap.find(status);
    bool charging = (iState != chgrStatusMap.end());

    if (charging)
    {
        ostringstream msg;
        msg << "Charger is charging, status = " << iState->second;
        mUi.Write(msg.str(), false);
    }
    else
    {
        ostringstream msg;
        msg << "Charger is not charging, status = " << status;
        mUi.Write(msg.str(), false);
    }

    return charging;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AudioToneStart(AudioDevice aDevice, uint16 aIface, AudioChannel aChannel)
{
    const uint16 RINGTONE_GENERATOR_CAPABILITY = 0x37;
    const uint16 STREAM_CODEC_OUTPUT_RATE_KEYID = 0x301;
    const uint16 STREAM_CODEC_OUTPUT_RATE = 8000;
    const uint16 STREAM_CODEC_OUTPUT_GAIN_KEYID = 0x303;
    const uint16 STREAM_CODEC_OUTPUT_GAIN = 10;

    // Message corresponds to:
    // RINGTONE_DECAY(0),
    // RINGTONE_TIMBRE(sine),
    // RINGTONE_TEMPO(0xFF),
    // RINGTONE_NOTE(C5, SEMIBREVE),
    // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
    // ...x52...
    // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
    // RINGTONE_END
    //
    // It gives a tone approx. 52 seconds in duration.
    const uint16 RINGTONE_MESSAGE[] = {
        0x1, 0xC000, 0xB000, 0x90FF, 0x1E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x5E01, 0x5E01, 0x5E01, 0x5E01,
        0x8000 };

    // Need to cache the stream information for when we're asked to stop.
    AudioStreamData streamData;

    if (teAudioGetSink(mTeHandle, static_cast<uint16>(aDevice), aIface,
        static_cast<uint16>(aChannel), &streamData.sinkId) != TE_OK)
    {
        throw CDutException("teAudioGetSink failed");
    }

    if (teAudioConfigure(mTeHandle, streamData.sinkId, STREAM_CODEC_OUTPUT_RATE_KEYID,
                         STREAM_CODEC_OUTPUT_RATE) != TE_OK)
    {
        throw CDutException("teAudioConfigure failed for output rate");
    }

    if (teAudioConfigure(mTeHandle, streamData.sinkId, STREAM_CODEC_OUTPUT_GAIN_KEYID,
                         STREAM_CODEC_OUTPUT_GAIN) != TE_OK)
    {
        throw CDutException("teAudioConfigure failed for output gain");
    }

    if (teAudioCreateOperator(mTeHandle, RINGTONE_GENERATOR_CAPABILITY,
                              &streamData.operatorId) != TE_OK)
    {
        throw CDutException("teAudioCreateOperator failed");
    }

    // Message the operator with ring tone elements
    if (teAudioOperatorMessage(mTeHandle, streamData.operatorId, RINGTONE_MESSAGE,
                               (sizeof(RINGTONE_MESSAGE) / sizeof(uint16))) != TE_OK)
    {
        throw CDutException("teAudioOperatorMessage failed");
    }

    // Connect the source to the sink
    // Need to get the source ID from the operator ID:
    //     sourceId = opId + operator_connection_number + 0x2000
    // The ring tone generator capability only has one output connection,
    // so operator_connection_number = 0.
    const uint16 sourceId = streamData.operatorId + 0 + 0x2000;
    if (teAudioConnect(mTeHandle, sourceId, streamData.sinkId,
                       &streamData.transformId) != TE_OK)
    {
        throw CDutException("teAudioConnect failed");
    }

    // Start the operator (starts tone)
    if (teAudioStartOperators(mTeHandle, &streamData.operatorId, 1) != TE_OK)
    {
        throw CDutException("teAudioStartOperators failed");
    }

    mAudioStreams.push_back(streamData);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AudioToneStop()
{
    if (!mAudioStreams.empty())
    {
        // Only one audio stream (index 0) is used for audio tone.

        // Stop the operator (stops tone)
        if (teAudioStopOperators(mTeHandle, &mAudioStreams.at(0).operatorId, 1)
            != TE_OK)
        {
            throw CDutException("teAudioStopOperators failed");
        }

        if (teAudioDisconnect(mTeHandle, mAudioStreams.at(0).transformId)
            != TE_OK)
        {
            throw CDutException("teAudioDisconnect failed");
        }

        if (teAudioDestroyOperators(mTeHandle, &mAudioStreams.at(0).operatorId, 1)
            != TE_OK)
        {
            throw CDutException("teAudioDestroyOperators failed");
        }

        if (teAudioCloseSink(mTeHandle, mAudioStreams.at(0).sinkId) != TE_OK)
        {
            throw CDutException("teAudioCloseSink failed");
        }

        mAudioStreams.erase(mAudioStreams.begin(), mAudioStreams.begin() + 1);
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AudioConfigMicBias(uint16 aId, uint16 aKey, uint32 aValue)
{
    if (teAudioMicBiasConfigure(mTeHandle, aId, aKey, aValue) != TE_OK)
    {
        throw CDutException("teAudioMicBiasConfigure failed");
    }
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AudioLoopbackStart(AudioDevice aSourceDevice, uint16 aSourceIface,
                                    AudioChannel aSourceChannel, AudioDevice aSinkDevice,
                                    uint16 aSinkIface, AudioChannel aSinkChannel)
{
    if ((aSourceChannel == AudioChannel::LEFT_AND_RIGHT && aSinkChannel != AudioChannel::LEFT_AND_RIGHT) ||
        (aSourceChannel != AudioChannel::LEFT_AND_RIGHT && aSinkChannel == AudioChannel::LEFT_AND_RIGHT))
    {
        throw CDutException("Unsupported channel combination. If either the source or sink device is using left AND right channels, then both must.");
    }

    // Need to cache the stream data for when we're asked to stop.
    AudioStreamData streamData1;
    AudioChannel sourceChan1 = (aSourceChannel == AudioChannel::LEFT_AND_RIGHT ? AudioChannel::LEFT : aSourceChannel);
    AudioChannel sinkChan1 = (aSinkChannel == AudioChannel::LEFT_AND_RIGHT ? AudioChannel::LEFT : aSinkChannel);

    if (teAudioGetSource(mTeHandle, static_cast<uint16>(aSourceDevice), aSourceIface,
        static_cast<uint16>(sourceChan1), &streamData1.sourceId) != TE_OK)
    {
        throw CDutException("teAudioGetSource failed");
    }

    if (teAudioGetSink(mTeHandle, static_cast<uint16>(aSinkDevice), aSinkIface,
        static_cast<uint16>(sinkChan1), &streamData1.sinkId) != TE_OK)
    {
        throw CDutException("teAudioGetSink failed");
    }

    if (aSourceChannel == AudioChannel::LEFT_AND_RIGHT)
    {
        AudioStreamData streamData2;
        if (teAudioGetSource(mTeHandle, static_cast<uint16>(aSourceDevice), aSourceIface,
            static_cast<uint16>(AudioChannel::RIGHT), &streamData2.sourceId) != TE_OK)
        {
            throw CDutException("teAudioGetSource failed");
        }

        if (teAudioGetSink(mTeHandle, static_cast<uint16>(aSinkDevice), aSinkIface,
            static_cast<uint16>(AudioChannel::RIGHT), &streamData2.sinkId) != TE_OK)
        {
            throw CDutException("teAudioGetSink failed");
        }

        if (teAudioConnect(mTeHandle, streamData2.sourceId, streamData2.sinkId,
            &streamData2.transformId) != TE_OK)
        {
            throw CDutException("teAudioConnect failed");
        }

        mAudioStreams.push_back(streamData2);
    }

    if (teAudioConnect(mTeHandle, streamData1.sourceId, streamData1.sinkId,
        &streamData1.transformId) != TE_OK)
    {
        throw CDutException("teAudioConnect failed");
    }

    mAudioStreams.push_back(streamData1);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AudioLoopbackStop()
{
    for (vector<AudioStreamData>::const_iterator iStrData = mAudioStreams.begin();
         iStrData != mAudioStreams.end();
         ++iStrData)
    {
        if (teAudioDisconnect(mTeHandle, iStrData->transformId) != TE_OK)
        {
            throw CDutException("teAudioDisconnect failed");
        }

        if (teAudioCloseSource(mTeHandle, iStrData->sourceId) != TE_OK)
        {
            throw CDutException("teAudioCloseSource failed");
        }

        if (teAudioCloseSink(mTeHandle, iStrData->sinkId) != TE_OK)
        {
            throw CDutException("teAudioCloseSink failed");
        }
    }

    mAudioStreams.clear();
}

////////////////////////////////////////////////////////////////////////////////

vector<uint8> CCdaDevice::I2CTransfer(uint16 aPioScl, uint16 aPioSda,
    uint16 aClockKhz, uint16 aDevAddr, vector<uint8> aTxOctets, uint16 aRxOctets)
{
    static const uint16 MAX_OCTETS = 32;

    if (aTxOctets.empty() && aRxOctets == 0)
    {
        throw CDutException("Invalid I2C operation, no data to write or read");
    }
    else if (aTxOctets.size() > MAX_OCTETS || aRxOctets > MAX_OCTETS)
    {
        ostringstream msg;
        msg << "Invalid I2C operation, number of octets to write or read exceeds maximum ("
            << MAX_OCTETS << ")";
        throw CDutException(msg.str());
    }

    const size_t bufSize = (aTxOctets.size() > aRxOctets ? aTxOctets.size() : aRxOctets);
    uint8* pData = new uint8[bufSize];
    (void)copy(aTxOctets.begin(), aTxOctets.end(), pData);
    uint16 rxdOctets = 0;
    if (teI2cTransfer(mTeHandle, aPioScl, aPioSda, aDevAddr, aClockKhz,
        static_cast<uint16>(aTxOctets.size()), aRxOctets, pData, &rxdOctets) != TE_OK)
    {
        delete[] pData;
        throw CDutException("teI2cTransfer failed");
    }

    vector<uint8> readData(pData, pData + rxdOctets);

    delete[] pData;

    return readData;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::AppWrite(uint8 aChannel, const std::vector<uint16>& aData)
{
    if (aChannel > HOST_COMMS_CHANNEL_MAX)
    {
        ostringstream msg;
        msg << "Invalid app (host comms) write operation, channel (" << aChannel
            << ") exceeds maximum (" << HOST_COMMS_CHANNEL_MAX << ")";
        throw CDutException(msg.str());
    }
    else if (aData.size() < HOST_COMMS_WRITE_MSG_LEN_MIN || 
             aData.size() > HOST_COMMS_WRITE_MSG_LEN_MAX)
    {
        ostringstream msg;
        msg << "Invalid app (host comms) write operation, data length (" 
            << aData.size() << ") is outside of the allowed range ("
            << HOST_COMMS_WRITE_MSG_LEN_MIN << " - "
            << HOST_COMMS_WRITE_MSG_LEN_MAX << ")";
        throw CDutException(msg.str());
    }

    if (teAppWrite(mTeHandle, aChannel, &aData[0], static_cast<uint16>(aData.size())) != TE_OK)
    {
        throw CDutException("teAppWrite failed");
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string CCdaDevice::AutoDetectTransport()
{
    string transport;

    // If 1 and only 1 device is connected, return that transport string
    TransportsMap transports;
    GetAvailableTransports(transports);
    if (transports.size() == 1)
    {
        transport = transports.cbegin()->second;
    }
    else if (transports.size() > 1)
    {
        ostringstream msg;
        msg << "Transports found:" << endl;
        for (CCdaDevice::TransportsMap::const_iterator iTrans = transports.begin();
            iTrans != transports.end();
            ++iTrans)
        {
            msg << "\t" << iTrans->first << ", " << iTrans->second << endl;
        }

        mUi.Write(msg.str(), true);

        throw CDutException("Multiple devices found, dutTransport must be specified");
    }
    else
    {
        throw CDutException("No connected devices found");
    }

    return transport;
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::ConfigInit()
{
    if (!mConfigInitialised)
    {
        if (teConfigCacheInit(mTeHandle, mCfgDbPath.c_str()) != TE_OK)
        {
            ostringstream msg;
            msg << "teConfigCacheInit failed for config database \"" << mCfgDbPath
                << "\". Check that the database file exists and contains the specified system version label";
            throw CDutException(msg.str());
        }
        else
        {
            // Read from the device to initialise the cache
            uint16 unused = 0;
            if (teConfigCacheRead(mTeHandle, NULL, unused) != TE_OK)
            {
                throw CDutException("teConfigCacheRead failed");
            }
            else
            {
                mConfigInitialised = true;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string CCdaDevice::Identify()
{
    const size_t MAX_NAME_LEN = 128;
    char nameBuffer[MAX_NAME_LEN];
    if (teGetChipDisplayName(mTeHandle, MAX_NAME_LEN, nameBuffer) != TE_OK)
    {
        throw CDutException("teGetChipDisplayName failed");
    }

    // Get the chip ID - easier to use it to determine if QCC304x and later.
    uint32 chipId;
    if (teGetChipId(mTeHandle, &chipId) != TE_OK)
    {
        throw CDutException("teGetChipId failed");
    }

    if (((chipId >> 16) & 0x00FF) >= 0x4B) // QCC304x and later
    {
        mUsesQHciRadioApis = true;    
    }

    return string(nameBuffer);
}

////////////////////////////////////////////////////////////////////////////////

void CCdaDevice::BurnFirmware()
{
    // These chararcters are cycled through to show a spinning "wheel" while
    // programming is proceeding. This indicates that there is activity even
    // if the progress percentage isn't moving for some time.
    static const vector<char> ACTIVE_INDICATORS = { '\\', '|', '/', '-' };

    CPtTimer timer("Firmware Burn");

    if (flOpenTrans(mTransport.c_str(), 0, 0) != TFL_OK)
    {
        ostringstream msg;
        msg << "Failed to connect to device using transport \"" << mTransport << "\"";
        throw CDutException(msg.str());
    }

    try
    {
        if (flReadProgramFiles(mFirmwarePath.c_str()) != TFL_OK)
        {
            ostringstream msg;
            msg << "Failed to read firmware image file \"" << mFirmwarePath << "\"";
            throw CDutException(msg.str());
        }

        if (flSetFlashType(TFL_TYPE_SQIF) != TFL_OK)
        {
            throw CDutException("Failed to set NVM type");
        }

        if (flSetSubsysChipSel(4, 0) != TFL_OK)
        {
            throw CDutException("Failed to set subsystem and chip select");
        }

        if (flProgramSpawn() != TFL_OK)
        {
            throw CDutException("Failed to start programming thread");
        }

        int32 progress = 0;
        size_t progLoopCount = 0;
        do
        {
            progress = flGetProgress();

            char activeIndicator = ACTIVE_INDICATORS.at(progLoopCount % ACTIVE_INDICATORS.size());
            ostringstream progMsg;
            progMsg << "Programming " << activeIndicator << " Progress = " << progress << "%";
            mUi.UpdateLine(progMsg.str());

            ++progLoopCount;
            if (progress < 100)
            {
                this_thread::sleep_for(seconds(1));
            }
        } while (progress < 100);

        // End the programming progress line
        mUi.Write("");

        const int32 error = flGetLastError();
        if (error != TFL_OK)
        {
            ostringstream msg;
            msg << "Programming failed with error: " << error;
            throw CDutException(msg.str());
        }
        else
        {
            mUi.Write("Verifying image...", false);
            if (flVerify() != TFL_OK)
            {
                throw CDutException("Failed verification");
            }

            mUi.Write("Successfully programmed device");
        }
    }
    catch (...)
    {
        flClose();

        throw;
    }

    flClose();
}
