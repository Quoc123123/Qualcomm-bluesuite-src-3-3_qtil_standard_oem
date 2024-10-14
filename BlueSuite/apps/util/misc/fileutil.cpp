/**********************************************************************
 *
 *  fileutil.cpp
 *
 *  Copyright (c) 2011-2022 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for consistent handling of file operations.
 *
 ***********************************************************************/

#include "fileutil.h"
#include "stringutil.h"
#include "sysutil.h"
#include "engine/enginefw_interface.h"
#include "common/portability.h"
#include "unicode/ichar.h"

#ifdef WIN32
# include <windows.h>
# ifndef WINCE
#  include <shlobj.h>
#  include <direct.h>
# endif
#else
# include <unistd.h>
# include <sys/stat.h>
# include <glob.h>
# include <dlfcn.h>
#endif

#include <assert.h>
#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

// Automatically add any non-class methods to the UTILITY group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_UTILITY

namespace fileutil
{
    /////////////////////////////////////////////////////////////////////////////

    bool FileExists
    (
        const std::string& aFilename
    )
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilename=>%s<", aFilename.c_str());

        ifstream file(aFilename.c_str());
        retVal = file.good();

        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool IsFileWriteable
    (
        const string& aFilename,
        bool          aPreserveContents,
        bool          aTemporaryFile,
        bool          aSupressErrorMsg
    )
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilename=>%s<", aFilename.c_str());
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aPreserveContents=%d", aPreserveContents);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aTemporaryFile=%d", aTemporaryFile);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aSupressErrorMsg=%d", aSupressErrorMsg);

        // Preserving the contents is not yet supported...
        assert(aPreserveContents == false);

        FILE* pFile = fopen(aFilename.c_str(), "w");
        {
            if (pFile)
            {
                MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "File open succeeded.");
                fclose(pFile);

                retVal = true;
            }
        }

        if (retVal == false && aSupressErrorMsg == false)
        {
            string failureMsg = "Unable to write to the ";
            if (aTemporaryFile)
            {
                failureMsg += "temporary ";
            }
            failureMsg += "file \"" + aFilename + "\".";
            MSG_HANDLER.SetErrorMsg(0, failureMsg);
        }

        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

#ifndef WINCE
    bool CreateDir
    (        
        const string& aDir
    )
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

        // attempt to create directory
        // (return value is ignored as success is checked later)
#ifdef WIN32
        (void)CreateDirectory(aDir.c_str(), NULL);
#else
        (void)mkdir(aDir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP);
#endif

        // check if we actually have a directory
        bool isdir;
        retVal = IsDir(aDir, isdir) && isdir;

        return retVal;
    }
#endif

    /////////////////////////////////////////////////////////////////////////////

#ifndef WINCE
    bool SetCurrentWorkingDir
    (
        const string& aFilename
    )
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilename=>%s<", aFilename.c_str());

        string baseDir = ExtractPath(aFilename);

    #if WIN32
        // Windows... (Non zero is successful)
        BOOL ret = SetCurrentDirectory(baseDir.c_str());
        if (ret != FALSE)
        {
            retVal = true;
        }
    #else
        // Unix... (Zero is successful)
        int ret = chdir(baseDir.c_str());
        if (ret == 0)
        {
            retVal = true;
        }
    #endif

        return retVal;
    }
