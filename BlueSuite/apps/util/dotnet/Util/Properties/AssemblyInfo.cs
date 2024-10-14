using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using QTIL.HostTools.Common.VersionInfo;

[assembly: AssemblyTitle("QTIL.HostTools.Common.Utils")]
[assembly: AssemblyDescription("QTIL HostTools Common Utility library")]
[assembly: AssemblyCompany(GlobalVersionInfo.VERSION_SHORT_COMPANY_NAME)]
[assembly: AssemblyProduct(DynamicVersionInfo.VERSION_PRODUCT)]
[assembly: AssemblyCopyright(GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_PREFIX + "2011" + GlobalVersionInfo.VERSION_COPYRIGHT_START_YEAR_SUFFIX)]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

[assembly: CLSCompliant(true)]

[assembly: ComVisible(false)]
// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("6e92a7d0-feb3-4c0c-9623-82a35cf9449b")]

[assembly: AssemblyVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]
[assembly: AssemblyFileVersion(DynamicVersionInfo.VERSION_APP_FULL_STR)]

// AssemblyInformationalVersion is shown as the "Product Version"
[assembly: AssemblyInformationalVersion(DynamicVersionInfo.VERSION_SPECIAL_BUILD)]

// Let the unit tests see internals
[assembly: InternalsVisibleTo("QTIL.HostTools.Common.Util.UnitTests")]
