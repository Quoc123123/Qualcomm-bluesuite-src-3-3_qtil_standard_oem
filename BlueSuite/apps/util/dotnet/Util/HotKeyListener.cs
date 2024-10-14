//------------------------------------------------------------------------------
//
// <copyright file="HotKeyListener.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Security.Permissions;
using System.Windows.Forms;

using QTIL.HostTools.Common.EngineFrameworkClr;

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// NativeWindow derived class to listen to operating system messages for 'hot keys'.
    /// </summary>
    [
    System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "HotKey"), PermissionSet(SecurityAction.Demand, Name = "FullTrust")
    ]
    public class HotKeyListener
        : NativeWindow, IDisposable
    {
        #region IDisposable support

        /// <summary>
        /// Tracks whether Dispose has been called.
        /// </summary>
        private Boolean mDisposed;

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual. A derived class should not be able to override this method.
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to take this object off the 
            //  finalization queue and prevent finalization code for this object from 
            //  executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources
        /// </summary>
        /// <param name="disposing"><c>true</c> to release both managed and unmanaged resources; <c>false</c> to release only unmanaged resources.</param>
        /// <remarks>
        /// Dispose(bool disposing) executes in two distinct scenarios.
        /// If disposing equals true, the method has been called directly or indirectly 
        /// by a user's code. Managed and unmanaged resources can be disposed.
        /// If disposing equals false, the method has been called by the runtime from 
        /// inside the finalizer and you should not reference other objects. 
        /// Only unmanaged resources can be disposed.
        /// </remarks>
        private void Dispose(bool disposing)
        {
            MessageHandler.DebugEntry();

            // Check to see if Dispose has already been called.
            if (!mDisposed)
            {
                // If disposing equals true, dispose all managed and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    // Nothing to do
                }

                // Call the appropriate methods to clean up unmanaged resources here.
                // If disposing is false, only the following code is executed.
                Unregister();

                // Note disposing has been done.
                mDisposed = true;
            }

            MessageHandler.DebugExit();
        }
        
        #endregion

        #region Local data

        /// <summary>
        /// Constant value was found in the "windows.h" header file.
        /// </summary>
        private const Int32 WM_HOTKEY = 0x312;

        /// <summary>
        /// Contains the list of hotkeys that are handled.
        /// </summary>
        private readonly Dictionary<UInt16, Keys> mHotKeys;

        /// <summary>
        /// Contains the parent form's handle.
        /// </summary>
        private IntPtr mFormHandle;

        #endregion

        #region Events

        /// <summary>
        /// Occurs when hot key pressed.
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "HotKey")]
        public event EventHandler<KeyEventArgs> HotKeyPressed;

        /// <summary>
        /// Raises the <see cref="E:HotKeyPressed"/> event.
        /// </summary>
        /// <param name="e">The <see cref="System.Windows.Forms.KeyEventArgs"/> instance containing the event data.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "HotKey")]
        protected virtual void OnHotKeyPressed(KeyEventArgs e)
        {
            MessageHandler.DebugEntry();

            if (HotKeyPressed != null)
            {
                MessageHandler.DebugEnhanced("Raising HotKeyPressed event");

                HotKeyPressed(this, e);
            }

            MessageHandler.DebugExit();
        }

        #endregion

        #region Support methods

        /// <summary>
        /// Convert Keys.Modifiers to fsModifiers
        /// </summary>
        /// <param name="key">HotKey</param>
        /// <param name="keyModifier">Key Modifier from Keys enum</param>
        /// <param name="hotKeyModifier">NativeMethods.fsModifiers equivalent</param>
        /// <returns>Matched Hotkeymodifier</returns>
        private static NativeMethods.fsModifiers ConvertKeysTofsModifiers(Keys key, Keys keyModifier, NativeMethods.fsModifiers hotKeyModifier)
        {
            MessageHandler.DebugEntry();

            NativeMethods.fsModifiers retVal;
            if ((key & keyModifier) == keyModifier)
            {
                retVal = hotKeyModifier;
            }
            else
            {
                retVal = NativeMethods.fsModifiers.None;
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Register the supplied Hotkey
        /// </summary>
        /// <param name="hWnd">Handle for hotkey event receiver</param>
        /// <param name="id">Global atom id for given hotkey</param>
        /// <param name="key">HotKey</param>
        /// <returns>success flag of register action</returns>
        private static Boolean RegisterKey(IntPtr hWnd, Int32 id, Keys key)
        {
            Boolean retVal;

            MessageHandler.DebugEntry();

            NativeMethods.fsModifiers modifier = NativeMethods.fsModifiers.None;
            modifier |= ConvertKeysTofsModifiers(key, Keys.Control, NativeMethods.fsModifiers.Control);
            modifier |= ConvertKeysTofsModifiers(key, Keys.Alt, NativeMethods.fsModifiers.Alt);
            modifier |= ConvertKeysTofsModifiers(key, Keys.Shift, NativeMethods.fsModifiers.Shift);

            retVal = NativeMethods.RegisterHotKey(hWnd, id, modifier, key & Keys.KeyCode);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Unregister specified hotkey
        /// </summary>
        /// <param name="hWnd">Hotkey event receiver</param>
        /// <param name="hotKeyId">Global atom id for given hotkey</param>
        /// <returns>success flag of unregister action</returns>
        private static Boolean UnregisterKey(IntPtr hWnd, Int32 hotKeyId)
        {
            MessageHandler.DebugEntry();

            Boolean retVal = NativeMethods.UnregisterHotKey(hWnd, hotKeyId);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        #endregion

        #region Registration

        /// <summary>
        /// Register the given hotkey
        /// </summary>
        /// <param name="hotKey">hotkey</param>
        /// <returns>success flag of register action</returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "hotKey")]
        public Boolean Register(Keys hotKey)
        {
            Boolean retVal = false;

            MessageHandler.DebugEntry();

            Unregister(hotKey);

            UInt16 hotKeyId = NativeMethods.GlobalAddAtom(String.Format(CultureInfo.InvariantCulture, "HK:{0}", hotKey));
            if (hotKeyId == 0)
            {
                String errorMessage = String.Format(CultureInfo.InvariantCulture, "Could not register atom for hotkey {0}.", hotKey);
                MessageHandler.DebugBasic(errorMessage);
                MessageHandler.DebugExit();
                throw new ArgumentException(errorMessage);
            }

            if (RegisterKey(mFormHandle, hotKeyId, hotKey))
            {
                mHotKeys.Add(hotKeyId, hotKey);
                retVal = true;
            }

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Unregister the specified registered hot key
        /// </summary>
        /// <param name="hotKey">A previously registered hotkey</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "hotKey")]
        public void Unregister(Keys hotKey)
        {
            MessageHandler.DebugEntry();

            UInt16 key = 0;
            foreach (KeyValuePair<UInt16, Keys> kvp in mHotKeys)
            {
                if (kvp.Value == hotKey)
                {
                    key = kvp.Key;

                    UnregisterKey(mFormHandle, key);
                    NativeMethods.GlobalDeleteAtom(key);
                    break;
                }
            }
            if (key > 0)
            {
                mHotKeys.Remove(key);
            }

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Unregister all registered hot keys
        /// </summary>
        public void Unregister()
        {
            MessageHandler.DebugEntry();

            foreach (KeyValuePair<UInt16, Keys> kvp in mHotKeys)
            {
                UInt16 key = kvp.Key;

                UnregisterKey(mFormHandle, key);
                NativeMethods.GlobalDeleteAtom(key);
            }
            mHotKeys.Clear();

            MessageHandler.DebugExit();
        }

        #endregion

        #region Overrides

        /// <summary>
        /// Called when handle created.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        internal void OnHandleCreated(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();

            // Window is now created, assign handle to NativeWindow.
            mFormHandle = ((Form)sender).Handle;
            AssignHandle(mFormHandle);

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Called when handle destroyed.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        internal void OnHandleDestroyed(object sender, EventArgs e)
        {
            MessageHandler.DebugEntry();

            // Window was destroyed, release hook.
            ReleaseHandle();

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Invokes the default window procedure associated with this window.
        /// </summary>
        /// <param name="m">A <see cref="T:System.Windows.Forms.Message"/> that is associated with the current Windows message.</param>
        [PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
        protected override void WndProc(ref Message m)
        {
            // Listen for operating system messages
            if (m.Msg == WM_HOTKEY)
            {
                OnHotKeyPressed(new KeyEventArgs(mHotKeys[(UInt16)m.WParam.ToInt32()]));
            }
            else
            {
                base.WndProc(ref m);
            }
        }

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Initializes a new instance of the <see cref="HotKeyListener"/> class.
        /// </summary>
        /// <param name="parent">The parent.</param>
        public HotKeyListener(Form parent)
        {
            MessageHandler.DebugEntry();

            // Hook events
            parent.HandleCreated += OnHandleCreated;
            parent.HandleDestroyed += OnHandleDestroyed;
            mFormHandle = parent.Handle;
            AssignHandle(mFormHandle);

            mHotKeys = new Dictionary<UInt16, Keys>();

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="HotKeyListener"/> is reclaimed by garbage collection.
        /// </summary>
        /// <remarks>
        /// Dispose invoked just in case consumer didn't do it.
        /// </remarks>
        ~HotKeyListener()
        {
            Dispose(false);
        }

        #endregion

    }
}