#endif // WINCE

    /////////////////////////////////////////////////////////////////////////////

    string GetCurrentWorkingDir()
    {
        string workingDir;
        FUNCTION_DEBUG_SENTRY_RET(string, workingDir);

#ifdef WINCE
        // Windows CE and mobile do not have a concept of a current working directory.
        // Effectively, the working directory for an application is its launch 
        // directory.
        ichar directory[MAX_PATH] = II("\0");
        GetModuleFileName(NULL, directory, MAX_PATH);

        // Remove the file name from the path, leave the trailing slash in place
        istring path = directory;
        path.erase(path.find_last_of(II("\\")) + 1);
        workingDir = inarrow(path);
#else
        bool success = false;
 #ifdef WIN32
        size_t pathBufferSize = MAX_PATH; // Initial value
        char* pPathBuffer = new char[pathBufferSize];

        DWORD ret = GetCurrentDirectory(static_cast<DWORD>(pathBufferSize), pPathBuffer);
        if (ret > pathBufferSize)
        {
            // Resize the path buffer and try again
            delete[] pPathBuffer;
            pathBufferSize = ret;
            pPathBuffer = new char[pathBufferSize];

            ret = GetCurrentDirectory(static_cast<DWORD>(pathBufferSize), pPathBuffer);
        }

        success = (ret > 0);
        if (success)
        {
            workingDir = pPathBuffer;
        }
        delete[] pPathBuffer;
 #else
        char currentPath[MAX_PATH] = {0};

        success = GETCWD(currentPath, sizeof(currentPath));
        if (success)
        {
            workingDir = currentPath;
        }
 #endif
        if (success)
        {
            // Add on the trailing slash character if necessary (so the output is consistent)
            const size_t len = workingDir.length();
            size_t slashPos = 0;
            if ((slashPos = workingDir.find_last_of("\\")) != string::npos && slashPos != len - 1)
            {
                workingDir += "\\";
            }
            if ((slashPos = workingDir.find_last_of("/")) != string::npos && slashPos != len - 1)
            {
                workingDir += "/";
            }

            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "The current working directory is >%s<", workingDir.c_str());
        }
#endif

        return workingDir;
    }

    /////////////////////////////////////////////////////////////////////////////

#if defined(WIN32) && !defined(WINCE)
    static bool GetWinKnownFolder
    (
        const KNOWNFOLDERID& aFolderId, ///< The known folder to get the path of, e.g. FOLDERID_LocalAppData.
        std::string& aPath              ///< The path will be returned in this output param.
    )
    {
        bool retVal = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);

        PWSTR pWStrPath = NULL;
        bool res = SUCCEEDED(SHGetKnownFolderPath(aFolderId, KF_FLAG_CREATE, NULL, &pWStrPath));
        if (!res)
        {
            retVal = false;

            // Not necessarily an error, depending in OS, so just log it
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Error getting system path");
        }
        else
        {
            aPath = inarrow(wstring(pWStrPath));
        }

        // Required to free memory allocated by SHGetKnownFolderPath
        CoTaskMemFree(pWStrPath);

        return retVal;
    }
