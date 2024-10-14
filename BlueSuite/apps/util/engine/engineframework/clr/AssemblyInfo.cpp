//------------------------------------------------------------------------------
//
// <copyright file="AssemblyInfo.cs" company="Qualcomm Technologies International, Ltd.">
// Copyright (c) 2013-2019 Qualcomm Technologies International, Ltd.
// All Rights Reserved.
// Qualcomm Technologies International, Ltd. Confidential and Proprietary.
// </copyright>
//
// <summary>Contains information about assembly</summary>
//
//------------------------------------------------------------------------------

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

#include "common\globalversioninfo.h"

[assembly:AssemblyTitleAttribute("EngineFrameworkClr")];
[assembly:AssemblyDescriptionAttribute("CLR wrapper around EngineFramework")];
[assembly:AssemblyCompanyAttribute(VERSION_SHORT_COMPANY_NAME)];
[assembly:AssemblyProductAttribute("EngineFrameworkClr")];
[assembly:AssemblyCopyrightAttribute(VERSION_COPYRIGHT_START_YEAR("2013"))];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

[assembly:ComVisible(false)];
[assembly:CLSCompliantAttribute(true)];


[assembly:AssemblyVersionAttribute(VERSION_APP_FULL_STR)];
[assembly:AssemblyFileVersion(VERSION_APP_FULL_STR)];

// AssemblyInformationalVersion is shown as the "Product Version"
[assembly:AssemblyInformationalVersion(VERSION_SPECIAL_BUILD)];