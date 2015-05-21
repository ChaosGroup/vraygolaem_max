# Microsoft Developer Studio Project File - Name="vraygolaem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vraygolaem - Win32 Max Release Official
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vraygolaem.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vraygolaem.mak" CFG="vraygolaem - Win32 Max Release Official"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vraygolaem - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vraygolaem - Win32 Max Release Official" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vraygolaem - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vraygolaem___Win32_Release_Max_6"
# PROP BASE Intermediate_Dir "vraygolaem___Win32_Release_Max_6"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "t:\build\vraygolaem\debug\x86"
# PROP Intermediate_Dir "t:\build\vraygolaem\debug\x86"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "\SDK\MaxSDK50\Include" /I "..\..\include\vray" /I "..\..\include\vutils" /I "..\..\include\rayserver" /I "..\..\include\resman" /I "..\..\include\imagesamplers" /I "..\..\include\imagefilters" /I "..\..\include\plugman" /I "..\..\include\putils" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /Qwd1125,880,1420 /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "k:\3dsmax\maxsdk60\Include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /Qwd1125,880,1420 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 vray50.lib bmm.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib core.lib geom.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\plugins\vraygolaem50.dlo" /libpath:"\SDK\MaxSDK50\Lib" /libpath:"..\..\lib"
# ADD LINK32 vrender60.lib plugman_s.lib vray60.lib bmm.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib core.lib geom.lib comctl32.lib uuid.lib odbc32.lib odbccp32.lib vrender60.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib putils_s.lib rayserver_s.lib vutils_s.lib /nologo /subsystem:windows /dll /machine:I386 /out:"w:\program files\max80\plugins\vrayplugins\vraygolaem60.dlo" /libpath:"k:\3dsmax\maxsdk60\Lib" /libpath:"..\..\..\lib\x86" /libpath:"..\..\..\lib\x86\vc6"

!ELSEIF  "$(CFG)" == "vraygolaem - Win32 Max Release Official"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vraygolaem___Win32_Release_Max_6_Official"
# PROP BASE Intermediate_Dir "vraygolaem___Win32_Release_Max_6_Official"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "t:\build\vraygolaem\max60\x86\official"
# PROP Intermediate_Dir "t:\build\vraygolaem\max60\x86\official"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "\SDK\MaxSDK60\Include" /I "..\..\include\vray" /I "..\..\include\vutils" /I "..\..\include\rayserver" /I "..\..\include\resman" /I "..\..\include\imagesamplers" /I "..\..\include\imagefilters" /I "..\..\include\plugman" /I "..\..\include\putils" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /Qwd1125,880,1420 /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "k:\3dsmax\maxsdk60\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_VFB_" /YX /FD /Qwd1125,880,1420 /O3 /Qxi /Qprec_div /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 vray60.lib bmm.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib core.lib geom.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\plugins\vraygolaem60.dlo" /libpath:"\SDK\MaxSDK60\Lib" /libpath:"..\..\lib"
# ADD LINK32 vrender60.lib vray60.lib bmm.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib core.lib geom.lib comctl32.lib uuid.lib odbc32.lib odbccp32.lib plugman_s.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib putils_s.lib vutils_s.lib /subsystem:windows /dll /pdb:"o:\vray\1.0\currentbuild\3dsmax\x86\adv\vraygolaem60.pdb" /debug /machine:I386 /def:".\plugin.def" /out:"o:\vray\1.0\currentbuild\3dsmax\x86\adv\vraygolaem60.dlo" /libpath:"k:\3dsmax\maxsdk60\Lib\x86" /libpath:"..\..\..\lib\x86" /libpath:"..\..\..\lib\x86\vc6"
# SUBTRACT LINK32 /nologo /pdb:none

!ENDIF 

# Begin Target

# Name "vraygolaem - Win32 Debug"
# Name "vraygolaem - Win32 Max Release Official"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\instance.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.def
# End Source File
# Begin Source File

SOURCE=.\vraygolaem.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\vraygolaem.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
