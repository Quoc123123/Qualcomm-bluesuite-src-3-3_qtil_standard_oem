/**********************************************************************
 *
 *  keyfile.cpp
 *
 *  Copyright (c) 2010-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Library for parsing and retrieving a 'key' (e.g. transport-unlocking key
 *  from a key file.
 *
 ***********************************************************************/
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include "common/types.h"
#include "spi/spi_common.h"
#include "misc/fileutil.h"
#include "misc/stringutil.h"
#include "keyfile.h"
#include "engine/enginefw_interface.h"

using namespace std;
using namespace stringutil;

const char KEYFILE_COMMENT_CHARACTER = '#';
const uint16  VALID_KEY_LENGTH_128_BIT = GBL_128_BIT_CUST_KEY_NUM_WORDS * 4;
const uint16  VALID_KEY_LENGTH_VUL = GBL_VUL_CUST_KEY_NUM_WORDS * 4;

CKeyFile::CKeyFile(const string  &aFileName)
{
    MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_KEYFILE_LIB);

    mFileName = aFileName; // name (inc path) of key file
    mLineNum = 0;
}

CKeyFile::CVarDevRangeKey::CVarDevRangeKey()
    : mDeviceId(0), mDeviceIdValid(false), mKeyValue(0)
{
}

void CKeyFile::CVarDevRangeKey::SetDeviceId(uint16 aDeviceId)
{
    mDeviceId = aDeviceId;
    mDeviceIdValid = true;
}

bool CKeyFile::CVarDevRangeKey::GetDeviceId(uint16 &aDeviceId)
{
    bool retVal = false;

    if (mDeviceIdValid)
    {
        aDeviceId = mDeviceId;
        retVal = true;
    }

    return retVal;
}

//******************************************************************************************
// This method parses the key file, validating its contents. If the file is found to
// contain a single valid key (which is a string of VALID_KEY_LENGTH_128_BIT or
// VALID_KEY_LENGTH_VUL hexadecimal characters), the method converts this to a vector of
// 4-digit hex numbers. If the file cannot be opened, a suitable error is raised.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum //indicates success / failure of key extraction from key file
CKeyFile::GetKeyFromFile
(
    KeyValue_t &aKey  //OUT: extracted key from key file
)
{
    vector<KeyValue_t> keys;
    KeyfileStatusEnum res = ReadKeysFromFile(keys);
    if (res == KEYFILE_SUCCESS && keys.size() > 1)
    {
        string temp = "Multiple keys found in key file '"
            + mFileName
            + "', but only one was expected";

        MSG_HANDLER.SetErrorMsg(0, temp);
        res = KEYFILE_ERR_INVALID_ENTRY_FOLLOWS_VALID_KEY;
    }
    else if (res == KEYFILE_SUCCESS && !keys.empty())
    {
        aKey = keys[0];
    }
    
    return res;
}

//******************************************************************************************
// This method parses the key file, validating its contents. If the file is found to
// contain one or more valid keys (which are a string of VALID_KEY_LENGTH_128_BIT or
// VALID_KEY_LENGTH_VUL hexadecimal characters), the method converts these to vectors of
// 4-digit hex numbers. If the file cannot be opened, a suitable error is raised.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::GetKeysFromFile(vector<KeyValue_t> &aKeys)
{
    return ReadKeysFromFile(aKeys);
}

//******************************************************************************************
// As per GetKeysFromFile(vector<KeyValue_t> &aKeys), but handles keys which may or may not
// be device-specific. The device id, and key value converted to hex, are stored in a
// CVarDevRangeKey object
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::GetKeysFromFile(vector<CVarDevRangeKey> &aDevRangeKeys)
{
    return ReadKeysFromFile(aDevRangeKeys);
}

