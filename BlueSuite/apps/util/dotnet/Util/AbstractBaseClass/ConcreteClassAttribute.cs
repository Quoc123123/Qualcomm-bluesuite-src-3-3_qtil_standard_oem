//------------------------------------------------------------------------------
//
// <copyright file="ConcreteClassAttribute.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2012-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;

// Extend System
namespace System
{
    /// <summary>
    /// Place this attribute on any abstract class where you want to declare a concrete version of that class at design time.
    /// </summary>
    [
    AttributeUsage(AttributeTargets.Class)
    ]
    public sealed class ConcreteClassAttribute
        : Attribute
    {
        /// <summary>
        /// The concrete class type.
        /// </summary>
        private readonly Type mConcreteType;

        /// <summary>
        /// Gets the type of the concrete class.
        /// </summary>
        /// <value>
        /// The type of the concrete class.
        /// </value>
        public Type ConcreteType
        {
            get
            {
                return mConcreteType;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ConcreteClassAttribute"/> class.
        /// </summary>
        /// <param name="concreteType">Type of the concrete.</param>
        public ConcreteClassAttribute(Type concreteType)
        {
            mConcreteType = concreteType;
        }

    }
}
