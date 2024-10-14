/**********************************************************************
 *
 *  sysutil.cpp
 *
 *  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions related to the system / platform.
 *
 ***********************************************************************/

#include "sysutil.h"
#include "engine/enginefw_interface.h"

#if defined(WIN32)
#include <windows.h>
#endif // WIN32

using namespace std;

// Automatically add any non-class methods to the UTILITY group
#undef  EF_GROUP
#define EF_GROUP CMessageHandler::GROUP_ENUM_UTILITY

namespace sysutil
{

#if defined(WIN32) && !defined(WINCE)
    bool GetWinVersion(uint32& aMajorVer, uint32& aMinorVer, bool& aIs64Bit)
    {
        bool ret = false;
        FUNCTION_DEBUG_SENTRY_RET(bool, ret);

        OSVERSIONINFO versionInfo;
        versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        BOOL result = GetVersionEx(&versionInfo);
        if (result != FALSE)
        {
            aMajorVer = versionInfo.dwMajorVersion;
            aMinorVer = versionInfo.dwMinorVersion;
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Windows OS Version Major=%d, Minor=%d",
                                     aMajorVer, aMinorVer);

            SYSTEM_INFO systemInfo;
            ZeroMemory(&systemInfo, sizeof(SYSTEM_INFO));
            GetNativeSystemInfo(&systemInfo);

            aIs64Bit = (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
                        systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
            MSG_HANDLER_NOTIFY_DEBUG(DEBUG_BASIC, "Windows is %dbit", (aIs64Bit ? 64 : 32));

            ret = true;
        }
        else
        {
            MSG_HANDLER.SetErrorMsg(0, "Error getting Windows version information");
        }

        return ret;
    }
#endif // defined(WIN32) && !defined(WINCE)

    /////////////////////////////////////////////////////////////////////////////

}