//******************************************************************************************
// This method parses the key file, validating its contents. If the file is found to
// contain one or more valid keys (which are a string of VALID_KEY_LENGTH_128_BIT or
// VALID_KEY_LENGTH_VUL hexadecimal characters), the method converts these to vectors of
// 4-digit hex numbers. If the file has invalid format, or cannot be opened, a suitable
// error is raised.
//******************************************************************************************
template<typename T> CKeyFile::KeyfileStatusEnum CKeyFile::ReadKeysFromFile(vector<T> &aKeys)
{
    KeyfileStatusEnum ret = KEYFILE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, ret);

    string fileline;
    ifstream ifile;

    ifile.open(mFileName.c_str(), ifstream::in);

    if (ifile.fail())
    {
        FileInaccessibleError();
        return KEYFILE_ERR_KEYFILE_INACCESSIBLE;
    }

    // Ensure we start with empty key set
    aKeys.clear();

    // Parse the file, line by line
    while ((ret == KEYFILE_SUCCESS) && getline(ifile, fileline))
    {
        mLineNum++;

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Parsing line %d, linelength is %d\n", mLineNum, fileline.length());

        DetermineLineActionEnum action = TrimLineAndDetermineAction(fileline);

        if (action == LINE_PROCESS)
        {
            //If we reached this point, we know the line is not whitespace and
            //isn't a comment. Check if the line is valid.

            // Now attempt to convert the non-trivial line to array format.
            T key;
            ret = ValidateLineAndExtractKey(fileline, key);

            if (ret == KEYFILE_SUCCESS)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Line %d ok\n", mLineNum);
                aKeys.push_back(key);
            }
        }
    }

    //All lines of valid syntax but no key found?
    if ((ret == KEYFILE_SUCCESS) && aKeys.empty())
    {
        NoKeyInKeyfileError();
        ret = KEYFILE_ERR_NO_KEY;
    }

    return ret;
}

//******************************************************************************************
// Writes each member of the aKeyValue to the supplied stream.
//******************************************************************************************
void CKeyFile::WriteKeyToStream(KeyValue_t aKeyValue, ostringstream &aStream)
{
    for (KeyValue_t::iterator y = aKeyValue.begin(); y != aKeyValue.end(); ++y)
    {
        stringutil::WriteHexNumberToStream(aStream, *y, sizeof(*y) * 2, false);
    }
}

//******************************************************************************************
// Writes the key of each member of aKeyValue to the supplied stream, preceded by
// '<device id>=' for device-specific keys
//******************************************************************************************
void CKeyFile::WriteKeyToStream(CVarDevRangeKey aKeyValue, ostringstream &aStream)
{
    uint16 deviceId;

    if (aKeyValue.GetDeviceId(deviceId))
    {
        aStream << deviceId << "=";
    }

    WriteKeyToStream(aKeyValue.mKeyValue, aStream);
}

//******************************************************************************************
// This method writes the key to a file.
// If the file cannot be opened for writing, a suitable error is raised.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::WriteKeyToOutputFile(KeyValue_t &aKey, string aOutputKeyFile)
{
    KeyfileStatusEnum ret = KEYFILE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, ret);

    vector<KeyValue_t> keys;
    keys.push_back(aKey);

    ret = WriteKeysToOutputFileWorker(keys, aOutputKeyFile);

    return ret;
}

//******************************************************************************************
// See template file of same name.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::WriteKeysToOutputFile(vector<KeyValue_t> &aKeys, string aOutputKeyFile)
{
    return WriteKeysToOutputFileWorker(aKeys, aOutputKeyFile);
}

//******************************************************************************************
// See template file of same name. Handles keys which may have a '<device id>=' prefix.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::WriteKeysToOutputFile(vector<CVarDevRangeKey> &aKeys, string aOutputKeyFile)
{
    return WriteKeysToOutputFileWorker(aKeys, aOutputKeyFile);
}

//******************************************************************************************
// This method writes the supplied list of keys to a file.
// If the file cannot be opened for writing, a suitable error is raised.
//******************************************************************************************
template<typename T> CKeyFile::KeyfileStatusEnum CKeyFile::WriteKeysToOutputFileWorker(vector<T> &aKeys, string aOutputKeyFile)
{
    KeyfileStatusEnum ret = KEYFILE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, ret);

