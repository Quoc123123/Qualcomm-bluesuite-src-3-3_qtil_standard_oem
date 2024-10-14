//------------------------------------------------------------------------------
//
// <copyright file="MACAddress.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// Static class which currently only provides functions to convert MAC
    /// addresses to and from strings.
    /// </summary>
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "MAC")]
    [CLSCompliant(false)] // Due to use of UInt16 parameters
    public static class MACAddress
    {
        /// <summary>
        /// Converts a string containing a MAC address to 3 ushorts. Also
        /// formats the resulting address back into a string. This can be
        /// useful to update user controls, e.g. if the user has typed
        /// "AABBCCDDEEFF" this function will return "AA-BB-CC-DD-EE-FF", which
        /// can be stored straight back into the user control.
        /// </summary>
        /// <param name="address">The string to be converted</param>
        /// <param name="dest_0">First part of the resulting MAC address</param>
        /// <param name="dest_1">Second part of the resulting MAC address</param>
        /// <param name="dest_2">Third part of the resulting MAC address</param>
        /// <returns>Resulting address converted back to a string</returns>
        /// 
        public static String FromString(String address, out UInt16 dest_0, out UInt16 dest_1, out UInt16 dest_2)
        {
            try
            {
                // If colons used then return formatted with colons, otherwise use hyphens
                Boolean has_colons = (address.IndexOf(':') >= 0);
                
                address = address.Replace(" ", "");
                address = address.Replace("-", "");
                address = address.Replace(":", "");
                
                dest_0 = Convert.ToUInt16(String.Format("{0}{1}{2}{3}", address[2], address[3], address[0], address[1]), 16);
                dest_1 = Convert.ToUInt16(String.Format("{0}{1}{2}{3}", address[6], address[7], address[4], address[5]), 16);
                dest_2 = Convert.ToUInt16(String.Format("{0}{1}{2}{3}", address[10], address[11], address[8], address[9]), 16);

                return ToString(dest_0, dest_1, dest_2, has_colons);
            }
            catch (Exception e)
            {
                if (e.Message == "Invalid hex character")
                {
                    throw new Exception("Failed to parse MAC address: Invalid hex character found.");
                }
                else
                {
                    throw;
                }
            }
        }

        /// <summary>
        /// Convert a MAC address stored in three ushorts to a string.
        /// </summary>
        public static String ToString(UInt16 src_0, UInt16 src_1, UInt16 src_2)
        {
            return ToString(src_0, src_1, src_2, false);
        }

        /// <summary>
        /// Convert a MAC address stored in three ushorts to a string.
        /// </summary>
        public static String ToString(UInt16 src_0, UInt16 src_1, UInt16 src_2, Boolean use_colons)
        {
            String fmt = "{1:X2}-{0:X2}-{3:X2}-{2:X2}-{5:X2}-{4:X2}";

            if (use_colons)
            {
                fmt = fmt.Replace('-', ':');
            }

            return String.Format(fmt, src_0 >> 8, src_0 & 0xFF,
                                      src_1 >> 8, src_1 & 0xFF,
                                      src_2 >> 8, src_2 & 0xFF);
        }

    }
}

