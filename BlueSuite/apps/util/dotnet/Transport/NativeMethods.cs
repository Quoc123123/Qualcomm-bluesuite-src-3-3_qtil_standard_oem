//------------------------------------------------------------------------------
//
// <copyright file="NativeMethods.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2020 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace QTIL.HostTools.Common.Transport
{
    // define some reasonable type mappings for pttransport types (see pttransport.h)
    using pttrans_stream_t = UInt32;
    using pttrans_return = Int32;

    public sealed class NativeMethods
    {
        #region PTTransport

        /// <summary>
        /// Value indicating success of a pttransport routine
        /// </summary>
        public const pttrans_return PTTRANS_SUCCESS = 0;

        /// <summary>
        /// Value indicating failure of a pttransport routine
        /// </summary>
        public const pttrans_return PTTRANS_FAILURE = 1;

        /// <summary>
        /// value of an unopened stream
        /// </summary>
        public const pttrans_stream_t PTTRANS_STREAM_INVALID = 0x7FFFFFFF;

        /// <summary>
        /// Enumerates available ports. (see pttransport.h for details)
        /// </summary>
        /// <param name="maxLen">The maximum stringt buffer length.</param>
        /// <param name="ports">The ports.</param>
        /// <param name="trans">The trans.</param>
        /// <param name="count">The number of ports enumerated.</param>
        /// <returns>PTTRANS_SUCCESS or PTTRANS_FAILURE</returns>
        [DllImport("pttransport.dll",
            EntryPoint = "pttrans_enumerate_strings",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi)
        ]
        internal static extern pttrans_return pttrans_enumerate_strings(ref UInt16 maxLen, StringBuilder ports, StringBuilder trans, ref UInt16 count);

        /// <summary>
        /// Opens a PtTransport stream using a connection string, setting p_stream if successful
        /// </summary>
        /// <param name="p_stream">output location for the handle for the stream that was opened</param>
        /// <param name="trans">a device connection string such as "SPITRANS=TRB SPIPORT=1"</param>
        /// <returns>PTTRANS_SUCCESS or PTTRANS_FAILURE</returns>
        [DllImport("pttransport.dll",
            EntryPoint = "pttrans_stream_open",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi)
        ]
        public static extern pttrans_return pttrans_stream_open(ref pttrans_stream_t p_stream, String trans);

        /// <summary>
        /// Closes a PtTransport stream
        /// </summary>
        /// <param name="stream">Handle for the stream that is to be closed</param>
        [DllImport("pttransport.dll",
            EntryPoint = "pttrans_stream_close",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi)
        ]
        public static extern void pttrans_stream_close(pttrans_stream_t stream);

        /// <summary>
        /// Gets a C-style pointer to the underlying CXapHelper object used by the specified stream.
        /// </summary>
        /// <param name="stream">Handle for the stream that is to be queried</param>
        /// <returns>A pointer to the underlying CXapHelper object associated with the stream</returns>
        [DllImport("pttransport.dll",
            EntryPoint = "GetCXapHelperFromStream",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi),
        ]
        public static extern IntPtr GetCXapHelperFromStream(pttrans_stream_t stream);

        #endregion

        #region Win32

        internal const uint GENERIC_READ = 0x80000000;
        internal const uint GENERIC_WRITE = 0x40000000;
        internal const int OPEN_EXISTING = 3;
        internal const int FILE_ATTRIBUTE_NORMAL = 0x80;

        /// <summary>
        /// Creates the file.
        /// </summary>
        /// <param name="lpFileName">Name of the file.</param>
        /// <param name="dwDesiredAccess">The desired access.</param>
        /// <param name="dwShareMode">The share mode.</param>
        /// <param name="SecurityAttributes">The security attributes.</param>
        /// <param name="dwCreationDisposition">The creation disposition.</param>
        /// <param name="dwFlagsAndAttributes">The flags and attributes.</param>
        /// <param name="hTemplateFile">The template file.</param>
        /// <returns></returns>
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
        internal static extern IntPtr CreateFile(
              String lpFileName,
              [MarshalAs(UnmanagedType.U4)] UInt32 dwDesiredAccess,
              [MarshalAs(UnmanagedType.U4)] UInt32 dwShareMode,
              IntPtr SecurityAttributes,
              [MarshalAs(UnmanagedType.U4)] UInt32 dwCreationDisposition,
              [MarshalAs(UnmanagedType.U4)] UInt32 dwFlagsAndAttributes,
              IntPtr hTemplateFile
              );

        /// <summary>
        /// Closes the handle.
        /// </summary>
        /// <param name="hObject">The object.</param>
        /// <returns></returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern Boolean CloseHandle(IntPtr hObject);

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Prevents a default instance of the <see cref="NativeMethods" /> class from being created.
        /// </summary>
        private NativeMethods()
        {
        }

        #endregion

    }
}