#ifndef WINCE
    // Attempt to create the folder containing the key file. (If it already exists, this won't cause a failure.)
    string filePathWithoutName = fileutil::ExtractPath(aOutputKeyFile);

    if (fileutil::IsPathAbsolute(aOutputKeyFile))
    {
        if (!fileutil::CreateDir(filePathWithoutName))
        {
            ret = KEYFILE_ERR_KEYFILE_INACCESSIBLE;
        }
    }
#endif // WINCE

    if (ret == KEYFILE_SUCCESS)
    {
        ofstream ofile;
        ofile.open(aOutputKeyFile.c_str(), ofstream::out);

        if (ofile.fail())
        {
            FileNonWriteableError(aOutputKeyFile);
            ret = KEYFILE_ERR_KEYFILE_INACCESSIBLE;
        }
        else
        {
            // Write each key to the file
            for (typename vector<T>::iterator x = aKeys.begin(); x != aKeys.end(); ++x)
            {
                ostringstream temp; // make sure each iteration starts with a fresh empty stream with no errors in the state

                if (x != aKeys.begin())
                {
                    ofile << "\n";
                }

                WriteKeyToStream(*x, temp);

                ofile << temp.str();
            }

            ofile.close();
        }
    }

    return ret;
}

//******************************************************************************************
// As per KeyStrToHex(), but handles parsing of any '<deviceid>=' prefix.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::KeyStrToDevRangeKey(const string &aKeyStr, CVarDevRangeKey &aDevRangeKey)
{
    KeyfileStatusEnum retVal = KEYFILE_SUCCESS;

    size_t tokenPosition = aKeyStr.find_first_of('=');
    string deviceIdString;
    string key;

    if (tokenPosition != string::npos)
    {
        deviceIdString = aKeyStr.substr(0, tokenPosition);
        const char* const pTempArray = deviceIdString.c_str();

        TrimLeadingAndTrailingWhitespace(deviceIdString);

        if (!deviceIdString.empty())
        {
            char* pLastCharConverted = 0;
            uint16 deviceId = (uint16)strtoul(deviceIdString.c_str(), &pLastCharConverted, 10);

            if (pLastCharConverted == pTempArray + deviceIdString.length())
            {
                aDevRangeKey.SetDeviceId(deviceId);
            }
            else
            {
                retVal = KEYFILE_ERR_BAD_DEVICE_ID;
            }
        }
        else
        {
            retVal = KEYFILE_ERR_BAD_DEVICE_ID;
        }

        if (retVal == KEYFILE_SUCCESS)
        {
            key = aKeyStr.substr(tokenPosition + 1, aKeyStr.size());

            if (retVal == string::npos)
            {
                retVal = KEYFILE_ERR_INVALID_HEX;
            }
        }
    }
    else
    {
        key = aKeyStr;
    }

    if (retVal == KEYFILE_SUCCESS)
    {
        TrimLeadingAndTrailingWhitespace(key);
        retVal = KeyStrToHex(key, aDevRangeKey.mKeyValue);
    }

    return retVal;
}

//******************************************************************************************
// This method processes a string representing a key, 4 characters at a time, and attempts
// to convert each 4-char string to a hex number. A valid key string is converted to
// array format, returned to the user via parameter aKey.
// This method does not set an error string (in order for it to be a static method).
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::KeyStrToHex(const string &aKeyStr, KeyValue_t &aKey)
{
    KeyfileStatusEnum retVal = KEYFILE_SUCCESS;
    MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_KEYFILE_LIB);
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, retVal);

    if ((aKeyStr.size() != VALID_KEY_LENGTH_128_BIT) && (aKeyStr.size() != VALID_KEY_LENGTH_VUL))
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is of invalid length");
        retVal = KEYFILE_ERR_BAD_KEY_LENGTH;
    }

    int keySize = 0;
    if (retVal == KEYFILE_SUCCESS)
    {
        keySize = (aKeyStr.size() == VALID_KEY_LENGTH_128_BIT) ? GBL_128_BIT_CUST_KEY_NUM_WORDS : GBL_VUL_CUST_KEY_NUM_WORDS;
        aKey.clear();
    }

    for (int i = 0; (i < keySize) && (retVal == KEYFILE_SUCCESS); i++)
    {
        const size_t NUM_HEX_DIGITS_IN_UINT16 = 4;
        const string tempString = aKeyStr.substr(i*NUM_HEX_DIGITS_IN_UINT16, NUM_HEX_DIGITS_IN_UINT16);
        const char* const pTempArray = tempString.c_str();

        // The use of strtoul in combination with pLastCharConverted below will detect
        // non-leading whitespace in a string and error, but leading whitespace is
        // just trimmed by strtoul so we have to treat this differently.
        if (isspace((unsigned char)pTempArray[0]))
        {
            retVal = KEYFILE_ERR_INVALID_HEX;
        }
        else
        {
            char* pLastCharConverted = 0;

            //Convert to hex
            aKey.push_back((uint16)strtoul(tempString.c_str(), &pLastCharConverted, 16));

            //Did strtoul successfully convert the whole 4-digit substring to hex?
            if (pLastCharConverted != pTempArray + tempString.length())
            {
                retVal = KEYFILE_ERR_INVALID_HEX;
            }
        }
    }

    if (retVal == KEYFILE_SUCCESS)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is of valid format");
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is of invalid format");
        aKey.clear();
    }

    assert((retVal != KEYFILE_SUCCESS) || aKey.size() == static_cast<size_t>(keySize));

    return retVal;
}

