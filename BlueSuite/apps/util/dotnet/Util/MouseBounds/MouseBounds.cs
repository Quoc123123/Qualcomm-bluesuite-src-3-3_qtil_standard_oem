//------------------------------------------------------------------------------
//
// <copyright file="MouseBounds.cs" company="Qualcomm Technologies International, Ltd.">
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
using System.Drawing;
using System.Windows.Forms;

namespace QTIL.HostTools.Common.Util
{

    /// <summary>
    /// Implements a message filter to indicate whether the mouse is within the bounds of a control.
    /// </summary>
    /// <remarks>
    /// This enables simple detection of whether the mouse is within the control, effectively
    /// igonring whether the mouse is within child control(s).
    /// Assumes that controls to be monitored are NOT overlapping.
    /// </remarks>
    public class MouseBounds
        : IMessageFilter, IDisposable
    {

        /// <summary>
        /// Constant from windows.h
        /// </summary>
        private const Int32 WM_NCMOUSEMOVE = 0x00A0;

        /// <summary>
        /// Constant from windows.h
        /// </summary>
        private const Int32 WM_MOUSEMOVE = 0x200;

        /// <summary>
        /// Constant from windows.h
        /// </summary>
        private const Int32 WM_NCMOUSELEAVE = 0x02A2;

        /// <summary>
        /// Constant from windows.h
        /// </summary>
        private const Int32 WM_MOUSELEAVE = 0x02A3;

        /// <summary>
        /// Contains the control(s) being monitored.
        /// </summary>
        /// <remarks>
        /// Dictionary of control -> state.
        /// </remarks>
        private readonly Dictionary<Control, Boolean> mControlStates;

        /// <summary>
        /// Gets a value indicating whether the mouse is within the bounds of the control.
        /// </summary>
        /// <param name="control">The control.</param>
        /// <returns>
        /// <c>true</c> if the mouse is within the bounds of the specified control; otherwise, <c>false</c>.
        /// </returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1702:CompoundWordsShouldBeCasedCorrectly", MessageId = "InBounds")]
        public Boolean MouseInBounds(Control control)
        {
            Boolean retVal = false;

            mControlStates.TryGetValue(control, out retVal);

            return retVal;
        }

        #region Events

        /// <summary>
        /// Occurs when the mouse moves in to the bounds of the control.
        /// </summary>
        public event EventHandler<MouseBoundsEventArgs> EnterControl;

        /// <summary>
        /// Raises the <see cref="E:EnterControl"/> event.
        /// </summary>
        /// <param name="e">The <see cref="System.MouseBoundsEventArgs"/> instance containing the event data.</param>
        private void OnEnterControl(MouseBoundsEventArgs e)
        {
            if (EnterControl != null)
            {
                EnterControl(this, e);
            }
        }

        /// <summary>
        /// Occurs when the mouse moves out of the bounds of the control.
        /// </summary>
        public event EventHandler<MouseBoundsEventArgs> LeaveControl;

        /// <summary>
        /// Raises the <see cref="E:LeaveControl"/> event.
        /// </summary>
        /// <param name="e">The <see cref="System.MouseBoundsEventArgs"/> instance containing the event data.</param>
        private void OnLeaveControl(MouseBoundsEventArgs e)
        {
            if (LeaveControl != null)
            {
                LeaveControl(this, e);
            }
        }

        #endregion

        #region Methods

        /// <summary>
        /// Adds the specified control.
        /// </summary>
        /// <param name="control">The control.</param>
        public void Add(Control control)
        {
            mControlStates.Add(control, false);
        }

        /// <summary>
        /// Filters out a message before it is dispatched.
        /// </summary>
        /// <param name="m">The message to be dispatched. You cannot modify this message.</param>
        /// <returns>
        /// true to filter the message and stop it from being dispatched; false to allow the message to continue to the next filter or control.
        /// </returns>
        public Boolean PreFilterMessage(ref Message m)
        {
            switch (m.Msg)
            {
                case WM_MOUSEMOVE:
                case WM_NCMOUSEMOVE:
                case WM_MOUSELEAVE:
                case WM_NCMOUSELEAVE:
                    CheckControlBounds();
                    break;

                default:
                    break;
            }

            // don't actually filter the message
            return false;
        }

        /// <summary>
        /// Checks if the current cursor position is contained within the bounds of the
        /// control and sets MouseInBounds which in turn fires event MouseBoundsChanged
        /// </summary>
        private void CheckControlBounds()
        {
            Control lastControl = null;
            Control inControl = null;

            // Find which, if any, of the controls last contained the mouse
            foreach (KeyValuePair<Control, Boolean> kvp in mControlStates)
            {
                if (kvp.Value)
                {
                    lastControl = kvp.Key;
                    break;
                }
            }

            // Find which, if any, of the controls contains the mouse
            foreach (Control control in mControlStates.Keys)
            {
                Rectangle controlBounds = control.Bounds;
                Point controlLocation = control.Parent.PointToScreen(Point.Empty);
                controlBounds.Offset(controlLocation);
                if (controlBounds.Contains(Cursor.Position))
                {
                    inControl = control;
                    break;
                }
            }

            // Have we changed control
            if (lastControl != inControl)
            {
                // Changed 
                if (lastControl != null)
                {
                    // Flag that we've left the last control
                    mControlStates[lastControl] = false;
                    OnLeaveControl(new MouseBoundsEventArgs(lastControl));
                }

                // Flag that we've entered a control
                if (inControl != null)
                {
                    mControlStates[inControl] = true;
                    OnEnterControl(new MouseBoundsEventArgs(inControl));
                }
            }
        }

        #endregion

        #region Constructors etc.

        /// <summary>
        /// Initializes a new instance of the <see cref="MouseBounds"/> class.
        /// </summary>
        public MouseBounds()
        {
            mControlStates = new Dictionary<Control, bool>();
            Application.AddMessageFilter(this);
        }

        #region IDisposable

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources
        /// </summary>
        /// <param name="disposing"><c>true</c> to release both managed and unmanaged resources; <c>false</c> to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                Application.RemoveMessageFilter(this);
            }
            // No unmanaged objects
        }

        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="MouseBounds"/> is reclaimed by garbage collection.
        /// </summary>
        ~MouseBounds()
        {
            // Simply call Dispose(false).
            Dispose(false);
        }

        #endregion

        #endregion

    }
}
