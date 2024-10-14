/**********************************************************************
 *
 *  stringutil.h
 *
 *  Copyright (c) 2010-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for std::string objects.
 *
 ***********************************************************************/

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include "common/types.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>

namespace stringutil
{
    typedef enum
    {
        STRING_TO_HEX_SUCCESS,
        STRING_TO_HEX_FAILURE
    }
    StringToHexStatusEnum;

    typedef enum
    {
        PAD_ALIGN_LEFT,
        PAD_ALIGN_RIGHT
    } PadAlignEnum;

    static char const* const WHITESPACE_DELIMS = " \t\f\v\n\r";
    static wchar_t const* const W_WHITESPACE_DELIMS = L" \t\f\v\n\r";

    /**
     *  Trims any leading whitespace from the string (in-situ).
     *  @param[in,out] aStringToTrim The string to be trimmed
     */
    void TrimLeadingWhitespace(std::string& aStringToTrim);
    void TrimLeadingWhitespace(std::wstring& aStringToTrim);

    /**
     *  Trims any trailing whitespace from the string (in-situ).
     *  @param[in,out] aStringToTrim The string to be trimmed
     */
    void TrimTrailingWhitespace(std::string& aStringToTrim);
    void TrimTrailingWhitespace(std::wstring& aStringToTrim);

    /**
     *  Trims any leading and any trailing whitespace from the string (in-situ).
     *  @param[in,out] aStringToTrim The string to be trimmed
     */
    void TrimLeadingAndTrailingWhitespace(std::string& aStringToTrim);
    void TrimLeadingAndTrailingWhitespace(std::wstring& aStringToTrim);

    /**
     *  Write a hex number to the stream with the appropriate formatting.
     *  @param[in,out] aMsgStream The stream to which the number shall be written
     *  @param[in] aNumberToDisplay The number to be displayed
     *  @param[in] aNumberOfDigits The minimum number of digits to be used
     *  @param[in] aAddPrefix if set to true, add "0x" before the number
     */
    void WriteHexNumberToStream
    (
        std::ostringstream& aMsgStream,
        int                 aNumberToDisplay,
        int                 aNumberOfDigits,
        bool                aAddPrefix = true
    );
    void WriteHexNumberToStream
    (
        std::wostringstream& aMsgStream,
        int                 aNumberToDisplay,
        int                 aNumberOfDigits,
        bool                aAddPrefix = true
    );

    /**
     *  Returns upper representation of aString
     *  @param[in] aString The string to be converted
     *  @return the converted string
     */
    std::string ToUpper(const std::string& aString);
    std::wstring ToUpper(const std::wstring& aString);

    /**
     *  Returns lower representation of aString
     *  @param[in] aString The string to be converted
     *  @return the converted string
     */
    std::string ToLower(const std::string& aString);
    std::wstring ToLower(const std::wstring& aString);

    /**
     * Converts an unsigned hex number stored in a string to an integer
     * @param[in] aString the string to be converted
     * @return the converted value
     */
    uint32 HexStrToInt(const std::string& aString);
    uint32 HexStrToInt(const std::wstring& aString);

    /** 
     * Given an octet string converts it to a normal string format, a character per byte.
     * E.g. "74657374" becomes "test".
     * @param[in] aString the string to be converted. Can contain leading and/or trailing whitespace, 
     * spaces between octets, and each octet can be preceded by "0x" (in this case, spaces between 
     * octets are required).
     * @return the text string, or the input string unchanged if the conversion failed.
     */
    std::string OctStrToString(const std::string& aString);
    std::wstring OctStrToString(const std::wstring& aString);

    /** 
     * Given a text string converts it to octet string format, where each character is converted to it's hex value.
     * E.g. "test" becomes "74657374".
     * @param[in] aString the string to be converted.
     * @return the octet string
     */
    std::string StringToOctStr(const std::string& aString);
    std::wstring StringToOctStr(const std::wstring& aString);

    /** 
     * Given a string containing a uint8 array (the octet array)
     * returns a string with the ascii representation of the array.
     * so if aString contains [0x00 0x01 0x02 0x03 0xff] it returns
     * "00010203FF"
     * @param[in] aString the string to be converted.
     * @return the octet string
    */
    std::string OctetToOctetString(const std::string& aString);
    std::wstring OctetToOctetString(const std::wstring& aString);

    /**
     * Convert a string to an integer.
     * @param[in] aString The string to convert
     * @param[out] aValue The integer value of the string
     * @return true if the string contains a valid integer, false otherwise
     */
    bool StringToInteger(const std::string& aString, int& aValue);
    bool StringToInteger(const std::wstring& aString, int& aValue);

    /**
     * Pad a string filling it with the aPad char if 
     * string length is less than aSize, and the pad character is not '\0'. 
     * Otherwise returns the source string itself.
     * @param[in] aSource the string to be padded
     * @param[in] aPad the character to use as padding
     * @param[in] aSize the total desired size of the string
     * @param[in] aAlign the alignment; if PAD_ALIGN_LEFT aSource is padded on the right, on the left otherwise
     * @return the padded string
     */
    std::string PadString(const std::string& aSource, char aPad, size_t aSize, PadAlignEnum aAlign = PAD_ALIGN_LEFT);
    std::wstring PadString(const std::wstring& aSource, wchar_t aPad, size_t aSize, PadAlignEnum aAlign = PAD_ALIGN_LEFT);

    /**
     * Parse a string and return a map with string tags.
     * If the assignment operator is non-zero, the string must be in the form:
     * <name><assign op><value>[<sep>...]
     * Eg. name1=value1;name2=value2
     * If the assignment operator is zero, the string must be in the form:
     * <name>[<sep>...]
     * @param[in] aString The string to split
     * @param[in] aAssign The assignment operator (e.g. '=' or ':'), or zero to indicate splitting the string with the separator
     * @param[in] aSep The separator (e.g. ';')
     * @param[out] aMap The generated map
     * @return true if parse ok, false otherwise
     */
    bool StringToMap(const std::string& aString, char aAssign, char aSep, std::map<std::string, std::string>& aMap);
    bool StringToMap(const std::wstring& aString, wchar_t aAssign, wchar_t aSep, std::map<std::wstring, std::wstring>& aMap);

    /**
     * Parse a string and return a vector of strings.
     * @param[in] aString The string to split
     * @param[in] aSep The separator (e.g. ';')
     * @param[out] aVec The generated vector
     */
    void StringToVector(const std::string& aString, char aSep, std::vector<std::string>& aVec);
    void StringToVector(const std::wstring& aString, wchar_t aSep, std::vector<std::wstring>& aVec);

    /**
    *  Check if the given string is a valid hex string
    *  @param[in] aString The string to check
    *  @param[in] aAllowPrefix true if optional "0x" prefix is allowed, false if not
    *  @return true if aString is a valid hex string, false otherwise
    */
    bool IsValidHexString(const std::string& aString, bool aAllowPrefix);
    bool IsValidHexString(const std::wstring& aString, bool aAllowPrefix);
}

#endif

