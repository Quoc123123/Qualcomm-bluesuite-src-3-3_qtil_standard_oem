/**********************************************************************
 *
 *  registry.h
 *
 *  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for windows registry
 *
 ***********************************************************************/

#ifndef REGISTRY_H
#define REGISTRY_H

#include "engine/enginefw_interface.h"
#include <windows.h>

#include <string>

#define REGISTRY_ROOT_KEY "Software\\QTIL"

#define REGISTRY_DEBUG_FILE_KEY   "DebugFile"
#define REGISTRY_DEBUG_LEVEL      "DebugLevel" 

namespace registry
{
    /// The maximum size of a string value that can be returned by GetStringFromRegistry.
    const int MAX_STRING_LEN = 1024;
    
    ///
    /// Retrieve a string value (for a string of maximum length MAX_STRING_LEN)
    /// from the Windows registry.
    /// @param[in]  hKey registry key handle. See Windows documentation of parameter 'hKey'
    /// of API 'RegOpenKeyEx' for further details.
    /// @param[in]  apKeyName The name of the registry key. (May be null.) See Windows
    /// documentation of parameter 'lpSubKey' of API 'RegOpenKeyEx' for further details.
    /// @param[in]  apKeyValueName The name of the registry value. (May be null.) See Windows
    /// documentation of parameter 'lpValueName' of API 'RegQueryValueEx' for further details.
    /// @param[out] aKeyValue The value found for the registry key handle / key name / value
    /// name combination.
    /// @return ERROR_SUCCESS (defined in Windows API) if a string value was successfully
    /// retrieved, a Windows System Error Code otherwise.
    ///
    LONG GetStringFromRegistry(HKEY hKey,  const char *apKeyName, const char *apKeyValueName, std::string &aKeyValue);

    ///
    /// Retrieve a DWORD value from the Windows registry.
    /// @param[in]  hKey registry key handle. See Windows documentation of parameter 'hKey'
    /// of API 'RegOpenKeyEx' for further details.
    /// @param[in]  apKeyName The name of the registry key. (May be null.) See Windows
    /// documentation of parameter 'lpSubKey' of API 'RegOpenKeyEx' for further details.
    /// @param[in]  apKeyValueName The name of the registry value. (May be null.) See Windows
    /// documentation of parameter 'lpValueName' of API 'RegQueryValueEx' for further details.
    /// @param[out] aKeyValue The value found for the registry key handle / key name / value
    /// name combination.
    /// @return ERROR_SUCCESS (defined in Windows API) if a string value was successfully
    /// retrieved, a Windows System Error Code otherwise.
    ///
    LONG GetNumberFromRegistry(HKEY hKey,  const char *apKeyName, const char *apKeyValueName, DWORD &aKeyValue);

    ///
    /// Sets a string value in the Windows registry. The thread calling this function must be
    /// running as administrator for the call to succeed.
    /// @param[in]  hKey registry key handle. See Windows documentation of parameter 'hKey'
    /// of API 'RegCreateKeyEx' for further details.
    /// @param[in]  apKeyName The name of the registry key. (May be null.) See Windows
    /// documentation of parameter 'lpSubKey' of API 'RegCreateKeyEx' for further details.
    /// @param[in]  apKeyValueName The name of the registry value. (May be null.) See Windows
    /// documentation of parameter 'lpValueName' of API 'RegSetValueEx' for further details.
    /// @param[in]  aKeyValue The value to be set for the registry key handle / key name / value
    /// name combination.
    /// @param[out] aKeyAleadyExisted indicates whether or not the key handle / key name combination
    /// already existed prior to calling this function.
    /// @return ERROR_SUCCESS (defined in Windows API) if a string value was successfully
    /// written, a Windows System Error Code otherwise.
    ///
    LONG WriteStringToRegistry(HKEY hKey,  const char *apKeyName, const char *apKeyValueName, const std::string &aKeyValue, bool &aKeyAlreadyExisted);

    ///
    /// Thin wrapper to the corresponding Windows API, to delete a registry key. (The windows
    /// API is no more difficult to use - this is just provided for consistency and convenience.)
    /// @param[in] hKey registry key handle. See Windows documentation of parameter 'hKey'
    /// of API 'RegDeleteKey' for further details.
    /// @param[in] apKeyName The name of the registry key. (May be null.) See Windows
    /// documentation of parameter 'lpSubKey' of API 'RegDeleteKey' for further details.
    /// @return ERROR_SUCCESS (defined in Windows API) if a string value was successfully
    /// deleted, a Windows System Error Code otherwise.
    ///
    LONG DeleteRegistryKey(HKEY hKey, const char *apKeyName);

    ///
    /// Thin wrapper to the corresponding Windows APIs, to delete a value for a registry key.
    /// (The windows APIs are currently more awkward to use but when XP support is dropped
    /// for this code, this will no longer be the case. In any case, it's good to have this
    /// function for consistency and convenience.)
    /// @param[in] hKey registry key handle. See Windows documentation of parameter 'hKey'
    /// of API 'RegDeleteValue' for further details.
    /// @param[in]  apKeyName The name of the registry key. (May be null.) See Windows
    /// documentation of parameter 'lpSubKey' of API 'RegOpenKeyEx' for further details.
    /// @param[in] apKeyValueName The name of the registry value. (May be null.) See Windows
    /// documentation of parameter 'lpValueName' of API 'RegDeleteValue' for further details.
    /// @return ERROR_SUCCESS (defined in Windows API) if a string value was successfully
    /// deleted, a Windows System Error Code otherwise.
    ///
    LONG DeleteRegistryKeyValue(HKEY hKey, const char *apKeyName, const char *apKeyValueName);
}

#endif