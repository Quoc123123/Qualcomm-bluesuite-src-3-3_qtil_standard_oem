//------------------------------------------------------------------------------
//
// <copyright file="SerialPortUtil.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Globalization;
using System.Text.RegularExpressions;

namespace QTIL.HostTools.Common.Transport
{
    /// <summary>
    /// Class containing Serial Port utility methods
    /// </summary>
    public sealed class SerialPortUtil
    {
        /// <summary>
        /// Prefix used for Windows serial ports.
        /// </summary>
        const String WIN_SP_PREFIX = "COM";

        /// <summary>
        /// IComparer class used for sorting "COMxx" strings in
        /// numerical order
        /// </summary>
        private class OrderComNumeric : IComparer<String>
        {
            public Int32 Compare(String aStr1, String aStr2)
            {
                Int32 result;

                // Ignore the prefix and sort as numbers.
                Int32 portNum1;
                Int32 portNum2;
                if (Int32.TryParse(aStr1.Substring(WIN_SP_PREFIX.Length), NumberStyles.None, CultureInfo.InvariantCulture, out portNum1) &&
                    Int32.TryParse(aStr2.Substring(WIN_SP_PREFIX.Length), NumberStyles.None, CultureInfo.InvariantCulture, out portNum2))
                {
                    result = portNum1.CompareTo(portNum2);
                }
                else
                {
                    // Fall back on a string compare if there's a problem converting the port numbers.
                    result = aStr1.CompareTo(aStr2);
                }

                return result;
            }
        }

        /// <summary>
        /// Populate a ComboBox with the available serial ports
        /// </summary>
        public static void AddAvailablePorts(ComboBox aCombo)
        {
            // Get serial ports
            List<String> ports = new List<String>();
            foreach (String port in System.IO.Ports.SerialPort.GetPortNames())
            {
                // If it isn't "COM*", we're ignoring it, as all serial ports should be named COM* on Windows.
                // SerialPort.GetPortNames() returns port names from the registry, and for some devices,
                // such as some Bluetooth serial ports, the names may not be zero terminated (which is against the rules).
                // The result is that we can get strings with random characters appended.
                // Workaround is to remove any characters after COMn which are not digits. This doesn't catch cases
                // where incorrect digits are present, but these cases should be caught either by the check for
                // duplicates, or by the check that the port can be opened (PortAvailable()).
                int index = port.IndexOf(WIN_SP_PREFIX);
                if (index == 0)
                {
                    String portStr = port;
                    if (port.Length > (WIN_SP_PREFIX.Length + 1)) // There should always be one digit
                    {
                        index += WIN_SP_PREFIX.Length;
                        // Remove any invalid characters
                        portStr = String.Format("{0}{1}", WIN_SP_PREFIX, Regex.Replace(port.Substring(index), "[^0-9]", ""));
                    }

                    // Remove any duplicates as we go (shouldn't have any, but if
                    // windows has a setup issue, it can happen).
                    if (!ports.Contains(portStr))
                    {
                        ports.Add(portStr);
                    }
                }
            }

            // Sort the ports (want ascending order, needs custom sort)
            ports.Sort(new OrderComNumeric());

            // Now populate
            foreach (String port in ports)
            {
                // Use Windows port name with prefix, works for all ports (without it, only 1 - 9 can be opened)
                String fullPortName = String.Format(CultureInfo.CurrentCulture, @"\\.\{0}", port);

                // Only add the available ports (not in use)
                if (PortAvailable(fullPortName))
                {
                    aCombo.Items.Add(new Transport(port, fullPortName));
                }
            }
        }

        /// <summary>
        /// Determine if serial port is available i.e. can be opened
        /// </summary>
        /// <param name="port">Name of the port.</param>
        /// <returns>
        ///   <c>true</c> if serial port is available; otherwise, <c>false</c>.
        /// </returns>
        private static Boolean PortAvailable(String aPort)
        {
            Boolean isAvailable = false;

            IntPtr portHandle = NativeMethods.CreateFile(aPort, NativeMethods.GENERIC_READ | NativeMethods.GENERIC_WRITE, 0, IntPtr.Zero, NativeMethods.OPEN_EXISTING, NativeMethods.FILE_ATTRIBUTE_NORMAL, IntPtr.Zero);

            if (portHandle.ToInt32() != -1)
            {
                NativeMethods.CloseHandle(portHandle);
                isAvailable = true;
            }

            return isAvailable;
        }

    }
}
