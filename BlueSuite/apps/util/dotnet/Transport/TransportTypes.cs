//------------------------------------------------------------------------------
//
// <copyright file="TransportTypes.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2022 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;

namespace QTIL.HostTools.Common.Transport
{
    /// <summary>
    /// These match the TestEngine defined values for openTestEngine.
    /// </summary>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA2217:DoNotMarkEnumsWithFlags"), Flags]
    public enum TransportTypes
    {
        /// <summary>
        ///
        /// </summary>
        None = 0,

        /// <summary>
        ///
        /// </summary>
        BCSP = 1,

        /// <summary>
        ///
        /// </summary>
        USB = 2,

        /// <summary>
        ///
        /// </summary>
        H4 = 4,

        /// <summary>
        ///
        /// </summary>
        H5 = 8,

        /// <summary>
        ///
        /// </summary>
        H4DS = 16,

        /// <summary>
        /// This isn't defined for TestEngine, as we use openTestEngineDebug instead.
        /// </summary>
        SPI = 32,

        /// <summary>
        ///
        /// </summary>
        PTAP = 64,

        /// <summary>
        ///
        /// </summary>
        TRB = 128,

        /// <summary>
        ///
        /// </summary>
        USBDBG = 256,

        /// <summary>
        ///
        /// </summary>
        USBCC = 512,

        /// <summary>
        ///
        /// </summary>
        ADBBT = 1024,

        /// <summary>
        ///
        /// </summary>
        Serial = BCSP | H4 | H5 | H4DS,

        /// <summary>
        ///
        /// </summary>
        Any = 0xffff
    }
}
