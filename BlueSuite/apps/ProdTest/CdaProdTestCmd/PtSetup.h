//**************************************************************************************************
//
//  PtSetup.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Setup class declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_SETUP_H
#define PT_SETUP_H

#include "common\types.h"
#include "PtException.h"
#include <string>
#include <vector>
#include <map>

///
/// Setup file not found exception class.
///
class CPtSetupFileNotFoundException : public CPtException
{
public:
    ///
    /// Constructor.
    /// @param[in] aMessage The exception message.
    ///
    explicit CPtSetupFileNotFoundException(const std::string& aMessage) : CPtException(aMessage) {};
};

///
/// Production test setup class.
///
class CPtSetup
{
public:

    ///
    /// Constructor.
    /// @param[in] aFilePath Setup file path.
    ///
    explicit CPtSetup(const std::string& aFilePath);

    ///
    /// Destructor.
    ///
    ~CPtSetup() {};

    ///
    /// Gets the values for a list setting (comma-separated)
    /// @param[in] aName The name of the setting.
    /// @param[in] aMandatory true if the setting must have a value, false if not.
    /// @return Setting value strings.
    /// @throws CPtException.
    ///
    std::vector<std::string> GetValueList(const std::string& aName, bool aMandatory) const;

    ///
    /// Gets a value for a named setting.
    /// @param[in] aName The setting name.
    /// @param[in] aMandatory true if the setting must have a value, false if not.
    /// @return The value of the setting, empty string if not found.
    /// @throws CPtException.
    ///
    std::string GetValue(const std::string& aName, bool aMandatory) const;

    ///
    /// Get a numerical setting value.
    /// Throws if the setting can't be found or is empty.
    /// @param[in] aString The string to convert.
    /// @return The value read.
    /// @throws CPtException.
    ///
    template <typename T>
    T GetValueNum(const std::string& aSettingName) const
    {
        string value = GetValue(aSettingName, true);

        T retVal;
        std::istringstream iss(value);
        if ((std::numeric_limits<T>::min() == 0 && value.at(0) == '-') ||
            !(iss >> retVal) || !iss.eof())
        {
            std::ostringstream msg;
            msg << "Configuration setting \"" << aSettingName << "\", value \""
                << value << "\" could not be converted";
            throw CPtException(msg.str());
        }

        return retVal;
    };

    ///
    /// Validates and extracts hex octets from an octet string.
    /// @param[in] aOctetString String of hex octets (0x prefix optional) separated by space,
    ///   e.g. "01 1A 0F".
    /// @param[out] aOctets The octet values.
    /// @throws CPtException.
    ///
    static void ExtractHexOctetsFromString(const std::string& aOctetString, std::vector<uint8>& aOctets);

private:

    ///
    /// Loads the setup from a file.
    /// @throws CPtException.
    ///
    void Load();

    ///
    /// Path to setup file.
    ///
    std::string mFilePath;

    ///
    /// ConfigMap type for mapping config item names to values
    ///
    typedef std::map<std::string, std::string> ConfigMap;

    ///
    /// Configuration items map
    ///
    ConfigMap mConfigItems;
};

#endif // PT_SETUP_H
