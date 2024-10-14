//------------------------------------------------------------------------------
//
// <copyright file="NamedValue.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Globalization;

using QTIL.HostTools.Common.EngineFrameworkClr;

// Extend System
namespace System
{
    /// <summary>
    /// 
    /// </summary>
    public class NamedValue<T>
    {
        private readonly String mName;

        /// <summary>
        /// Gets the name.
        /// </summary>
        public String Name
        {
            get
            {
                return mName;
            }
        }

        private readonly T mValue;

        /// <summary>
        /// Gets the value.
        /// </summary>
        public T Value
        {
            get
            {
                return mValue;
            }
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override String ToString()
        {
            MessageHandler.DebugEntry();

            String retVal = String.Format(CultureInfo.CurrentCulture, "{0} ({1})", mName, mValue);

            MessageHandler.DebugExit(retVal);

            return retVal;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="NamedValue"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="value">The value.</param>
        public NamedValue(String name, T value)
        {
            MessageHandler.DebugEntry();

            mName = name;
            mValue = value;

            MessageHandler.DebugExit();
        }

        /// <summary>
        /// Prevents a default instance of the <see cref="NamedValue&lt;T&gt;"/> class from being created.
        /// </summary>
        private NamedValue()
        {            
        }

    }
}
