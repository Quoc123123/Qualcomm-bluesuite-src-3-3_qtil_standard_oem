//
// GlobalVersionInfo.cs
//
// Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//
// Company-wide version information
//
using System;

namespace QTIL.HostTools.Common.VersionInfo
{
    /// <summary>
    /// Data class defining global version information
    /// </summary>
    internal static class GlobalVersionInfo
    {
        public const String VERSION_SHORT_COMPANY_NAME = "QTIL";
        public const String VERSION_LONG_COMPANY_NAME = "Qualcomm Technologies International, Ltd";

        public const String VERSION_COPYRIGHT_NO_START_YEAR = "Copyright (c) " + DynamicVersionInfo.VERSION_YEAR + " " + VERSION_LONG_COMPANY_NAME + ".\r\n" +
                                                              "All Rights Reserved.\r\n" +
                                                              "Qualcomm Technologies International, Ltd. Confidential and Proprietary.";
        // The following prefix and suffix strings are used around copyright years,
        // because we have to provide constants for assembly attribute values
        public const String VERSION_COPYRIGHT_START_YEAR_PREFIX = "Copyright (c) ";
        public const String VERSION_COPYRIGHT_START_YEAR_SUFFIX = "-" + DynamicVersionInfo.VERSION_YEAR + " " + VERSION_LONG_COMPANY_NAME + ".\r\n" +
                                                                  "All Rights Reserved.\r\n" +
                                                                  "Qualcomm Technologies International, Ltd. Confidential and Proprietary.";
    }
}
