using System;
using System.Reflection;
using System.Runtime.InteropServices;

using QTIL.HostTools.Common.VersionInfo;
using System.Resources;

[assembly: AssemblyTitle("QTIL.HostTools.Common.Dialogs")]
[assembly: AssemblyDescription("QTIL HostTools common dialogs")]
[assembly: AssemblyCompany(GlobalVersionInfo.VERSION_SHORT_COMPANY_NAME)]
[assembly: AssemblyProduct(DynamicVersionInfo.VERSION_PRODUCT)]
[assembly: AssemblyCopyright(GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_PREFIX + "2011" + GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_SUFFIX)]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]
// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("0a078e33-f132-499f-88eb-8bf75439c350")]

[assembly: AssemblyVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]
[assembly: AssemblyFileVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]

// AssemblyInformationalVersion is shown as the "Product Version"
[assembly: AssemblyInformationalVersion(DynamicVersionInfo.VERSION_SPECIAL_BUILD)]
[assembly: NeutralResourcesLanguageAttribute("en-GB")]
