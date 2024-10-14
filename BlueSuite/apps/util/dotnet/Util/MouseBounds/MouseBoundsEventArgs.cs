//------------------------------------------------------------------------------
//
// <copyright file="MouseBoundsEventArgs.cs" company="Qualcomm Technologies International, Ltd.">
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

namespace QTIL.HostTools.Common.Util
{
    /// <summary>
    /// EventArgs for MouseBounds events
    /// </summary>
    public class MouseBoundsEventArgs
        : EventArgs
    {
        /// <summary>
        /// Contains the control
        /// </summary>
        private readonly Control mControl;

        /// <summary>
        /// Gets the control.
        /// </summary>
        public Control Control
        {
            get
            {
                return mControl;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MouseBoundsEventArgs"/> class.
        /// </summary>
        /// <param name="control">The control.</param>
        public MouseBoundsEventArgs(Control control)
        {
            mControl = control;
        }
    }
}
