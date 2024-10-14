//**************************************************************************************************
//
//  PtPsStoreKey.h
//
//  Copyright (c) 2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  PS store key class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_PS_STORE_KEY_H
#define PT_PS_STORE_KEY_H

#include "common\types.h"
#include <string>
#include <vector>


///
/// Production test PS store key class
///
class CPtPsStoreKey
{
public:

    ///
    /// Constructor.
    /// @param[in] aKey PS key setting string in the form:
    ///      [<prefix>:]<name|id>=[val0Lsb val0Msb val1Lsb val1Msb ...].
    ///   For audio keys the format for a setting is: audio:<id>=[val0Lsb val0Msb val1Lsb val1Msb ...].
    ///   For apps keys the format for a setting is: apps:<name|id>=[val0Lsb val0Msb val1Lsb val1Msb ...].
    ///   If aAllowAudio is true, the prefix must be supplied to distinguish between apps and audio keys,
    ///   otherwise it is optional and if specified must be "apps".
    ///   E.g.: "apps:USR3=[01 00]", or "audio:0x000080=[01 10 01 00 01 00 07 F8 E4 01]",
    ///   or: "USR3=[01 00]".
    ///   The id can be specified as 0x prefixed hex, or as decimal.
    ///   The value octets must be hexadecimal (0x prefix optional).
    /// @param[in] aAllowAudio True if audio keys are allowed, false otherwise.
    /// @throws CPtException.
    ///
    CPtPsStoreKey(const std::string& aKey, bool aAllowAudio);

    ///
    /// Destructor.
    ///
    virtual ~CPtPsStoreKey() {};

    ///
    /// Returns the string representation of the key.
    /// @return Key as a string.
    ///
    std::string ToString() const;

    ///
    /// Returns the key ID.
    /// @return Key ID.
    ///
    uint32 Id() const { return mId; };

    ///
    /// Returns whether the key is an audio key or not.
    /// @return true if the key is an audio key, false otherwise (apps key).
    ///
    bool IsAudioKey() const { return mAudioKey; };

    ///
    /// Returns the key value.
    /// @return Key value words.
    ///
    const std::vector<uint16>& Value() const { return mValue; };

private:

    ///
    /// Parses a string of PS store keys, storing them internally as PsStoreKey instances.
    /// @param[in] aKeys PS key settings strings in the form specified for the constructor.
    /// @param[in] aAllowAudio True if audio keys are allowed, false otherwise.
    /// @throws CPtException.
    ///
    void ParseKey(const std::string& aKey, bool aAllowAudio);

    ///
    /// Converts a PS key name/ID string to a key ID value.
    /// If the key ID string contains a name+index string, and the
    /// name and index can't be mapped to one of the known key ranges,
    /// an exception is thrown.
    /// @param[in] aKeyId The key name/ID string to convert.
    /// @param[in] aAudioKey true if the key is an audio PS key.
    /// @return The key ID value.
    /// @throws CPtException.
    /// 
    /// 
    uint32 GetPsKeyIdVal(const std::string& aKeyId, bool aAudioKey) const;

    ///
    /// The PS key ID.
    ///
    uint32 mId;

    ///
    /// Whether the key is an audio key (true), or apps key (false).
    ///
    bool mAudioKey;

    ///
    /// The PS key value words.
    ///
    std::vector<uint16> mValue;
};

#endif // PT_PS_STORE_KEY_H
