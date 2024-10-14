//------------------------------------------------------------------------------
//
// <copyright file="AssemblyInfo.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2018-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary></summary>
//
//------------------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using QTIL.HostTools.Common.VersionInfo;
using System.Resources;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("HidDfu Application")]
[assembly: AssemblyDescription("GUI tool for performing Device Firmware Upgrade over HID")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany(GlobalVersionInfo.VERSION_SHORT_COMPANY_NAME)]
[assembly: AssemblyProduct(DynamicVersionInfo.VERSION_PRODUCT)]
[assembly: AssemblyCopyright(GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_PREFIX + "2018" + GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_SUFFIX)]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("e5fa3a6c-d081-473f-a18e-36727bc0dd27")]

// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
[assembly: AssemblyVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]
[assembly: AssemblyFileVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]

// AssemblyInformationalVersion is shown as the "Product Version"
[assembly: AssemblyInformationalVersion(DynamicVersionInfo.VERSION_SPECIAL_BUILD)]
[assembly: NeutralResourcesLanguageAttribute("en-GB")]
