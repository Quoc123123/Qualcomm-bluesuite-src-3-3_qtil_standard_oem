/**********************************************************************
 *
 *  keyfile.h
 *
 *  Copyright (c) 2010-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Keyfile parsing class. For security reasons, no details are provided
 *  via error messages of what constitutes invalid format for a key.
 *
 ***********************************************************************/

#ifndef __KEYFILE_H__
#define __KEYFILE_H__

#include "common/types.h"
#include <vector>
#include <string>

class CKeyFile
{
public:

    /// Something to hold the value of a secure store key
    typedef std::vector<uint16> KeyValue_t;

    // Stores a key, whose range may be restricted to a single device (<device id>=<key> keyfile format) or
    // apply to multiple devices (those without a device-specific key)
    class CVarDevRangeKey
    {
    public:
        CVarDevRangeKey();
        void SetDeviceId(uint16 aDeviceId);
        bool GetDeviceId(uint16 &aDeviceId);
        KeyValue_t mKeyValue;

    private:
        uint16 mDeviceId; // private to enforce consistent setting with mDeviceIdValid
        bool mDeviceIdValid; // private to enforce consistent setting with mDeviceId
    };

    // Return status for CKeyFile methods
    typedef enum
    {
        KEYFILE_SUCCESS,
        KEYFILE_ERR_KEYFILE_INACCESSIBLE,            // Could not open keyfile
        KEYFILE_ERR_INVALID_ENTRY_FOLLOWS_VALID_KEY, // A valid key has been found, but there is a non-whitespace, non-comment line later in keyfile
        KEYFILE_ERR_BAD_KEY_LENGTH,                  // A non-comment, non-whitespace line whose length is not VALID_KEY_LENGTH has been encountered
        KEYFILE_ERR_INVALID_HEX,                     // A non-comment, non-whitespace line containing non-hex characters (other than '=') has been encountered
        KEYFILE_ERR_NO_KEY,                          // File is empty, or all lines in the file are of comment / whitespace - no key is present
        KEYFILE_ERR_BAD_DEVICE_ID                    // For a line of the form <deviceid>=<key>, the <deviceid> is not an integer
    } KeyfileStatusEnum;

    explicit CKeyFile(const std::string  &aFileName);
    KeyfileStatusEnum GetKeyFromFile(KeyValue_t &aKey); // Validates the keyfile syntax and if valid, returns the key (single key expected)
    KeyfileStatusEnum GetKeysFromFile(std::vector<KeyValue_t> &aKeys); // Validates the keyfile syntax and if valid, returns the key(s) (one or more keys expected)
    KeyfileStatusEnum GetKeysFromFile(std::vector<CVarDevRangeKey> &aDevRangeKeys); // As per GetKeysFromFile(), but for keyfile allowing <deviceid>=<key value> format
    KeyfileStatusEnum WriteKeyToOutputFile(KeyValue_t &aKey, std::string aOutputKeyFile); // Writes the key to the supplied output file
    KeyfileStatusEnum WriteKeysToOutputFile(std::vector<KeyValue_t> &aKeys, std::string aOutputKeyFile); // Writes the keys to the supplied output file
    KeyfileStatusEnum WriteKeysToOutputFile(std::vector<CVarDevRangeKey> &aKeys, std::string aOutputKeyFile);  // Writes the keys to the supplied output file
    KeyfileStatusEnum KeyStrToDevRangeKey(const std::string &aKeyStr, CVarDevRangeKey &aDevRangeKey); // Converts hex key string to (device id, if included and) vector format

    static KeyfileStatusEnum KeyStrToHex(const std::string &aKeyStr, KeyValue_t &aKey); // Converts hex key string to vector format
    static KeyfileStatusEnum KeyValToHexStr(const KeyValue_t &aKey, std::string &aKeyStr); // Converts vector format to a hex key string
    static bool KeyIsAllZeros(const KeyValue_t &aKey); // returns true if key is all zeros key, false otherwise

private:

    typedef enum
    {
        CONVERT_SUCCESS,
        CONVERT_FAILURE
    }
    KeyConversionStatusEnum;

    typedef enum
    {
        LINE_PROCESS,
        LINE_SKIP
    }
    DetermineLineActionEnum;

    template<typename T> KeyfileStatusEnum ReadKeysFromFile(std::vector<T> &aKeys);
    void WriteKeyToStream(KeyValue_t aKeyValue, std::ostringstream &aStream);
    void WriteKeyToStream(CVarDevRangeKey aKeyValue, std::ostringstream &aStream);
    DetermineLineActionEnum TrimLineAndDetermineAction(std::string &aFileline);
    KeyfileStatusEnum ValidateLineAndExtractKey(std::string &aFileline, std::vector<uint16> &aUnlockCode);
    KeyfileStatusEnum ValidateLineAndExtractKey(std::string &aFileline, CVarDevRangeKey &aUnlockCode);
    template<typename T> KeyfileStatusEnum WriteKeysToOutputFileWorker(std::vector<T> &aKeys, std::string aOutputKeyFile); // Writes the keys to the supplied output file
    void SetErrorMsgForLine(const std::string &aErrString);
    void FileInaccessibleError(void);
    void NoKeyInKeyfileError(void);
    void FileNonWriteableError(const std::string &aOutputKeyFile);

    std::string mFileName;
    int mLineNum;
};

#endif