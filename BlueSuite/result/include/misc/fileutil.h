/**********************************************************************
 *
 *  fileutil.h
 *
 *  Copyright (c) 2011-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for consistent handling of file operations.
 *
 ***********************************************************************/

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "engine/enginefw_interface.h"
#include "common/types.h"

#include <string>
#include <list>

namespace fileutil
{
    typedef enum RelativeDirectoryBase
    {
        DIR_CURR_WORKING, // The current working directory
        DIR_EXE // The directory of the binary incorporating this library
    } RelativeDirectoryBase;

    ///
    /// Determine if a file exists (and can be read from).
    /// @param[in] aFilename The filename to test.
    /// @return true if the file exists and can be read from, false otherwise.
    ///
    bool FileExists
    (
        const std::string& aFilename
    );

    ///
    /// Determine (by delete-defining the file) if it is possible to write to the
    /// file. Set an error message if the file is not writeable.
    /// @param[in] aFilename The filename to test.
    /// @param[in] aPreserveContents true if the contents are to be preserved,
    /// false otherwise.
    /// @param[in] aTemporaryFile true if the file being tested is a temporary file,
    /// false otherwise.
    /// @param[in] aSupressErrorMsg Until all users have been converted to use
    /// the engine mechanism, allow a secondary/legacy error mechanism.
    /// @return true if it is possible to write to the file, false otherwise.
    ///
    bool IsFileWriteable
    (
        const std::string& aFilename,
        bool               aPreserveContents,
        bool               aTemporaryFile,
        bool               aSupressErrorMsg
    );

#ifndef WINCE
    ///
    /// Create a directory at the location given. No action is performed 
    /// if a file or directory of that name already exists. Note: The parent must
    /// already exist.
    /// @param[in] aDir Absolute path to the directory to create
    /// @return true if successfully created; false otherwise.
    ///
    bool CreateDir
    (        
        const std::string& aDir
    );
#endif

#ifndef WINCE
    ///
    /// Work out the base directory of the filename specified and change the
    /// current working directory to that directory.
    /// If the filename does not contain a path (i.e. it is a single filename),
    /// do nothing.
    /// Windows CE / mobile don't have a current working directory concept, so this 
    /// function is not supported on those platforms.
    /// @param[in] aFilename The filename to parse to determine the directory.
    /// @return true if successful; false otherwise.
    ///
    bool SetCurrentWorkingDir
    (
        const std::string& aFilename
    );
#endif

    ///
    /// Determine the current working directory.
    /// Windows CE / mobile don't have a current working directory concept, so this 
    /// function returns the directory of the running executable on those platforms.
    /// @return The CWD, always with a trailing slash of the appropriate type.
    /// 
    std::string GetCurrentWorkingDir();

    ///
    /// Returns the path of the executable.
    /// @return the executable's path
    ///
    std::string GetExeDir();

    ///
    /// Returns the path of the calling binary module (the library this static library is linked into).
    /// @return the module's path.
    ///
    std::string GetModuleDir();

    ///
    /// Return a copy of the file extension.
    /// @param[in] aFilename The filename to parse.
    /// @return A string containing the extension (including the period character).
    /// or the empty string if there is no extension.
    ///
    std::string GetFileExtension(const std::string& aFilename);

    ///
    /// Work out if the filename supplied is a relative or absolute path.
    /// @param[in] aFilename The filename to parse to determine the directory.
    /// @return true if the path is an absolute path, false for a relative path.
    /// @note The purpose of this method is to determine if any text can be PREFIXED to the
    /// string to manipulate the directory. In the case of "C:abc\def", whilst technically
    /// the path is relative, the return answer will be false (because it may not be prefixed).
    ///
    bool IsPathAbsolute(const std::string& aFilename);

    ///
    /// Extracts the path part from a filename including path.
    /// @param[in] aFilenameWithPath The filename (containing path) to parse.
    /// @return a string representing the path (which may be empty).
    std::string ExtractPath(const std::string &aFilenameWithPath);

    ///
    /// Takes a file or directory path and converts it if necessary to make it absolute.
    /// @param aRelativeBase base directory to use if aPath is relative
    /// @param aPath the path to convert
    /// @param aAbsPath the absolute path
    /// @return true, if aPath is already or has been successfully converted to absolute, false otherwise
    bool ConvertPathToAbsolute(RelativeDirectoryBase aRelativeBase, const std::string& aPath, std::string& aAbsPath);

    ///
    /// Extracts the raw filename from a filename including path.
    /// @param[in]  aFilenameWithPath The filename (containing path) to parse.
    /// @param[out] aFilenameWithoutPath The file component of aFilenameWithPath
    /// @return true if successfully extracted file name, false otherwise.
    bool GetFilenameFromFilepath(const std::string &aFilenameWithPath, std::string &aFilenameWithoutPath);

    ///
    /// Returns the path separator based on linux or windows.
    /// @return the correct path separator
    ///
    std::string PathSeparator();

    ///
    /// @return true if the file has a path, false if it's only the file name
    ///
    bool HasPath(const std::string& aFileName);

    ///
    /// Returns whether the passed path is a dir or not (hence a file).
    /// @param[in] aPath the given path
    /// @param[out] aIsDir true if the path is a dir, false otherwise
    /// @return true if the call succeeded, false otherwise
    ///
    bool IsDir(const std::string& aPath, bool& aIsDir);

    ///
    /// Returns the list of files in the given folder.
    /// @param[in] aDir the directory to read from
    /// @param[in] aWildcard contains a wildcard for the file search (can be empty)
    /// @param[out] the list of files
    /// @return true if the directory exists and zero or more files were found, false otherwise
    ///
    bool GetFiles(const std::string& aDir, const std::string& aWildcard, std::list<std::string>& aFiles);

    ///
    /// Overwrites aMatchedPath with the first file/folder that matches a search
    /// pattern (if one is found). Otherwise, aMatchedPath retains its original value.
    /// @param[in] aSearchPattern The path to find (eg "\myFolder\PartialNa*")
    /// @param[out] aMatchedPath The matching path (eg "\myFolder\PartialName1")
    /// @return true if aMatchedPath has been replaced with a matching filepath, false otherwise
    ///
    bool MatchPath(const std::string &aSearchPattern, std::string& aMatchedPath);

    ///
    /// Compares two files for equality.
    /// This is intended to be used by unit tests (to compare a generated file with a reference file).
    /// The equivalent functionality exists in the Python test infrastructure, but because there
    /// was no unit test equivalent, this method was written. It is not intended to be a fully
    /// functional delta analysis method, rather a SIMPLE "are the file the same" approach. If the
    /// files are different, the user (once alerted to the fact) can use a more refined text comparison
    /// application to determine what the differences were.
    /// @note Only supported for narrow (single byte) files.
    /// @param[in] aPath1 The first file to open
    /// @param[in] aPath2 The second file to open
    /// @param[in] aIgnoreLineEndingVariations true to do a text comparison and ignore UNIX/DOS line
    /// ending differences only
    /// @param[out] aMessage When the return code is false, this string will be populated with a
    /// SIMPLE message of the reason.
    /// @return true if both files were found and equivalent, false otherwise
    ///
    bool CompareFiles(const std::string& aPath1, const std::string& aPath2,
        bool aIgnoreLineEndingVariations, std::string& aMessage);
}

#endif
