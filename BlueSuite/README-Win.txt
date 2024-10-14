
These instructions describe how to build and run the included BlueSuite 3.x
components for Windows.


Prerequisites
-------------------------------------------------------------------------------
1. Microsoft Visual Studio 2019 is required to build the applications.
    NOTE: .NET and Visual C++ support is required, and these are disabled in
    the Visual Studio 2019 installer by default. They must be enabled by
    selecting the following options under the "Workloads" tab when configuring
    the installation:
      ".NET desktop development"
      "Desktop development with C++"
    Then, under "Installation Details" on the RHS, open
    "Desktop development with C++" optional features and ensure that the
    following options are selected:
      "C++ MFC for latest v142 build tools (x86 and x64)"
      "C++/CLI support for v142 build tools (Latest)".
2. For HidDfu components, the Windows Driver Kit Version 7.1.0 must be
    installed. This is in order to get the Windows HID header and library
    files.
    NOTE: Ensure "Full Development Environment" is selected in the installation
    options. The installation path should be left at the default, as the
    default path for the 7.1.0 WDK is used by the HidDfu project. If a
    different path is chosen, the HidDfu project header and library paths will
    need to be modified accordingly. It is recommended that all other options
    are left at their defaults.
3. A compatible Qualcomm IC must be connected for the HidDfu tools to run
    successfully.
4. For SecurityCmd and CdaProdTestCmd, OpenSSL 1.1.1 (revision i or later) is
    required.
5. For the MultiProgCmd and CdaProdTestCmd production test applications, a
    compatible BlueSuite (binary) tools must be installed. This would generally
    be the same version as the source release package (except for the 4th
    version field).
6. In order to run the built binaries, Microsoft Visual Studio 2019 C++ runtime
    libraries are required. These are included in a Visual Studio 2019
    installation, but can also be deployed separately using the redistributable
    installers found at:
    https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads.
7. CdaProdTestCmd has additional prerequisites, see
    apps\ProdTest\CdaProdTestCmd\README.txt.


Preparation
-------------------------------------------------------------------------------
1. Unzip the source code package.
2. Obtain and build OpenSSL for the required platform configurations.
    See README-OpenSSL.txt for details.
3. If the MultiProgCmd and CdaProdTestCmd production test applications are to
    be built:
    a) Edit the populate-bluesuite-apis.bat file as necessary in order to set
       the locations of the BlueSuite binary installation and BlueSuite source
       tree (as per the instructions in the batch file).
    b) Run the batch file.


Building
-------------------------------------------------------------------------------
Release and Debug configurations are provided for both Win32 and x64 platforms.
Win32 built components will run on both 32-bit and 64-bit Windows.
x64 built components only run on 64-bit windows.

NOTE: The run_vs_*.bat files referenced below contain a hard-coded path to
Visual Studio 2019 (default installation location). The path must be changed
if Visual Studio 2019 is installed to a different location.

To build an application:
    1. Run the file apps\run_vs_<app_name>.bat to launch the solution in Visual
       Studio 2019.
    2. Build the required configuration.


Running the tools
-------------------------------------------------------------------------------
Binaries are built to the output folder:
    result\<x86win32|x86win64>\bin\<Debug|Release>
   
For the command-line tools (HidDfuCmd, SecurityCmd, MultiProgCmd and
CdaProdTestCmd), running the tool with the "-help" argument will show the usage
details.
For MultiProgCmd, see also apps\ProdTest\MultiProgCmd\README.txt.
For CdaProdTestCmd, see also apps\ProdTest\CdaProdTestCmd\README.txt.

For HidDfuApp, the file result\help\HidDfuTools.chm contains usage details.

NOTE: If debug builds of the MultiProgCmd and CdaProdTestCmd applications are
to be run subsequent to building debug versions of any of the other tools, the
result\<x86win32|x86win64>\bin\Debug folder can contain a mix of release DLLs
copied from a BlueSuite (binary) installation using the
populate-bluesuite-apis.bat script, and debug DLLs built from source as
dependencies of the other applications. This mix of release and debug DLLs
can cause failure to run. To resolve this, ensure that only the release builds
of the DLLs are present in result\<x86win32|x86win64>\bin\Debug by re-running
the populate-bluesuite-apis.bat script to copy the release DLLs from the
BlueSuite (binary) installation.
