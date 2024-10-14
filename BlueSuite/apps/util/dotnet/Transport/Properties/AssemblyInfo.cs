using System;
using System.Reflection;
using System.Runtime.InteropServices;

using QTIL.HostTools.Common.VersionInfo;
using System.Resources;

[assembly: AssemblyTitle("QTIL.HostTools.Common.Transport")]
[assembly: AssemblyDescription("Provides base classes for Transport settings")]
[assembly: AssemblyCompany(GlobalVersionInfo.VERSION_SHORT_COMPANY_NAME)]
[assembly: AssemblyProduct(DynamicVersionInfo.VERSION_PRODUCT)]
[assembly: AssemblyCopyright(GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_PREFIX + "2011" + GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_SUFFIX)]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

[assembly: CLSCompliant(false)]

[assembly: ComVisible(false)]
// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("06dd919a-a3a5-4a7e-bb72-b4f9cac53424")]

[assembly: AssemblyVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]
[assembly: AssemblyFileVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]

// AssemblyInformationalVersion is shown as the "Product Version"
[assembly: AssemblyInformationalVersion(DynamicVersionInfo.VERSION_SPECIAL_BUILD)]
[assembly: NeutralResourcesLanguageAttribute("en-GB")]
