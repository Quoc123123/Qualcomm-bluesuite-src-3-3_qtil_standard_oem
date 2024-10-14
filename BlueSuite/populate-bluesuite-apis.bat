@rem This batch file populates a BlueSuite source tree with the BlueSuite DLL APIs
@rem include and lib files.

@rem BS_BIN and BS_SRC will need editing to point to the correct directories.
@rem BS_BIN is the BlueSuite (binary) tools installation directory.
@rem BS_SRC is the BlueSuite source release directory to populate.
set "BS_BIN=C:\Program Files (x86)\QTIL\BlueSuite 3.3.16"
set "BS_SRC=C:\sourceReleaseBlueSuite3xWindows-1.2.3.4"
@echo off

if not exist "%BS_BIN%" (
    echo "%BS_BIN%" does not exist. Update BS_BIN to be your BlueSuite ^(binary^) tools installation directory.
    goto :EOF
)
if not exist "%BS_SRC%" (
    echo "%BS_SRC%" does not exist. Update BS_SRC to be your BlueSuite source code directory.
    goto :EOF
)

set "TARG_INC=%BS_SRC%\result\include"
set "TARG_LIB_32=%BS_SRC%\result\x86win32\lib"
set "TARG_LIB_64=%BS_SRC%\result\x86win64\lib"
set "TARG_BIN_32=%BS_SRC%\result\x86win32\bin"
set "TARG_BIN_64=%BS_SRC%\result\x86win64\bin"

rem include files
xcopy "%BS_BIN%\include\TestFlash.h" "%TARG_INC%\spi\" /Y /I /F
xcopy "%BS_BIN%\include\TestEngine.h" "%TARG_INC%\hci\" /Y /I /F

rem import libraries
for %%f in (TestFlash.lib, TestEngine.lib) do (
    for %%i in (Debug, Release) do (
        xcopy "%BS_BIN%\lib\%%f" "%TARG_LIB_32%\%%i\" /Y /I /F
        xcopy "%BS_BIN%\x64\lib\%%f" "%TARG_LIB_64%\%%i\" /Y /I /F
    )
)

rem binaries
for %%f in (CoreFramework.dll, CuratorLibrary.dll, DataRecorder.dll, DevTrbTrans.dll,
    EngineFrameworkCpp.dll, Flash.dll, HydBinary.hpm, HydIsp.hpm, HydProtocols.dll,
    hydra_user.dll, HydText.hpm, PsHelp.dll, PtapLibrary.dll, PtToolCmd.dll, PtTransport.dll,
    PtTrb.dll, ptusbspi.dll, SecurLib.dll, SQLiteMetadataProvider.hpm, TestEngine.dll,
    TestFlash.dll, Transports.dll
) do (
    for %%i in (Debug, Release) do (
        xcopy "%BS_BIN%\%%f" "%TARG_BIN_32%\%%i\" /Y /I /F
        xcopy "%BS_BIN%\x64\%%f" "%TARG_BIN_64%\%%i\" /Y /I /F
    )
)
