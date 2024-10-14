//**************************************************************************************************
//
//  PtUtil.h
//
//  Copyright (c) 2020-2022 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
//  Utility functions declaration, part of an example application for production test.
//
//**************************************************************************************************

#ifndef PT_UTIL_H
#define PT_UTIL_H

#include <string>
#include <vector>

///
/// Utility functions
///
namespace QTIL
{
namespace PtUtil
{
    ///
    /// Split a list of strings into a vector of strings.
    /// If quoted strings (single or double quoted) are present, the separator
    /// string within the matching quotes is not treated as a separator.
    /// @param[in] aMultiString Strings divided by aSeparator.
    /// @param[in] aSeparator The separator string.
    /// @return The strings obtained.
    ///
    std::vector<std::string> SplitString(const std::string& aMultiString,
        const std::string& aSeparator);

    ///
    /// Trims any white space from the start of a string.
    /// @param[in,out] aStringToTrim The string to trim.
    ///
    void TrimStringStart(std::string& aStringToTrim);

    ///
    /// Trims any white space from the end of a string.
    /// @param[in,out] aStringToTrim The string to trim.
    ///
    void TrimStringEnd(std::string& aStringToTrim);

    ///
    /// Trims any white space from the start and end of a string.
    /// @param[in,out] aStringToTrim The string to trim.
    ///
    void TrimString(std::string& aStringToTrim);

    ///
    /// Converts a string to lower case.
    /// @param[in,out] aStr The string to convert.
    ///
    void ToLower(std::string& aStr);

    ///
    /// Converts a string to upper case.
    /// @param[in,out] aStr The string to convert.
    ///
    void ToUpper(std::string& aStr);

    ///
    /// Determines if a given file exists.
    /// @param[in] aFile The file path.
    /// @return true if the file exists (and can be read), false otherwise.
    ///
    bool FileExists(const std::string& aFile);

    ///
    /// Gets the last windows error message.
    /// @return The last error message string.
    ///
    std::string GetLastWinErrorMessage();
}
}

#endif // PT_UTIL_H
