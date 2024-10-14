//------------------------------------------------------------------------------
//
// <copyright file="PTapLibrary.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2022 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace QTIL.HostTools.Common.Transport
{
    public static class PTapLibrary
    {
        public const int OK = 0;
        public const int SHORT_STRINGS = 1;
        public const int BAD_ARGUMENT = 2;
        public const int FAULT = 3;

        [DllImport(
            "PTapLibrary.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumeratePtapHifs")]
        public extern static int EnumerateHifs(
            ref ushort length,
            StringBuilder hifIds,
            StringBuilder types,
            StringBuilder names,
            ref ushort count);

        [DllImport(
            "PTapLibrary.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumeratePtapServices")]
        public extern static int EnumerateServices(
            ref ushort length,
            StringBuilder serviceIds,
            StringBuilder curatorIds,
            StringBuilder serviceClasses,
            StringBuilder states,
            StringBuilder availabilities,
            StringBuilder hifIds,
            ref ushort count);
    }

    public static class HydProtocols
    {
        public const int RESULT_OK = 0;
        public const int RESULT_SHORT_STRINGS = 1;
        public const int RESULT_BAD_ARGUMENT = 2;
        public const int RESULT_FAULT = 3;

        [DllImport(
            "HydProtocols.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumerateTrbs")]
        public extern static int EnumerateTrbs(
            ref ushort length,
            StringBuilder names,
            StringBuilder transs,
            ref ushort count);

        [DllImport(
            "HydProtocols.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumerateUsbDbgs")]
        public extern static int EnumerateUsbDbgs(
            ref ushort length,
            StringBuilder names,
            StringBuilder transs,
            ref ushort count);

        [DllImport(
            "HydProtocols.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumerateUsbCcs")]
        public extern static int EnumerateUsbCcs(
            ref ushort length,
            StringBuilder names,
            StringBuilder transs,
            ref ushort count);

        [DllImport(
            "HydProtocols.dll",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi,
            EntryPoint = "EnumerateAdbBts")]
        public extern static int EnumerateAdbBts(
            ref ushort length,
            StringBuilder names,
            StringBuilder transs,
            ref ushort count);
    }
}
