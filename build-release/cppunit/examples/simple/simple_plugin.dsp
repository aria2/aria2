# Microsoft Developer Studio Project File - Name="simple_plugin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=simple_plugin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "simple_plugin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple_plugin.mak" CFG="simple_plugin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simple_plugin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple_plugin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simple_plugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "simple_plugin___Win32_Release"
# PROP BASE Intermediate_Dir "simple_plugin___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleasePlugIn"
# PROP Intermediate_Dir "ReleasePlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_PLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_PLUGIN_EXPORTS" /D "CPPUNIT_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /dll /machine:I386 /libpath:"../../lib/"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\ReleasePlugIn\simple_plugin.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Running tests...
PostBuild_Cmds=..\..\lib\DllPlugInTester_dll.exe "$(TargetPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "simple_plugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "simple_plugin___Win32_Debug"
# PROP BASE Intermediate_Dir "simple_plugin___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugPlugIn"
# PROP Intermediate_Dir "DebugPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_PLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_PLUGIN_EXPORTS" /D "CPPUNIT_DLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"DebugPlugIn/simple_plugind.dll" /pdbtype:sept /libpath:"../../lib/"
# Begin Special Build Tool
TargetPath=.\DebugPlugIn\simple_plugind.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Running tests...
PostBuild_Cmds=..\..\lib\DllPlugInTesterd_dll.exe -b --xml tests.xml -c "$(TargetPath)"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "simple_plugin - Win32 Release"
# Name "simple_plugin - Win32 Debug"
# Begin Source File

SOURCE=.\ExampleTestCase.cpp
# End Source File
# Begin Source File

SOURCE=.\ExampleTestCase.h
# End Source File
# Begin Source File

SOURCE=.\SimplePlugIn.cpp
# End Source File
# End Target
# End Project
