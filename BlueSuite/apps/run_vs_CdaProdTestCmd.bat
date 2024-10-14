set HOSTBUILD_RESULT=%~dp0..\result
set TOP_COMMON_HOSTTOOLS=%~dp0
set TOP_HOSTTOOLS=%~dp0
rem Remove the sharing violation error code if MSDEV was open when the copy took place...
ver
start "" "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\IDE\devenv.exe" CdaProdTestCmd.sln
exit

