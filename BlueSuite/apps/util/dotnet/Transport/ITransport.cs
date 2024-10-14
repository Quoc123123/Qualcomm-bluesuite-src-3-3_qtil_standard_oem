//------------------------------------------------------------------------------
//
// <copyright file="ITransport.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2011-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Net;

namespace QTIL.HostTools.Common.Transport
{
    /// <summary>
    /// Defines the transport interface.
    /// </summary>
    [CLSCompliant(false)]
    public interface ITransport
    {
        /// <summary>
        /// Gets the baud rate.
        /// </summary>
        uint BaudRate { get;}

        /// <summary>
        /// Gets the IP address.
        /// </summary>
        IPAddress IPAddress { get;}

        /// <summary>
        /// Gets the port.
        /// </summary>
        String Port { get; }

        /// <summary>
        /// Gets the trans.
        /// </summary>
        String Trans { get; }

        /// <summary>
        /// Gets the type.
        /// </summary>
        TransportTypes Type { get;}
    }
}
