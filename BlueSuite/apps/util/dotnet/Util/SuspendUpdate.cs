//------------------------------------------------------------------------------
//
// <copyright file="SuspendUpdate.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Windows.Forms;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// Simple class to wrap in a non-pInvoke manner methods to suspend/resume redrawing of a control.
    /// </summary>
    public static class SuspendUpdate
    {
        /// <summary>
        /// Windows message ID for SETREDRAW.
        /// </summary>
        private const int WM_SETREDRAW = 0x000B;

        /// <summary>
        /// Suspends drawing on the specified control.
        /// </summary>
        /// <param name="control">The control.</param>
        public static void Suspend(Control control)
        {
            MessageHandler.DebugEntry();

            Message msgSuspendUpdate = Message.Create(control.Handle, WM_SETREDRAW, IntPtr.Zero, IntPtr.Zero);

            NativeWindow window = NativeWindow.FromHandle(control.Handle);
            window.DefWndProc(ref msgSuspendUpdate);

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Resumes drawing on the specified control.
        /// </summary>
        /// <param name="control">The control.</param>
        public static void Resume(Control control)
        {
            MessageHandler.DebugEntry();

            // Create a C "true" boolean as an IntPtr 
            IntPtr wparam = new IntPtr(1);
            Message msgResumeUpdate = Message.Create(control.Handle, WM_SETREDRAW, wparam, IntPtr.Zero);

            NativeWindow window = NativeWindow.FromHandle(control.Handle);
            window.DefWndProc(ref msgResumeUpdate);

            control.Invalidate();

            MessageHandler.DebugExit();
        }
    }
}
