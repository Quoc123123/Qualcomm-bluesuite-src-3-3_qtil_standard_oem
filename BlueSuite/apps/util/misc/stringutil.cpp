/**********************************************************************
 *
 *  stringutil.cpp
 *
 *  Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for std::string objects.
 *
 ***********************************************************************/

#include "stringutil.h"
#include "engine/enginefw_interface.h"
#include "common/portability.h"

#include <iomanip>
#include <algorithm>
#include <limits>

#ifndef WINCE
#include <errno.h> // Not present for Windows CE / Mobile
#endif

using namespace std;

// Automatically add any non-class methods to the UTILITY group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_UTILITY

namespace stringutil
{
    void TrimLeadingWhitespace
    (
        string& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

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


    void TrimLeadingWhitespace
    (
        std::wstring& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

        const std::wstring::size_type firstNonWhiteSpace = aStringToTrim.find_first_not_of(W_WHITESPACE_DELIMS);

        // If there is a first non-whitespace character and it's not the first character, then there's
        // leading whitespace so erase it. Otherwise, if the string is all whitespace, just erase it.
        if (firstNonWhiteSpace != std::wstring::npos)
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

    void TrimTrailingWhitespace
    (
        string& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

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

    void TrimTrailingWhitespace
    (
        std::wstring& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

        const std::wstring::size_type lastNonWhiteSpace = aStringToTrim.find_last_not_of(W_WHITESPACE_DELIMS);

        // If there is a last non-whitespace character and it's not the last character, then
        // there's trailing whitespace so erase it. Otherwise, if the string is all whitespace, just erase it.
        if (lastNonWhiteSpace != std::wstring::npos)
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

    void TrimLeadingAndTrailingWhitespace
    (
        string& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

        TrimLeadingWhitespace(aStringToTrim);
        TrimTrailingWhitespace(aStringToTrim);
    }

    void TrimLeadingAndTrailingWhitespace
    (
        std::wstring& aStringToTrim //Line to be trimmed
    )
    {
        FUNCTION_DEBUG_SENTRY;

        TrimLeadingWhitespace(aStringToTrim);
        TrimTrailingWhitespace(aStringToTrim);
    }

    /////////////////////////////////////////////////////////////////////////////

    void WriteHexNumberToStream
    (
        ostringstream& aMsgStream,
        int aNumberToDisplay,
        int aNumberOfDigits,
        bool aAddPrefix
    )
    {
        FUNCTION_DEBUG_SENTRY;
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aNumberToDisplay=0x%08x", aNumberToDisplay);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aNumberOfDigits=%d", aNumberOfDigits);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aAddPrefix=%s", (aAddPrefix ? "true" : "false"));

        // Determine the flags and fill settings prior to any changes
        ios_base::fmtflags priorFormatFlags = aMsgStream.flags();
        char priorFillChar = aMsgStream.fill();

        if (aAddPrefix)
        {
            aMsgStream << "0x";
        }

        aMsgStream << setfill('0') << setw(aNumberOfDigits) << hex <<
            aNumberToDisplay << setfill(priorFillChar) << setiosflags(priorFormatFlags);
    }

    void WriteHexNumberToStream
    (
        wostringstream& aMsgStream,
        int aNumberToDisplay,
        int aNumberOfDigits,
        bool aAddPrefix
    )
    {
        FUNCTION_DEBUG_SENTRY;
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aNumberToDisplay=0x%08x", aNumberToDisplay);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aNumberOfDigits=%d", aNumberOfDigits);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aAddPrefix=%s", (aAddPrefix ? "true" : "false"));

        // Determine the flags and fill settings prior to any changes
        ios_base::fmtflags priorFormatFlags = aMsgStream.flags();
        wchar_t priorFillChar = aMsgStream.fill();

        if (aAddPrefix)
        {
            aMsgStream << L"0x";
        }

        aMsgStream << setfill(L'0') << setw(aNumberOfDigits) << hex <<
            aNumberToDisplay << setfill(priorFillChar) << setiosflags(priorFormatFlags);
    }

    /////////////////////////////////////////////////////////////////////////////

    string ToUpper(const string& aString)
    {
        string ret = aString;
        FUNCTION_DEBUG_SENTRY_RET(string, ret);

        // explicit cast needed to resolve ambiguity
        transform(aString.begin(), aString.end(), ret.begin(), (int(*)(int))toupper);
        return ret;
    }

    std::wstring ToUpper(const std::wstring& aString)
    {
        std::wstring ret = aString;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, ret);

        // explicit cast needed to resolve ambiguity
        transform(aString.begin(), aString.end(), ret.begin(), (wint_t(*)(wint_t))towupper);
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////

    string ToLower(const string& aString)
    {
        string ret = aString;
        FUNCTION_DEBUG_SENTRY_RET(string, ret);

        // explicit cast needed to resolve ambiguity
        transform(aString.begin(), aString.end(), ret.begin(), (int(*)(int))tolower);
        return ret;
    }

    std::wstring ToLower(const std::wstring& aString)
    {
        std::wstring ret = aString;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, ret);

        // explicit cast needed to resolve ambiguity
        transform(aString.begin(), aString.end(), ret.begin(), (wint_t(*)(wint_t))towlower);
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////

    uint32 HexStrToInt(const string& aString)
    {
        uint32 value = 0;
        FUNCTION_DEBUG_SENTRY_RET(uint32, value);

        istringstream iss(aString);
        iss >> hex >> value;

        // if we went out of range (or it is not a valid number etc) return a defined value
        if (iss.fail())
        {
            return 0;
        }

#ifdef linux
        // on 64 bit linux, a uint32 may not be 32 bits...
        if (sizeof(uint32) > 4)
        {   // check for value overflow of 32 bits
            if (value > 0xFFFFFFFF)
            {
                return 0;
            }
        }
#endif
        return value;
    }

    uint32 HexStrToInt(const std::wstring& aString)
    {
        uint32 value = 0;
        FUNCTION_DEBUG_SENTRY_RET(uint32, value);

        wistringstream iss(aString);
        iss >> hex >> value;

        // if we went out of range (or it is not a valid number etc) return a defined value
        if (iss.fail())
        {
            return 0;
        }

#ifdef linux
        // on 64 bit linux, a uint32 may not be 32 bits...
        if (sizeof(uint32) > 4)
        {   // check for value overflow of 32 bits
            if (value > 0xFFFFFFFF)
            {
                return 0;
            }
        }
#endif
        return value;
    }

    /////////////////////////////////////////////////////////////////////////////

    string OctStrToString(const string& aString)
    {
        string result;
        FUNCTION_DEBUG_SENTRY_RET(string, result);

        char numstr[3];
        numstr[2] = 0;
        size_t i = 0;

        while(i < aString.size())
        {
            if(isspace(aString[i]))
            {
                // ignore whitespace.
                ++i;
            }
            else if(i+3 < aString.size()                                                              // we have at least 4 characters
                    && aString[i] == '0' && aString[i+1] == 'x'                                       // beginning with '0x'
                    && isxdigit((unsigned char)aString[i+2]) && isxdigit((unsigned char)aString[i+3]) // then 2 hex digits
                    && (i == 0 || isspace(aString[i-1]))                                              // with a preceding space (or begin string)
                    && (i+4 == aString.size() || isspace(aString[i+4])))                              // and a proceeding space (or end string)
            {
                // <whitespace>0x<hex><hex><whitespace>
                // it's a 0x to signify hex digits coming up.
                // we always interpret as hex digits anyway so just ignore it.
                i += 2;
            }
            else if(i+1 < aString.size() && isxdigit((unsigned char)aString[i]) && isxdigit((unsigned char)aString[i+1]))
            {
                // <hex><hex>
                // normal case. interpret 2 hex digits as a character.
                numstr[0] = aString[i];
                numstr[1] = aString[i+1];
                uint8 value = (uint8)HexStrToInt(numstr);
                result += (char)value;
                i += 2;
            }
            else
            {
                // ... something else. this is not an octet string after all.
                return aString;
            }
        }
        
        return result;
    }

    std::wstring OctStrToString(const std::wstring& aString)
    {
        std::wstring result;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, result);

        wchar_t numstr[3];
        numstr[2] = 0;
        size_t i = 0;

        while(i < aString.size())
        {
            if(iswspace(aString[i]))
            {
                // ignore whitespace.
                ++i;
            }
            else if(i+3 < aString.size()                                    // we have at least 4 characters
                    && aString[i] == L'0' && aString[i+1] == L'x'           // beginning with '0x'
                    && iswxdigit(aString[i+2]) && iswxdigit(aString[i+3])   // then 2 hex digits
                    && (i == 0 || iswspace(aString[i-1]))                   // with a preceding space (or begin string)
                    && (i+4 == aString.size() || iswspace(aString[i+4])))   // and a proceeding space (or end string)
            {
                // <whitespace>0x<hex><hex><whitespace>
                // it's a 0x to signify hex digits coming up.
                // we always interpret as hex digits anyway so just ignore it.
                i += 2;
            }
            else if(i+1 < aString.size() && iswxdigit(aString[i]) && iswxdigit(aString[i+1]))
            {
                // <hex><hex>
                // normal case. interpret 2 hex digits as a character.
                numstr[0] = aString[i];
                numstr[1] = aString[i+1];
                uint8 value = (uint8)HexStrToInt(numstr);
                result += (wchar_t)value;
                i += 2;
            }
            else
            {
                // ... something else. this is not an octet string after all.
                return aString;
            }
        }
        
        return result;
    }

    /////////////////////////////////////////////////////////////////////////////

    string StringToOctStr(const string& aString)
    {
        string octStr;
        FUNCTION_DEBUG_SENTRY_RET(string, octStr);

        char strVal[3];
        strVal[2] = 0;

        // Convert each character to full hex form.
        for (size_t i = 0; i < aString.size(); ++i)
        {
            SNPRINTF(strVal, sizeof(strVal), "%.2x", (uint8)(aString[i]));
            octStr += strVal;
        }

        return octStr;
    }

    std::wstring StringToOctStr(const std::wstring& aString)
    {
        std::wstring octStr;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, octStr);

        wchar_t strVal[3];
        strVal[2] = 0;

        // Convert each character to full hex form.
        for (size_t i = 0; i < aString.size(); ++i)
        {
            swprintf(strVal, sizeof(strVal), L"%.2x", (uint8)(aString[i]));
            octStr += strVal;
        }

        return octStr;
    }

