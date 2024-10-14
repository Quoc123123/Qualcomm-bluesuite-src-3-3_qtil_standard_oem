//------------------------------------------------------------------------------
//
// <copyright file="Transport.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2022 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;

namespace QTIL.HostTools.Common.Transport
{
    /// <summary>
    /// Simple class encapsulating device name and transport options
    /// </summary>
    public class Transport
    {
        private string mPort;

        /// <summary>
        /// Gets/Sets the port.
        /// </summary>
        public string Port
        {
            get
            {
                return mPort;
            }
            set
            {
                mPort = value;
            }
        }

        private string mTrans;

        /// <summary>
        /// Gets/Sets the trans.
        /// </summary>
        public string Trans
        {
            get
            {
                return mTrans;
            }
            set
            {
                mTrans = value;
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is remote SPI.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is remote SPI; otherwise, <c>false</c>.
        /// </value>
        public bool IsRemoteSPI
        {
            get
            {
                return string.Equals(mPort, "remote", StringComparison.OrdinalIgnoreCase);
            }
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString()
        {
            return mPort;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Transport"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="trans">The trans.</param>
        public Transport(string name, string trans)
        {
            mPort = name;
            mTrans = trans;
        }

    }
}
