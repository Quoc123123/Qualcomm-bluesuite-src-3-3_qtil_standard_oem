//**************************************************************************************************
//
//  PtUtil.cpp
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Utility functions definition, part of an example application for production test.
//
//**************************************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "PtUtil.h"
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

namespace QTIL
{
namespace PtUtil
{
    /// Whitespace characters
    static char const* const WHITESPACE_DELIMS = " \t\f\v\n\r";

    ////////////////////////////////////////////////////////////////////////////////

    vector<string> SplitString(const string& aMultiString,
        const string& aSeparator)
    {
        vector<string> strings;

        if (!aMultiString.empty() && !aSeparator.empty())
        {
            string::size_type start(0);
            string::size_type stop(0);

            while (start != string::npos)
            {
                stop = aMultiString.find(aSeparator, start);

                // Check for and handle quoted strings which could include the
                // separator character (which is not to be treated as a separator
                // in this case).
                char startChar = aMultiString.at(start);
                if (stop != string::npos && 
                    ((startChar == '\"' || startChar == '\'') &&
                    stop > start + 1 && aMultiString.at(stop - 1) != startChar))
                {
                    // Separator is within a quoted string - move to end of it.
                    string::size_type endQuotePos = aMultiString.find(startChar, stop);
                    if (endQuotePos != string::npos)
                    {
                        stop = endQuotePos + 1;
                        if (stop >= aMultiString.size())
                        {
                            stop = string::npos;
                        }
                    }
                }

                if (stop != string::npos)
                {
                    strings.push_back(aMultiString.substr(start, (stop - start)));
                    start = stop + aSeparator.size();
                    if (start >= aMultiString.size())
                    {
                        strings.push_back("");
                        start = string::npos;
                    }
                }
                else
                {
                    strings.push_back(aMultiString.substr(start));
                    start = stop;
                }
            }
        }

        return strings;
    }

    ////////////////////////////////////////////////////////////////////////////////

    void TrimStringStart(string& aStringToTrim)
    {
        const string::size_type firstNonWhiteSpace = aStringToTrim.find_first_not_of(WHITESPACE_DELIMS);

        // If there is a first non-whitespace character and it's not the first character, then there's
        // leading whitespace so erase it. Otherwise, if the string is all whitespace, just erase it.
        if (firstNonWhiteSpace != string::npos)
        {
            if (firstNonWhiteSpace > 0)
            {
                aStringToTrim.erase(0, firstNonWhiteSpace);
            }
        }
        else if (!aStringToTrim.empty())
        {
            aStringToTrim.clear();
        }
    }

    /////////////////////////////////////////////////////////////////////////////

    void TrimStringEnd(string& aStringToTrim)
    {
        const string::size_type lastNonWhiteSpace = aStringToTrim.find_last_not_of(WHITESPACE_DELIMS);

        // If there is a last non-whitespace character and it's not the last character, then
        // there's trailing whitespace so erase it. Otherwise, if the string is all whitespace, just erase it.
        if (lastNonWhiteSpace != string::npos)
        {
            if (lastNonWhiteSpace < aStringToTrim.size() - 1)
            {
                aStringToTrim.erase(lastNonWhiteSpace + 1);
            }
        }
        else if (!aStringToTrim.empty())
        {
            aStringToTrim.clear();
        }
    }

    /////////////////////////////////////////////////////////////////////////////

    void TrimString(string& aStringToTrim)
    {
        TrimStringStart(aStringToTrim);
        TrimStringEnd(aStringToTrim);
    }

    /////////////////////////////////////////////////////////////////////////////

    void ToLower(std::string& aStr)
    {
        transform(aStr.begin(), aStr.end(), aStr.begin(),
            [](char c) {return static_cast<char>(::tolower(c)); });
    }

    /////////////////////////////////////////////////////////////////////////////

    void ToUpper(std::string& aStr)
    {
        transform(aStr.begin(), aStr.end(), aStr.begin(),
            [](char c) {return static_cast<char>(::toupper(c)); });
    }

    /////////////////////////////////////////////////////////////////////////////

    bool FileExists(const std::string& aFile)
    {
        ifstream file(aFile.c_str());
        return file.good();
    }

    /////////////////////////////////////////////////////////////////////////////

    std::string GetLastWinErrorMessage()
    {
        static const size_t MAX_LEN(1024);
        char message[MAX_LEN];
        DWORD lastErrCode = ::GetLastError();

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            lastErrCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            message,
            MAX_LEN,
            NULL);

        // Remove the trailing newline characters
        string messageStr(message);
        size_t pos = messageStr.rfind("\r\n");
        if (pos != string::npos)
        {
            messageStr.erase(pos);
        }

        ostringstream codeStr;
        codeStr << "Code = " << lastErrCode << ": ";
        messageStr.insert(0, codeStr.str());

        return messageStr;
    }
}
}