    /////////////////////////////////////////////////////////////////////////////

    string OctetToOctetString(const string& aString)
    {
        string octStr;
        FUNCTION_DEBUG_SENTRY_RET(string, octStr);

        stringstream ss;

        for (string::const_iterator it = aString.begin(); it != aString.end(); ++it)
        {
            ss << hex << setfill('0') << setw(2) << (*it & 0xff);
        }

        octStr = ss.str();

        return octStr;
    }

    std::wstring OctetToOctetString(const std::wstring& aString)
    {
        std::wstring octStr;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, octStr);

        wstringstream ss;

        for (std::wstring::const_iterator it = aString.begin(); it != aString.end(); ++it)
        {
            ss << hex << setfill(L'0') << setw(2) << (*it & 0xff);
        }

        octStr = ss.str();

        return octStr;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool StringToInteger(const string& aString, int& aValue)
    {
        bool ret = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, ret);

        if(aString.empty())
        {
            ret = false;
        }
        else
        {
            char *pEnd;
#ifndef WINCE
            // Can check errno, where supported
            errno = 0;
#endif
            long lval = strtol(aString.c_str(), &pEnd, 0);
            aValue = static_cast<int>(lval);
#ifndef WINCE
            if (*pEnd != '\0' || errno != 0)
#else
            if (*pEnd != '\0')
#endif
            {
                ret = false;
            }
            else
            {
                if ( (lval > std::numeric_limits<int>::max()) || (lval < std::numeric_limits<int>::min()) )
                {   // number cannot be held in an int, so we failed to convert
                    ret = false;
                }
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED,"Converted string \"%s\" to integer \"%d\"", aString.c_str(), aValue);
            }
        }

        return ret;
    }

    bool StringToInteger(const std::wstring& aString, int& aValue)
    {
        bool ret = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, ret);

        if (aString.empty())
        {
            ret = false;
        }
        else
        {
            wchar_t *pEnd;
#ifndef WINCE
            // Can check errno, where supported
            errno = 0;
#endif
            long lval = wcstol(aString.c_str(), &pEnd, 0);
            aValue = static_cast<int>(lval);
#ifndef WINCE
            if (*pEnd != L'\0' || errno != 0)
#else
            if (*pEnd != L'\0')
#endif
            {
                ret = false;
            }
            else
            {
                if ((lval > std::numeric_limits<int>::max()) || (lval < std::numeric_limits<int>::min()))
                {   // number cannot be held in an int, so we failed to convert
                    ret = false;
                }
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "Converted string \"%ls\" to integer \"%d\"", aString.c_str(), aValue);
            }
        }

        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////

    string PadString(const string& aSource, char aPad, size_t aSize, PadAlignEnum aAlign)
    {
        string result = aSource;
        FUNCTION_DEBUG_SENTRY_RET(string, result);

        if (aSource.size() < aSize && aPad != '\0')
        {
            for(size_t i=0; i<(aSize-aSource.size()); ++i)
            {
                if (aAlign == PAD_ALIGN_LEFT)
                {
                    result += aPad;
                }
                else
                {
                    result = aPad + result;
                }
            }
        }

        return result;
    }

    std::wstring PadString(const std::wstring& aSource, wchar_t aPad, size_t aSize, PadAlignEnum aAlign)
    {
        std::wstring result = aSource;
        FUNCTION_DEBUG_SENTRY_RET(std::wstring, result);

        if (aSource.size() < aSize && aPad != L'\0')
        {
            for (size_t i = 0; i<(aSize - aSource.size()); ++i)
            {
                if (aAlign == PAD_ALIGN_LEFT)
                {
                    result += aPad;
                }
                else
                {
                    result = aPad + result;
                }
            }
        }

        return result;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool StringToMap(const string& aString, char aAssign, char aSep, map<std::string, std::string>& aMap)
    {
        bool rv = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, rv);

        stringstream ss(aString);
        string item;
        while (rv && getline(ss, item, aSep))
        {
            if (item.empty())
            {
                continue;
            }

            if (aAssign == 0)
            {
                aMap[item] = "";
            }
            else
            {
                rv = (item.find(aAssign) != string::npos);
                if (rv)
                {
                    const string name = item.substr(0, item.find(aAssign));
                    const string value = item.substr(item.find(aAssign)+1);
                    aMap[name] = value;
                }
            }
        }

        return rv;
    }

    bool StringToMap(const std::wstring& aString, wchar_t aAssign, wchar_t aSep, map<std::wstring, std::wstring>& aMap)
    {
        bool rv = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, rv);

        wstringstream ss(aString);
        std::wstring item;
        while (rv && getline(ss, item, aSep))
        {
            if (item.empty())
            {
                continue;
            }

            if (aAssign == 0)
            {
                aMap[item] = L"";
            }
            else
            {
                rv = (item.find(aAssign) != std::wstring::npos);
                if (rv)
                {
                    const std::wstring name = item.substr(0, item.find(aAssign));
                    const std::wstring value = item.substr(item.find(aAssign) + 1);
                    aMap[name] = value;
                }
            }
        }

        return rv;
    }

    /////////////////////////////////////////////////////////////////////////////

    void StringToVector(const string& aString, char aSep, vector<string>& aVec)
    {
        FUNCTION_DEBUG_SENTRY;

        stringstream ss(aString);
        string item;

        while (getline(ss, item, aSep))
        {
            if (item.empty())
            {
                continue;
            }

            aVec.push_back(item);
        }
    }

    void StringToVector(const std::wstring& aString, wchar_t aSep, vector<std::wstring>& aVec)
    {
        FUNCTION_DEBUG_SENTRY;

        wstringstream ss(aString);
        std::wstring item;

        while (getline(ss, item, aSep))
        {
            if (item.empty())
            {
                continue;
            }

            aVec.push_back(item);
        }
    }

    /////////////////////////////////////////////////////////////////////////////

    bool IsValidHexString(const string& aString, bool aAllowPrefix)
    {
        bool retVal = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

        // Empty string isn't a valid hex string
        retVal = !aString.empty();
        if (retVal)
        {
            string::const_iterator it = aString.begin();

            // Optional "0x" prefix - move passed it if present
            if (aAllowPrefix && aString.size() >= 2 && aString.at(0) == '0' && aString.at(1) == 'x')
            {
                it += 2;
            }

            for (; it != aString.end(); ++it)
            {
                if (!isxdigit(*it))
                {
                    retVal = false;
                    break;
                }
            }
        }

        return retVal;
    }

    bool IsValidHexString(const std::wstring& aString, bool aAllowPrefix)
    {
        bool retVal = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

        // Empty string isn't a valid hex string
        retVal = !aString.empty();
        if (retVal)
        {
            std::wstring::const_iterator it = aString.begin();

            // Optional "0x" prefix - move passed it if present
            if (aAllowPrefix && aString.size() >= 2 && aString.at(0) == L'0' && aString.at(1) == L'x')
            {
                it += 2;
            }

            for (; it != aString.end(); ++it)
            {
                if (!iswxdigit(*it))
                {
                    retVal = false;
                    break;
                }
            }
        }

        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////
}
