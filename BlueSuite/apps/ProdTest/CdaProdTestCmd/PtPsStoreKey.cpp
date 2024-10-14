//**************************************************************************************************
//
//  PtPsStoreKey.cpp
//
//  Copyright (c) 2021-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PS store key class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "PtPsStoreKey.h"
#include "PtException.h"
#include "PtUtil.h"
#include "PtSetup.h"
#include <sstream>
#include <iomanip>
#include <regex>

using namespace QTIL;
using namespace std;


////////////////////////////////////////////////////////////////////////////////

CPtPsStoreKey::CPtPsStoreKey(const std::string& aKey, bool aAllowAudio)
    : mId(0), mAudioKey(false)
{
    ParseKey(aKey, aAllowAudio);
}

////////////////////////////////////////////////////////////////////////////////

void CPtPsStoreKey::ParseKey(const std::string& aKey, bool aAllowAudio)
{
    // Regexes for a key definition match: [<prefix>:]<name|id>=[<value>] where
    // the format of the value is: [val0Lsb val0Msb val1Lsb val1Msb ...].
    // For example: "apps:USR3=[01 00]" or
    //              "audio:0x000080=[01 10 01 00 01 00 07 F8 E4 01]" or
    //              "USR3=[01 00]" (no apps/audio prefix needed if aAllowAudio==true).
    // The value part is optional (no value means delete).
    // Whitespace is optional around the '=' separator and value octets.
    // The id can be 0x prefixed hex, or decimal.
    // Value octets must be hex, 0x prefix optional.
    // There must be an even number of value octets.
    static const string REGEX_PS_KEY_DEF_STR("(\\w+)\\s*=\\s*(?:\\[\\s*(.+)\\s*\\])?");
    static const regex REGEX_PS_KEY_DEF(REGEX_PS_KEY_DEF_STR);
    static const regex REGEXP_PS_APPS_KEY_DEF("apps:" + REGEX_PS_KEY_DEF_STR);
    static const regex REGEXP_PS_AUDIO_KEY_DEF("audio:" + REGEX_PS_KEY_DEF_STR);

    if (aKey.empty())
    {
        throw CPtException("PS key string is empty");
    }
    else
    {
        // Check the type
        cmatch match;
        if (regex_match(aKey.c_str(), match, REGEXP_PS_APPS_KEY_DEF) ||
            regex_match(aKey.c_str(), match, REGEX_PS_KEY_DEF))
        {
            mAudioKey = false;
        }
        else if (aAllowAudio && regex_match(aKey.c_str(), match, REGEXP_PS_AUDIO_KEY_DEF))
        {
            mAudioKey = true;
        }
        else
        {
            ostringstream msg;
            msg << "PS key \"" << aKey << "\" is invalid";
            throw CPtException(msg.str());
        }

        // Convert the PS key name/ID string to a value
        try
        {
            mId = GetPsKeyIdVal(match.str(1), mAudioKey);
        }
        catch (CPtException& ex)
        {
            ostringstream msg;
            msg << "PS key \"" << aKey << "\" is invalid (" << ex.what() << ")";
            throw CPtException(msg.str());
        }

        // Convert the value string into octets
        vector<uint8> psValOctets;
        try
        {
            CPtSetup::ExtractHexOctetsFromString(match.str(2), psValOctets);
        }
        catch (CPtException& ex)
        {
            ostringstream msg;
            msg << "PS key \"" << aKey << "\" value is invalid (" << ex.what() << ")";
            throw CPtException(msg.str());
        }

        // Check the value is in multiples of 2 octets (PS key values are one or more uint16s)
        if (psValOctets.size() % 2)
        {
            ostringstream msg;
            msg << "PS key \"" << aKey << "\" value must contain an even number of octets";
            throw CPtException(msg.str());
        }

        // Convert to words
        for (vector<uint8>::const_iterator iVal = psValOctets.begin();
            iVal != psValOctets.end();
            iVal += 2)
        {
            mValue.push_back(static_cast<uint16>(*iVal) | (*(iVal + 1) << 8));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

uint32 CPtPsStoreKey::GetPsKeyIdVal(const std::string& aKeyId, bool aIsAudioKey) const
{
    uint32 keyId = 0;

    istringstream idIss(aKeyId);
    if (aKeyId.size() >= 2 && aKeyId.compare(0, 2, "0x") == 0)
    {
        idIss >> hex;
    }

    if (aIsAudioKey)
    {
        // Audio key IDs are 24bit, numerical ID only
        if (idIss.str().find('-') != string::npos ||
            !(idIss >> keyId) || !idIss.eof() || keyId > 0xFFFFFF)
        {
            ostringstream msg;
            msg << "Audio PS key ID value \"" << aKeyId
                << "\" must be a value between 0 and 0xFFFFFF (0x prefixed hex, or dec)";
            throw CPtException(msg.str());
        }
    }
    else
    {
        // Key ID could be a name+index string, e.g. "USR3", or an ID value in hex (0x prefixed), or dec.
        if (isalpha(aKeyId.at(0)))
        {
            // Named value (name+index), try and convert
            string::size_type pos = aKeyId.find_first_of("0123456789");
            if (pos == string::npos)
            {
                ostringstream msg;
                msg << "Apps PS key ID name \"" << aKeyId << "\" is missing an index value";
                throw CPtException(msg.str());
            }

            string name(aKeyId.substr(0, pos));
            uint16 index;
            istringstream idxIss(aKeyId.substr(pos));
            if (!(idxIss >> index) || !idxIss.eof())
            {
                ostringstream msg;
                msg << "Apps PS key ID name \"" << aKeyId << "\" index part must be a decimal value";
                throw CPtException(msg.str());
            }

            // Convert to upper case for comparisons
            PtUtil::ToUpper(name);

            // See ps_if.h from the relevant ChipCode package for the key type ranges
            static const uint32 PSKEXTENSION = 0x2000;
            if (name == "USR" && index <= 49)
            {
                keyId = 650 + index;
            }
            else if (name == "USR" && index >= 50 && index <= 99)
            {
                keyId = PSKEXTENSION + 1950 + index - 50;
            }
            else if (name == "DSP" && index <= 49)
            {
                keyId = PSKEXTENSION + 600 + index;
            }
            else if (name == "CONNLIB" && index <= 49)
            {
                keyId = PSKEXTENSION + 1550 + index;
            }
            else if (name == "CUSTOMER" && index <= 89)
            {
                keyId = PSKEXTENSION + 2000 + index;
            }
            else if (name == "CUSTOMER" && index >= 90 && index <= 299)
            {
                keyId = PSKEXTENSION + 2100 + index - 90;
            }
            else
            {
                ostringstream msg;
                msg << "Apps PS key ID name \"" << aKeyId
                    << "\" can't be mapped to an ID in the known ranges";
                throw CPtException(msg.str());
            }
        }
        else
        {
            // Numerical ID given - IDs are 16bit for apps keys.
            if (idIss.str().find('-') != string::npos || 
                !(idIss >> keyId) || !idIss.eof() || keyId > 0xFFFF)
            {
                ostringstream msg;
                msg << "Apps PS key ID value \"" << aKeyId
                    << "\" must be value between 0 and 0xFFFF (0x prefixed hex, or dec)";
                throw CPtException(msg.str());
            }
        }
    }

    return keyId;
}

////////////////////////////////////////////////////////////////////////////////

std::string CPtPsStoreKey::ToString() const
{
    // Audio keys are 24bit, others 16bit
    const size_t keyWidth = (mAudioKey ? 6 : 4);

    ostringstream keyStr;
    keyStr << (mAudioKey ? "audio" : "apps");
    keyStr << ":0x" << hex << uppercase
           << setfill('0') << setw(keyWidth) << mId << '=';

    if (!mValue.empty())
    {
        keyStr << '[';

        for (vector<uint16>::const_iterator iVal = mValue.begin();
            iVal != mValue.end();
            ++iVal)
        {
            // Swap bytes to match input format (and ConfigCmd file format)
            keyStr << hex << setw(2) << uppercase << (*iVal & 0xFF) << ' '
                   << setw(2) << ((*iVal & 0xFF00) >> 8);
            if (iVal < mValue.end() - 1)
            {
                keyStr << ' ';
            }
        }

        keyStr << ']';
    }

    return keyStr.str();
}
