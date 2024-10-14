/**********************************************************************
 *
 *  registry.cpp
 *
 *  Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions for windows registry.
 *
 ***********************************************************************/

#include "registry.h"

// Automatically add any non-class methods to the UTILITY group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_UTILITY

namespace registry
{
    /////////////////////////////////////////////////////////////////////////////

    LONG GetStringFromRegistry(HKEY hKey,  const char *apKeyName, const char *apKeyValueName, std::string &aKeyValue)
    {
        LONG err = ERROR_SUCCESS;
        FUNCTION_DEBUG_SENTRY_RET(LONG, err);

        HKEY key;
        DWORD size = MAX_STRING_LEN;
        char str[MAX_STRING_LEN+1];

        err = RegOpenKeyEx(hKey, apKeyName, 0, KEY_READ, &key);
        if (err == ERROR_SUCCESS)
        {
            err = RegQueryValueEx(key, apKeyValueName, NULL, NULL, (BYTE*)str, &size);
            if(err == ERROR_SUCCESS)
            {
               str[size - 1] = '\0';  // Null terminate
               aKeyValue = str; 
            }
            (void)RegCloseKey(hKey);
        }

        return err;
    }

    /////////////////////////////////////////////////////////////////////////////

    LONG GetNumberFromRegistry(HKEY hKey,  const char *apKeyName, const char *apKeyValueName, DWORD &aKeyValue)
    {
        LONG err = ERROR_SUCCESS;
        FUNCTION_DEBUG_SENTRY_RET(LONG, err);

        HKEY key;
        DWORD size = sizeof(DWORD);
        DWORD type = REG_NONE;
        
        err = RegOpenKeyEx(hKey, apKeyName, 0, KEY_READ, &key);
        if (err == ERROR_SUCCESS)
        {
            err = RegQueryValueEx(key, apKeyValueName, NULL, &type, (BYTE*)&aKeyValue, &size);
            (void)RegCloseKey(hKey);
            if ( !(type == REG_BINARY || type == REG_DWORD || type == REG_DWORD_LITTLE_ENDIAN) )
            {
                err = ERROR_INVALID_DATATYPE;
            }
            else if (type == REG_DWORD_BIG_ENDIAN) /* unlikely but then ... */
            {
                aKeyValue = _byteswap_ulong(aKeyValue);
            }
        }

        return err;
    }

    /////////////////////////////////////////////////////////////////////////////

    LONG WriteStringToRegistry(HKEY hKey, const char *apKeyName, const char *apKeyValueName, const std::string &aKeyValue, bool &aKeyAlreadyExisted)
    {
        LONG err = ERROR_SUCCESS;
        FUNCTION_DEBUG_SENTRY_RET(LONG, err);

        HKEY resultKey;
        DWORD disposition;

        err = RegCreateKeyEx(
                hKey,
                apKeyName,
                NULL,
                NULL,
                0,
                KEY_READ | KEY_WRITE,
                NULL,
                &resultKey,
                &disposition);

        if (err == ERROR_SUCCESS)
        {
            aKeyAlreadyExisted = (disposition == REG_OPENED_EXISTING_KEY) ? true : false;

            err = RegSetValueEx(
                    resultKey,
                    apKeyValueName,
                    NULL,
                    REG_SZ,
                    reinterpret_cast<const BYTE*>(aKeyValue.c_str()),
                    static_cast<DWORD>(aKeyValue.size()));

            (void)RegCloseKey(hKey);
        }

        return err;
    }

    /////////////////////////////////////////////////////////////////////////////

    LONG DeleteRegistryKey(HKEY hKey, const char *apKeyName)
    {
        LONG err = ERROR_SUCCESS;
        FUNCTION_DEBUG_SENTRY_RET(LONG, err);

        err = RegDeleteKey(hKey, apKeyName);

        return err;
    }


    LONG DeleteRegistryKeyValue(HKEY hKey, const char *apKeyName, const char *apKeyValueName)
    {
        LONG err = ERROR_SUCCESS;
        FUNCTION_DEBUG_SENTRY_RET(LONG, err);

        HKEY key;

        err = RegOpenKeyEx(hKey, apKeyName, 0, KEY_SET_VALUE, &key);

        if (err == ERROR_SUCCESS)
        {
            err = RegDeleteValue(key, apKeyValueName);
        }

        return err;
    }
}

