//**************************************************************************************************
//
//  PtSetup.cpp
//
//  Copyright (c) 2020-2021 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Setup class definition, part of an example application for production test.
//
//**************************************************************************************************

#include "PtSetup.h"
#include "PtUtil.h"
#include "PtException.h"
#include <fstream>
#include <sstream>
#include <regex>

using namespace QTIL;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

CPtSetup::CPtSetup(const std::string& aFilePath)
  : mFilePath(aFilePath)
{
    Load();
}

////////////////////////////////////////////////////////////////////////////////

void CPtSetup::Load()
{
    static const regex REGEXP_CONFIG_LINE("\\s*(\\S+)\\s*=\\s*(.+)?");

    ifstream cfgFile(mFilePath);

    if (cfgFile.good())
    {
        string line;

        while (!cfgFile.eof())
        {
            getline(cfgFile, line);

            if (!line.empty() && line.at(0) != '#') // Skip blank and comment lines
            {
                cmatch match;
                if (regex_match(line.c_str(), match, REGEXP_CONFIG_LINE))
                {
                    // Trim any trailing spaces from the value
                    string value = match.str(2);
                    PtUtil::TrimStringEnd(value);

                    mConfigItems[match.str(1)] = value;
                }
                else
                {
                    ostringstream msg;
                    msg << "Configuration file \"" << mFilePath
                        << "\" contains invalid content \"" << line << "\"";
                    throw CPtException(msg.str());
                }
            }
        }

        if (mConfigItems.empty())
        {
            ostringstream msg;
            msg << "No configuration settings found in \"" << mFilePath << "\"";
            throw CPtException(msg.str());
        }
    }
    else
    {
        ostringstream msg;
        msg << "Failed to load configuration file \"" << mFilePath << "\"";
        throw CPtSetupFileNotFoundException(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CPtSetup::GetValueList(const std::string& aName,
    bool aMandatory) const
{
    string value = GetValue(aName, aMandatory);
    vector<string> values = PtUtil::SplitString(value.c_str(), ",");
    if ((aMandatory || !value.empty()) && values.empty())
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aName << "\" must be set to a comma-separated list containing at least one value";
        throw CPtException(msg.str());
    }

    return values;
}


////////////////////////////////////////////////////////////////////////////////

std::string CPtSetup::GetValue(const std::string& aName, bool aMandatory) const
{
    if (aName.empty())
    {
        throw CPtException("Configuration setting name is empty");
    }

    string value;

    ConfigMap::const_iterator iSetting = mConfigItems.find(aName);
    if (iSetting != mConfigItems.end())
    {
        value = iSetting->second;
    }

    if (aMandatory && value.empty())
    {
        ostringstream msg;
        msg << "Configuration setting \"" << aName << "\" must be set";
        throw CPtException(msg.str());
    }

    return value;
}

////////////////////////////////////////////////////////////////////////////////

void CPtSetup::ExtractHexOctetsFromString(const string& aOctetString,
    vector<uint8>& aOctets)
{
    // Split the octet string
    vector<string> octetStrings = PtUtil::SplitString(aOctetString.c_str(), " ");

    // Remove any empty entries (e.g. from accidental double separators between values)
    auto EmptyString = [](string const& aStr) { return aStr.empty(); };
    octetStrings.erase(remove_if(octetStrings.begin(), octetStrings.end(), EmptyString), octetStrings.end());

    // Convert the value strings into a vector of uint8 values
    for (vector<string>::const_iterator iValStr = octetStrings.begin();
        iValStr != octetStrings.end();
        ++iValStr)
    {
        istringstream iss(*iValStr);
        uint16 val = 0; // Using uint16 to avoid interpretation as char
        if (iss.str().find('-') != string::npos ||
            !(iss >> hex >> val) || !iss.eof() || val > 0xFF)
        {
            ostringstream msg;
            msg << "Value \""
                << *iValStr << "\" is not a valid hex octet";
            throw CPtException(msg.str());
        }

        aOctets.push_back(static_cast<uint8>(val));
    }
}