//******************************************************************************************
// This method processes a KeyValue_t into a hex string representing a key.
// A valid key in array format is converted to a textual representation
// returned to the user via parameter aKeyStr.
// This method does not set an error string (in order for it to be a static method).
//******************************************************************************************
CKeyFile::KeyfileStatusEnum CKeyFile::KeyValToHexStr(const KeyValue_t &aKey, string &aKeyStr)
{
    KeyfileStatusEnum retVal = KEYFILE_SUCCESS;
    MSG_HANDLER_ADD_TO_GROUP(CMessageHandler::GROUP_ENUM_KEYFILE_LIB);
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, retVal);

    aKeyStr.clear();
    if (aKey.empty())
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is empty");
        retVal = KEYFILE_ERR_NO_KEY;
    }
    else if ((aKey.size() != GBL_128_BIT_CUST_KEY_NUM_WORDS) && (aKey.size() != GBL_VUL_CUST_KEY_NUM_WORDS))
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is of invalid length");
        retVal = KEYFILE_ERR_BAD_KEY_LENGTH;
    }
    else
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Key is of valid size");

        // build up the string with the hex digits for each element in the key vector
        ostringstream strStream;
        for (KeyValue_t::const_iterator pKeyElement = aKey.begin(); pKeyElement != aKey.end(); ++pKeyElement)
        {
            stringutil::WriteHexNumberToStream(strStream, *pKeyElement, sizeof(*pKeyElement) * 2, false);
        }
        aKeyStr = strStream.str();
    }

    return retVal;
}

//******************************************************************************************
// This method determines whether the passed in key consists of all zeros, or not.
//******************************************************************************************
bool CKeyFile::KeyIsAllZeros(const KeyValue_t &aKey)
{
    bool retVal = true;
    const size_t keyVecSize = aKey.size();

    for (size_t i = 0; i < keyVecSize; i++)
    {
        if (aKey[i] != 0x0000)
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

//******************************************************************************************
// This method:
// a) Trims any leading whitespace from the string
// b) assesses if it is a comment line
// c) If not a comment line, trims any trailing whitespace from it
// Comment lines and whitespace lines are for skipping.
//******************************************************************************************
CKeyFile::DetermineLineActionEnum //shows whether to skip line or continue processing it
CKeyFile::TrimLineAndDetermineAction
(
    string &aFileline //Line to be trimmed and assessed
)
{
    DetermineLineActionEnum action = LINE_PROCESS;
    FUNCTION_DEBUG_SENTRY_RET(DetermineLineActionEnum, action);

    // ignore an empty line
    if (aFileline.length()== 0)
    {
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Skipping empty line");
        action = LINE_SKIP;
    }

    if (action == LINE_PROCESS)
    {
        TrimLeadingWhitespace(aFileline);

        // ignore a whitespace line
        if (aFileline.length()== 0)
        {
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Skipping whitespace line");
            action = LINE_SKIP;
        }

        if (action == LINE_PROCESS)
        {
            // comment line? If so discard
            if (aFileline[0] == KEYFILE_COMMENT_CHARACTER)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Skipping comment line");
                action = LINE_SKIP;
            }

            if (action == LINE_PROCESS)
            {
                TrimTrailingWhitespace(aFileline);
                //Line can't be empty at this point
            }
        }
    }

    return action;
}

//******************************************************************************************
// Thin wrapper to static method KeyStrToHex, but which sets errors specific to this 
// CKeyFile object.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum //indicates success / failure of key extraction from key file
CKeyFile::ValidateLineAndExtractKey
(
    string &aFileline,           //Line to be validated as an unlock code
    vector<uint16> &aUnlockCode  //unlock code extracted if validation successful
)
{
    KeyfileStatusEnum ret = KEYFILE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, ret);

    ret = KeyStrToHex(aFileline, aUnlockCode);

    switch (ret)
    {
    case KEYFILE_SUCCESS:
        break;
    case KEYFILE_ERR_BAD_KEY_LENGTH:
        SetErrorMsgForLine("Key of invalid length");
        break;
    case KEYFILE_ERR_INVALID_HEX:
    default:
        SetErrorMsgForLine("Invalid entry");
        break;
    }

    return ret;
}

