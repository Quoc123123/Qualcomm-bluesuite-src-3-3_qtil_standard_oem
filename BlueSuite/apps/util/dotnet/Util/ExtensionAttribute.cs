//------------------------------------------------------------------------------
//
// <copyright file="ExtensionAttribute.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2017 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

﻿// Extend System.Runtime.CompilerServices
namespace System.Runtime.CompilerServices
{
    /// <summary>
    /// Trivial (attribute) class to allow extension methods to be written using .NET Framework 2.0
    /// </summary>
    [AttributeUsage(AttributeTargets.Assembly | AttributeTargets.Class | AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public sealed class ExtensionAttribute
        : Attribute
    {
    }
}
