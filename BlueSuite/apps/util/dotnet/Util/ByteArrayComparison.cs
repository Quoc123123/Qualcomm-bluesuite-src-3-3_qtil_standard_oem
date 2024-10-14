//------------------------------------------------------------------------------
//
// <copyright file="ByteArrayComparison.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
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

namespace QTIL.HostTools.Common.Util
{
    public class ByteArrayComparison
    {
        /// <summary>
        /// PInvokes memcmp on two byte arrays.
        /// </summary>
        /// <param name="b1">The first byte array.</param>
        /// <param name="b2">The second byte array.</param>
        /// <param name="count">The count.</param>
        /// <returns></returns>
        [DllImport("msvcrt.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Int32 memcmp(Byte[] b1, Byte[] b2, Int64 count);

        /// <summary>
        /// Compares the two byte lists.
        /// </summary>
        /// <param name="b1">The first array.</param>
        /// <param name="b2">The second array.</param>
        /// <returns></returns>
        public static Boolean Compare(List<Byte> b1, List<Byte> b2)
        {
            // Validate buffers are the same length.
            // This also ensures that the count does not exceed the length of either buffer.  
            return (b1.Count == b2.Count) && (memcmp(b1.ToArray(), b2.ToArray(), b1.Count) == 0);
        }

        /// <summary>
        /// Compares the two byte arrays.
        /// </summary>
        /// <param name="b1">The first array.</param>
        /// <param name="b2">The second array.</param>
        /// <returns></returns>
        public static Boolean Compare(Byte[] b1, Byte[] b2)
        {
            // Validate buffers are the same length.
            // This also ensures that the count does not exceed the length of either buffer.  
            return (b1.Length == b2.Length) && (memcmp(b1, b2, b1.Length) == 0);
        }

    }
}

