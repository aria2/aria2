# Microsoft Developer Studio Project File - Name="DllPlugInTesterTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DllPlugInTesterTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DllPlugInTesterTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DllPlugInTesterTest.mak" CFG="DllPlugInTesterTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DllPlugInTesterTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTesterTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DllPlugInTesterTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseTest"
# PROP Intermediate_Dir "ReleaseTest"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /Ox /Ot /Oa /Ow /Og /Oi /Ob0 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\ReleaseTest\DllPlugInTesterTest.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Unit testing...
PostBuild_Cmds=$(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTesterTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugTest"
# PROP Intermediate_Dir "DebugTest"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"../../lib"
# Begin Special Build Tool
TargetPath=.\DebugTest\DllPlugInTesterTest.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Unit testing...
PostBuild_Cmds=$(TargetPath)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DllPlugInTesterTest - Win32 Release"
# Name "DllPlugInTesterTest - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\DllPlugInTester\CommandLineParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DllPlugInTester\CommandLineParser.h
# End Source File
# Begin Source File

SOURCE=.\CommandLineParserTest.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandLineParserTest.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\cppunit_dll.dll

!IF  "$(CFG)" == "DllPlugInTesterTest - Win32 Release"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\ReleaseTest
InputPath=..\..\lib\cppunit_dll.dll
InputName=cppunit_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "DllPlugInTesterTest - Win32 Debug"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\DebugTest
InputPath=..\..\lib\cppunit_dll.dll
InputName=cppunit_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\lib\cppunitd_dll.dll

!IF  "$(CFG)" == "DllPlugInTesterTest - Win32 Release"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\ReleaseTest
InputPath=..\..\lib\cppunitd_dll.dll
InputName=cppunitd_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "DllPlugInTesterTest - Win32 Debug"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\DebugTest
InputPath=..\..\lib\cppunitd_dll.dll
InputName=cppunitd_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\DllPlugInTester\DllPlugInTesterTest.cpp
# End Source File
# End Target
# End Project