#endif

    /////////////////////////////////////////////////////////////////////////////

    string GetFileExtension
    (
        const string& aFilename
    )
    {
        string extension;
        FUNCTION_DEBUG_SENTRY_RET(string, extension);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilename=>%s<", aFilename.c_str());

        // Work out just the filename (i.e. strip off the path)
        string filenameWithoutPath = aFilename;
        string baseDir = ExtractPath(filenameWithoutPath);
        filenameWithoutPath.erase(0, baseDir.length());

        // Find the last dot, but there must be at least one character before it
        // and one after it to qualify as an extension.
        size_t indexOfLastDot = filenameWithoutPath.rfind(".");
        if (indexOfLastDot == string::npos ||
            indexOfLastDot == 0            ||
            indexOfLastDot == filenameWithoutPath.length() - 1)
        {
            indexOfLastDot = filenameWithoutPath.length();
        }
        extension = filenameWithoutPath.substr(indexOfLastDot);

        return extension;
    }

    /////////////////////////////////////////////////////////////////////////////

    ///
    /// Internal function to locate where the last "\\" or "/" is within a path string.
    /// @param[in] aFilenameWithPath The filename to parse.
    /// @return position if found, otherwise string::npos.
    ///
    string::size_type PositionOfLastPathSeparator(const string& aFilenameWithPath)
    {
        string::size_type positionOfLastSlashChar = string::npos;
        FUNCTION_DEBUG_SENTRY_RET(string::size_type, positionOfLastSlashChar);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilenameWithPath=>%s<", aFilenameWithPath.c_str());

        // Under Cygwin, the OS is Windows but the filenames have forward slashes.
        // It is therefore necessary to cope with slashes of any kind, irrespective of OS.

        // Look for any slash characters (forward or backward)...
        size_t positionOfLastForwardSlashChar = aFilenameWithPath.find_last_of('/');

        if (positionOfLastForwardSlashChar != string::npos)
        {
            positionOfLastSlashChar = positionOfLastForwardSlashChar;
        }
#ifdef WIN32
        size_t positionOfLastBackSlashChar = aFilenameWithPath.find_last_of('\\');

        if (positionOfLastBackSlashChar != string::npos &&
            (positionOfLastBackSlashChar > positionOfLastSlashChar || positionOfLastSlashChar == string::npos))
        {
            positionOfLastSlashChar = positionOfLastBackSlashChar;
        }
#endif

        return positionOfLastSlashChar;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool IsPathAbsolute(const string& aFilename)
    {
        bool absolutePath = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, absolutePath);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilename=>%s<", aFilename.c_str());

        // Under Cygwin, the OS is Windows but the filenames have forward slashes.
        // It is therefore necessary to cope with slashes of any kind, irrespective of OS.

        if (aFilename.length() > 0)
        {
            if (aFilename.at(0) == '\\' || aFilename.at(0) == '/')
            {
                absolutePath = true;
            }

#ifdef WIN32
            if (aFilename.length() > 1 && aFilename.at(1) == ':')
            {
                absolutePath = true;
            }
#endif

#ifdef linux
            if (aFilename.at(0) == '~')
            {
                absolutePath = true;
            }
#endif
        }

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "File is %s", (absolutePath ? "absolute" : "relative"));
        return absolutePath;
    }

    /////////////////////////////////////////////////////////////////////////////

    std::string ExtractPath(const std::string &aFilenameWithPath)
    {
        string pathStr;
        FUNCTION_DEBUG_SENTRY_RET(string, pathStr);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilenameWithPath=>%s<", aFilenameWithPath.c_str());

        string::size_type positionOfLastSlashChar = PositionOfLastPathSeparator(aFilenameWithPath);

        // If there is a slash, then the filename contains a path part
        if (positionOfLastSlashChar != string::npos)
        {
            if (positionOfLastSlashChar + 1 < aFilenameWithPath.length())
            {
                // strip the filename part off the filepath
                pathStr = aFilenameWithPath.substr(0, positionOfLastSlashChar + 1);
            }
            else
            {
                // input was a path without a filename part
                pathStr = aFilenameWithPath;
            }
        }

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Base Directory=>%s<", pathStr.c_str());
        return pathStr;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool ConvertPathToAbsolute(RelativeDirectoryBase aRelativeBase, const std::string& aPath, std::string& aAbsPath)
    {
        bool retVal = true;

        if (fileutil::IsPathAbsolute(aPath))
        {
            aAbsPath = aPath;
        }
        else
        {
            const string baseDir = (aRelativeBase == DIR_CURR_WORKING) ? GetCurrentWorkingDir() : GetModuleDir();
            const string pathStr = baseDir + aPath;

#ifdef WIN32            
            size_t pathBufferSize = MAX_PATH; // Initial value
            char* pPathBuffer = new char[pathBufferSize];

            DWORD ret = GetFullPathName(pathStr.c_str(), static_cast<DWORD>(pathBufferSize), pPathBuffer, NULL);
            if (ret == 0)
            {
                retVal = false;
            }
            else if (ret > pathBufferSize)
            {
                // Resize the path buffer and try again
                delete[] pPathBuffer;
                pathBufferSize = ret;
                pPathBuffer = new char[pathBufferSize];

                retVal = (GetFullPathName(pathStr.c_str(), static_cast<DWORD>(pathBufferSize), pPathBuffer, NULL) > 0);
            }

            if (retVal)
            {
                aAbsPath = pPathBuffer;
            }

            delete[] pPathBuffer;
#else
            string fileWithoutPathStr;
            char* pResolvedPath = realpath(pathStr.c_str(), NULL);
            if (pResolvedPath == NULL)
            {
                // Path likely doesn't exist, try removing the file name and resolving the directory path
                // If that works, we can append the file name to the resolved path. This allows us to
                // get a resolved path to a file that doesn't (yet) exist.
                const string pathWithoutFileStr = fileutil::ExtractPath(pathStr);
                retVal = GetFilenameFromFilepath(pathStr, fileWithoutPathStr);
                if (retVal)
                {
                    pResolvedPath = realpath(pathWithoutFileStr.c_str(), NULL);
                }
            }

            retVal = (pResolvedPath != NULL);
            if (retVal)
            {
                ostringstream absPathStr;
                absPathStr << pResolvedPath;
                if (!fileWithoutPathStr.empty())
                {
                    absPathStr << '/' << fileWithoutPathStr;
                }
                aAbsPath = absPathStr.str();
            }

            free(pResolvedPath);
#endif

        }

        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool GetFilenameFromFilepath(const string &aFilenameWithPath, string &aFilenameWithoutPath)
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aFilenameWithPath=>%s<", aFilenameWithPath.c_str());

        // the input may be a file without a path already
        aFilenameWithoutPath = aFilenameWithPath;

        string::size_type positionOfLastSlashChar = PositionOfLastPathSeparator(aFilenameWithPath);

        // strip off the base directory (if the filename contains a path)
        if (positionOfLastSlashChar != string::npos)
        {
            aFilenameWithoutPath = aFilenameWithPath.substr(positionOfLastSlashChar + 1);
        }

        retVal = !aFilenameWithoutPath.empty();
        
        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

    string GetExeDir()
    {
        string path;
        FUNCTION_DEBUG_SENTRY_RET(string, path);

        ichar* pBuff = NULL;

    #ifdef WIN32
        size_t BUFF_SIZE = 4096;
        pBuff = new ichar[BUFF_SIZE];
        pBuff[0] = '\0';

        GetModuleFileName(NULL, pBuff, static_cast<DWORD>(BUFF_SIZE));
    #else
        pBuff = new ichar[MAX_PATH];
        pBuff[0] = '\0';

        const int numChars = readlink("/proc/self/exe", pBuff, MAX_PATH - 1);
        // Need to null terminate
        if (numChars >= 0)
        {
            pBuff[numChars] = '\0';
        }
    #endif

        path = inarrow(pBuff);
        delete[] pBuff;

        const size_t pos = path.rfind(PathSeparator());
        if (pos != string::npos)
        {
            path = path.substr(0, pos + 1);
        }
        else
        {
            path = "." + PathSeparator();
        }

        return path;
    }

    /////////////////////////////////////////////////////////////////////////////

    std::string GetModuleDir()
    {
        string path;
        FUNCTION_DEBUG_SENTRY_RET(string, path);

        ichar* pBuff = NULL;

    #ifdef WIN32
        HMODULE moduleHandle = NULL;
    # ifdef UNDER_CE
        // Can't use GetModuleHandleEx on CE, so using EngineFrameworkCpp.dll,
        // as all modules require that.
        moduleHandle = GetModuleHandle(II("EngineFrameworkCpp.dll"));
        if (moduleHandle != NULL)
    # else
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                              (LPCTSTR)&GetModuleDir, &moduleHandle))
    # endif
        {
            size_t BUFF_SIZE = 4096;
            pBuff = new ichar[BUFF_SIZE];
            pBuff[0] = '\0';

            GetModuleFileName(moduleHandle, pBuff, static_cast<DWORD>(BUFF_SIZE));
        }
    #else
        pBuff = new ichar[MAX_PATH];
        pBuff[0] = '\0';
        Dl_info dlInfo;
        if (dladdr((void*)GetModuleDir, &dlInfo))
        {
            strcpy(pBuff, dlInfo.dli_fname);
        }
    #endif

        path = inarrow(pBuff);
        delete[] pBuff;

        const size_t pos = path.rfind(PathSeparator());
        if (pos != string::npos)
        {
            path = path.substr(0, pos + 1);
        }
        else
        {
            path = "." + PathSeparator();
        }

        return path;
    }

    /////////////////////////////////////////////////////////////////////////////

    string PathSeparator()
    {
#ifdef WIN32
        return "\\";
#else
        return "/";
#endif
    }

    /////////////////////////////////////////////////////////////////////////////

    bool HasPath(const std::string& aFileName)
    {
        return (aFileName.find(PathSeparator()) != string::npos);
    }

    /////////////////////////////////////////////////////////////////////////////

    bool IsDir(const string& aPath, bool& aIsDir)
    {
        bool retVal = true;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aPath=>%s<", aPath.c_str());

#ifdef WIN32
        // Changes to the following line will need to be reverted when B-132962 is implemented...
#ifdef UNDER_CE
        DWORD attr = INVALID_FILE_ATTRIBUTES;
#else
        DWORD attr = GetFileAttributesA(inarrow(aPath).c_str());
#endif
        retVal = (attr != INVALID_FILE_ATTRIBUTES);   // return value is true if call succeeded, false otherwise
        if (retVal)
        {
            aIsDir = ((attr & FILE_ATTRIBUTE_DIRECTORY) ? true : false);
        }
#else
        struct stat sb;
        retVal = stat(aPath.c_str(), &sb) == 0;
        if (retVal)
        {
            aIsDir = S_ISDIR(sb.st_mode);
        }
#endif

        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_ENHANCED, "aIsDir=%s", (aIsDir ? "true" : "false"));
        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool GetFiles(const std::string& aDir, const std::string& aWildcard, std::list<std::string>& aFiles)
    {
        bool retVal;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aDir=>%s<", aDir.c_str());

        const string separator = (aDir.rfind(fileutil::PathSeparator()) == aDir.size()-1) ? "" : fileutil::PathSeparator();
        
        bool isDirResult;
        retVal = IsDir(aDir, isDirResult);
        const string search = aDir + separator + (aWildcard.empty() ? "*" : aWildcard);

#ifdef WIN32
        WIN32_FIND_DATAW findFileData;
        HANDLE pFind;

#ifdef UNDER_CE
        pFind = INVALID_HANDLE_VALUE;
#else
        pFind = FindFirstFileW(iwiden(search).c_str(), &findFileData);
#endif

        if (pFind != INVALID_HANDLE_VALUE)
        {
            stringstream fname;
            fname << aDir << separator << inarrow(findFileData.cFileName);
            
            bool isDir = false;
            if (fileutil::IsDir(fname.str(), isDir) && !isDir)
            {
                aFiles.push_back(fname.str());
            }

            while (FindNextFileW(pFind, &findFileData))
            {
                fname.str("");
                fname << aDir << separator << inarrow(findFileData.cFileName);
                if (fileutil::IsDir(fname.str(), isDir) && !isDir)
                {
                    // if IsDir succeeds and fname is not a dir then add it
                    aFiles.push_back(fname.str());
                }
            }

            FindClose(pFind);
            retVal = true;
        }
        else
        {
            // In the case of no files found, the handle is INVALID_HANDLE_VALUE
            // and the value of GetLastError is ERROR_FILE_NOT_FOUND.
            // In this situation, the function should succeed (and just not populate aFiles).
            retVal = retVal && (GetLastError() == ERROR_FILE_NOT_FOUND);
        }
#else
        glob_t globbuf;
        int osRetVal = glob(search.c_str(), 0, NULL, &globbuf);
        if (osRetVal == 0 || osRetVal == GLOB_NOMATCH)
        {
            for(size_t i=0;i<globbuf.gl_pathc;++i)
            {
                aFiles.push_back(globbuf.gl_pathv[i]);
            }
        }
        else
        {
            retVal = false;
        }

        globfree(&globbuf);
#endif

        return retVal;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool MatchPath(const std::string &aSearchPattern, std::string& aMatchedPath)
    {
        bool foundMatch = false;
#ifdef WIN32
        // windows method of finding first path that matches a pattern
        WIN32_FIND_DATA findFileData;
        HANDLE pFind = FindFirstFileA(inarrow(aSearchPattern).c_str(), &findFileData);
        if (pFind != INVALID_HANDLE_VALUE)
        {
            aMatchedPath = ExtractPath(aSearchPattern); // get the path part of the search
            aMatchedPath += findFileData.cFileName; // add the found filename part to it
            FindClose(pFind);
            foundMatch = true;
        }
#else
        // linux method of finding first path that matches a pattern
        glob_t globbuf;
        int osRetVal = glob(aSearchPattern.c_str(), 0, NULL, &globbuf);
        if (osRetVal == 0)
        {
            if (globbuf.gl_pathc > 0)
            {
                aMatchedPath = globbuf.gl_pathv[0]; // gets the found path+filename
                foundMatch = true;
            }
        }
        globfree(&globbuf);
#endif
        return foundMatch;
    }

    /////////////////////////////////////////////////////////////////////////////

    bool CompareFiles(const string& aPath1, const string& aPath2,
        bool aIgnoreLineEndingVariations, string& aMessage)
    {
        bool retVal = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, retVal);
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aPath1=>%s<", aPath1.c_str());
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aPath2=>%s<", aPath2.c_str());
        MSG_HANDLER_NOTIFY_DEBUG(DEBUG_PARAMETER, "aIgnoreLineEndingVariations=%d", aIgnoreLineEndingVariations);

        FILE* pFile1 = fopen(aPath1.c_str(), "rb");
        FILE* pFile2 = fopen(aPath2.c_str(), "rb");
        aMessage.clear();

        char buffer[2];
        bool stillReading = true;
        while (stillReading && pFile1 != NULL && pFile2 != NULL)
        {
            size_t readFile1 = fread(&buffer[0], 1, 1, pFile1);
            size_t readFile2 = fread(&buffer[1], 1, 1, pFile2);
            if (readFile1 != readFile2)
            {
                // One file is a different length to the other
                stillReading = false;

                aMessage = "'";
                if (readFile1 < readFile2)
                {
                    aMessage += aPath1;
                    aMessage += "' is a subset of (shorter than) '";
                    aMessage += aPath2;
                }
                else
                {
                    aMessage += aPath2;
                    aMessage += "' is a subset of (shorter than) '";
                    aMessage += aPath1;
                }
                aMessage += "'.";
            }
            else if (readFile1 == 0) // did not get any more data from file
            {
                // Both files have reached the EOF at exactly the same time
                retVal = true;
                stillReading = false;
                break;
            }

            bool bothCharactersAreNewLines = false;
            if (aIgnoreLineEndingVariations)
            {
                bothCharactersAreNewLines = true;
                if (buffer[0] == '\r' || buffer[0] == '\n')
                {
                    fread(&buffer[0], 1, 1, pFile1);
                    if (buffer[0] != '\r' && buffer[0] != '\n')
                    {
                        fseek(pFile1, -1, SEEK_CUR);
                    }
                }
                else
                {
                    bothCharactersAreNewLines = false;
                }

                if (buffer[1] == '\r' || buffer[1] == '\n')
                {
                    fread(&buffer[1], 1, 1, pFile2);
                    if (buffer[1] != '\r' && buffer[1] != '\n')
                    {
                        fseek(pFile2, -1, SEEK_CUR);
                    }
                }
                else
                {
                    bothCharactersAreNewLines = false;
                }
            }

            if (bothCharactersAreNewLines == false && buffer[0] != buffer[1])
            {
                // One file differed from the other
                stillReading = false;

                aMessage  = "'";
                aMessage += aPath1;
                aMessage += "' is different to '";
                aMessage += aPath2;
                aMessage += "'.";
            }

            if (stillReading && feof(pFile1) && feof(pFile2))
            {
                // Both files have reached the EOF at exactly the same time
                retVal = true;
                stillReading = false;
            }
        }

        // Close the first file
        if (pFile1 == NULL)
        {
            aMessage += "'";
            aMessage += aPath1;
            aMessage += "' could not be opened. ";
        }
        else
        {
            fclose(pFile1);
        }

        // Close the second file
        if (pFile2 == NULL)
        {
            aMessage += "'";
            aMessage += aPath2;
            aMessage += "' could not be opened. ";
        }
        else
        {
            fclose(pFile2);
        }

        return retVal;
    }
}
