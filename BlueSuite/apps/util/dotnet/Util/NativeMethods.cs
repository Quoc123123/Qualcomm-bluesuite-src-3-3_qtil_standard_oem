//------------------------------------------------------------------------------
//
// <copyright file="NativeMethods.cs" company="Qualcomm Technologies International, Ltd.">
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
using System.Windows.Forms;

namespace QTIL.HostTools.Common.Util
{
    internal class NativeMethods
    {
        /// <summary>
        /// Adds a character string to the global atom table.
        /// </summary>
        /// <param name="lpString">The string.</param>
        /// <returns>
        /// Returns a unique value (an atom) identifying the string. 
        /// </returns>
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern UInt16 GlobalAddAtom(String lpString);

        /// <summary>
        /// Decrements the reference count of a global string atom. 
        /// If the atom's reference count reaches zero, GlobalDeleteAtom removes the string 
        /// associated with the atom from the global atom table.
        /// </summary>
        /// <param name="nAtom">The atom.</param>
        /// <returns></returns>
        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        internal static extern UInt16 GlobalDeleteAtom(UInt16 nAtom);

        /// <summary>
        /// Keymodifers for hotkey
        /// </summary>
        [Flags]
        internal enum fsModifiers
        {
            /// <summary>
            /// None.
            /// </summary>
            None = 0,

            /// <summary>
            /// Alt key.
            /// </summary>
            Alt = 1,

            /// <summary>
            /// Control key.
            /// </summary>
            Control = 2,

            /// <summary>
            /// Shift Key
            /// </summary>
            Shift = 4,

            /// <summary>
            /// Either Windows key
            /// </summary>
            Win = 8
        }

        /// <summary>
        /// Defines a system-wide hot key.
        /// </summary>
        /// <param name="hWnd">The window handle.</param>
        /// <param name="keyId">The key id.</param>
        /// <param name="fsModifiers">The key modifiers.</param>
        /// <param name="vk">The vk.</param>
        /// <returns></returns>
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern Boolean RegisterHotKey(IntPtr hWnd, Int32 keyId, fsModifiers fsModifiers, Keys vk);

        /// <summary>
        /// Frees a hot key previously registered by the calling thread. 
        /// </summary>
        /// <param name="hWnd">The window handle.</param>
        /// <param name="id">The identifier of the hot key to be freed.</param>
        /// <returns></returns>
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern Boolean UnregisterHotKey(IntPtr hWnd, Int32 id);

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
