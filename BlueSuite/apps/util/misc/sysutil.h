/**********************************************************************
 *
 *  sysutil.h
 *
 *  Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *
 *  Utility functions related to the system / platform.
 *
 ***********************************************************************/

#ifndef SYSUTIL_H
#define SYSUTIL_H

#include "common/types.h"

namespace sysutil
{
#if defined(WIN32) && !defined(WINCE)
    /**
     * Get the windows version information.
     * @param[out] aMajorVer Windows major version number.
     * @param[out] aMinorVer Windows minor version number.
     * @param[out] aIs64Bit True if 64bit windows, false if 32bit.
     * @return true on success, false on error.
     */
    bool GetWinVersion(uint32& aMajorVer, uint32& aMinorVer, bool& aIs64Bit);
#endif /* defined(WIN32) && !defined(WINCE) */
}

#endif

