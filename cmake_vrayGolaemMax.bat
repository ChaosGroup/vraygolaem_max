@echo off

setlocal

set LAUNCHER_LOCATION=%~dp0

set MAYA_VERSION=2017
set MAX_VERSION=2018
set TOOLSET=-T v140

if %MAX_VERSION% LSS 2018 set TOOLSET=-T v110

"C:/Program Files/CMake/bin/cmake.exe" ^
-G"Visual Studio 15 2017 Win64" %TOOLSET% %* ^
-DCMAKE_INSTALL_PREFIX="C:/install/golaem" ^
-DGOLAEMDEVKIT_ROOTDIR="C:/programs/golaem/Maya/%MAYA_VERSION%/6.2.3/devkit" ^
-D3DSMAXSDK_VERSION=%MAX_VERSION% ^
-D3DSMAXSDK_ROOTDIR="C:/Program Files/Autodesk/3ds Max %MAX_VERSION%" ^
-D3DSMAXSDK_INCDIR="%KDRIVE2%/3dsmax/maxsdk%MAX_VERSION%/include" ^
-D3DSMAXSDK_LIBDIR="%KDRIVE2%/3dsmax/maxsdk%MAX_VERSION%/lib/x64" ^
-DVRAYFOR3DSMAX_ROOTDIR="C:/Program Files/Chaos Group/V-Ray/3dsmax %MAX_VERSION% for x64" ^
"%LAUNCHER_LOCATION%"

endlocal