//******************************************************************************************
// Thin wrapper to static method KeyStrToDevRangeKey, but which sets errors specific to this 
// CKeyFile object.
//******************************************************************************************
CKeyFile::KeyfileStatusEnum //indicates success / failure of key extraction from key file
CKeyFile::ValidateLineAndExtractKey
(
    string &aFileline,           //Line to be validated as an unlock code
    CVarDevRangeKey &aUnlockCode  //unlock code extracted if validation successful
)
{
    KeyfileStatusEnum ret = KEYFILE_SUCCESS;
    FUNCTION_DEBUG_SENTRY_RET(KeyfileStatusEnum, ret);

    ret = KeyStrToDevRangeKey(aFileline, aUnlockCode);

    switch (ret)
    {
    case KEYFILE_SUCCESS:
        break;
    case KEYFILE_ERR_BAD_KEY_LENGTH:
        SetErrorMsgForLine("Key of invalid length");
        break;
    case KEYFILE_ERR_INVALID_HEX:
    case KEYFILE_ERR_BAD_DEVICE_ID:
    default:
        SetErrorMsgForLine("Invalid entry");
        break;
    }

    return ret;
}

//******************************************************************************************
// Raises error indicating file doesn't exist / cannot be opened
//******************************************************************************************
void CKeyFile::FileInaccessibleError(void)
{
    FUNCTION_DEBUG_SENTRY;

    string temp;

    temp = "Can't open file '" + mFileName + "'";

    MSG_HANDLER.SetErrorMsg(0, temp);
}

//******************************************************************************************
// Raises error indicating file doesn't exist / cannot be opened
//******************************************************************************************
void CKeyFile::FileNonWriteableError(const string &aOutputKeyFile)
{
    FUNCTION_DEBUG_SENTRY;

    string temp;

    temp = "Can't write to file '" + aOutputKeyFile + "'";

    MSG_HANDLER.SetErrorMsg(0, temp);
}

//**************************************************************************************************
// Raises error indicating file is either a) empty or b) all whitespace / comments - contains no key
//**************************************************************************************************
void CKeyFile::NoKeyInKeyfileError(void)
{
    FUNCTION_DEBUG_SENTRY;

    string temp;

    temp = "No key present in key file '" + mFileName + "'. (File is empty, or just whitespace / comments.)";

    MSG_HANDLER.SetErrorMsg(0, temp);
}

//******************************************************************************************
// Method to report an error specific to a line in the key file.
//******************************************************************************************
void CKeyFile::SetErrorMsgForLine(const string &aErrString) // error text to be appended with line number info
{
    FUNCTION_DEBUG_SENTRY;

    ostringstream iss;

    iss << aErrString << " at line " << mLineNum << " of key file '" << mFileName << "'";

    MSG_HANDLER.SetErrorMsg(0, iss.str());
}