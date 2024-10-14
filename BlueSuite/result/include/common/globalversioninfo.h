//
// globalversioninfo.h
//
// Copyright (c) 2010-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Company-wide version information
//
// Restrict text characters to ASCII subset. Otherwise conversion from resource text codepage to
// console codepage will be required.
//
#ifndef GLOBALVERSIONINFO_H
#define GLOBALVERSIONINFO_H

#include "common/dynamicversioninfo.h"

// Company-wide strings
#define VERSION_SHORT_COMPANY_NAME  "QTIL"
#define VERSION_LONG_COMPANY_NAME   "Qualcomm Technologies International, Ltd"

#ifdef ISPP_INVOKED
// Need this format when using this file within Inno Setup scripts.
// ISPP_INVOKED is always defined by the Inno Setup Preprocessor.
// Note, newline not supported for Inno Setup AppCopyright setting.
#define VERSION_COPYRIGHT_NO_START_YEAR         "Copyright (c) " + VERSION_YEAR + " " + VERSION_LONG_COMPANY_NAME + ". " + \
                                                "All Rights Reserved. " + \
                                                "Qualcomm Technologies International, Ltd. Confidential and Proprietary."
#define VERSION_COPYRIGHT_START_YEAR(startYear) "Copyright (c) " + startYear + "-" + VERSION_YEAR + " " + VERSION_LONG_COMPANY_NAME + ". " + \
                                                "All Rights Reserved. " + \
                                                "Qualcomm Technologies International, Ltd. Confidential and Proprietary."
#else
#define VERSION_COPYRIGHT_NO_START_YEAR         "Copyright (c) " VERSION_YEAR " " VERSION_LONG_COMPANY_NAME ".\n" \
                                                "All Rights Reserved.\n" \
                                                "Qualcomm Technologies International, Ltd. Confidential and Proprietary."
#define VERSION_COPYRIGHT_START_YEAR(startYear) "Copyright (c) " startYear "-" VERSION_YEAR " " VERSION_LONG_COMPANY_NAME ".\n" \
                                                "All Rights Reserved.\n" \
                                                "Qualcomm Technologies International, Ltd. Confidential and Proprietary."
#endif // ISPP_INVOKED

#endif // GLOBALVERSIONINFO_H
