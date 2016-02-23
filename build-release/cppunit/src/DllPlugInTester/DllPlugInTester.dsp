# Microsoft Developer Studio Project File - Name="DllPlugInTester" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DllPlugInTester - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DllPlugInTester.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DllPlugInTester.mak" CFG="DllPlugInTester - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DllPlugInTester - Win32 Release Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTester - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTester - Win32 Release Static" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTester - Win32 Debug Static" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTester - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DllPlugInTester - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DllPlugInTester - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DllPlugInTester___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUnicode"
# PROP Intermediate_Dir "ReleaseUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /out:"ReleaseUnicode\DllPlugInTesteru.exe" /libpath:"../../lib"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\ReleaseUnicode\DllPlugInTesteru.exe
TargetName=DllPlugInTesteru
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTester - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DllPlugInTester___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugUnicode"
# PROP Intermediate_Dir "DebugUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/DllPlugInTesterd.exe" /pdbtype:sept /libpath:"../../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"DebugUnicode\DllPlugInTesterud.exe" /pdbtype:sept /libpath:"../../lib"
# Begin Special Build Tool
TargetPath=.\DebugUnicode\DllPlugInTesterud.exe
TargetName=DllPlugInTesterud
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTester - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DllPlugInTester___Win32_Release_Static"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Release_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /out:"..\..\lib\DllPlugInTester.exe" /libpath:"../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\Release\DllPlugInTester.exe
TargetName=DllPlugInTester
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy $(TargetPath) ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTester - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DllPlugInTester___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Debug_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\lib\DllPlugInTesterd.exe" /pdbtype:sept /libpath:"../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"Debug\DllPlugInTesterd.exe" /pdbtype:sept /libpath:"../../lib"
# Begin Special Build Tool
TargetPath=.\Debug\DllPlugInTesterd.exe
TargetName=DllPlugInTesterd
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy $(TargetPath) ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTester - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DllPlugInTester___Win32_Release"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDll"
# PROP Intermediate_Dir "ReleaseDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /subsystem:console /machine:I386 /out:"..\..\lib\DllPlugInTester_dll.exe" /libpath:"../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /subsystem:console /machine:I386 /out:"ReleaseDll\DllPlugInTester_dll.exe" /libpath:"../../lib"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\ReleaseDll\DllPlugInTester_dll.exe
TargetName=DllPlugInTester_dll
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DllPlugInTester - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DllPlugInTester___Win32_Debug"
# PROP BASE Intermediate_Dir "DllPlugInTester___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDll"
# PROP Intermediate_Dir "DebugDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\lib\DllPlugInTesterd_dll.exe" /pdbtype:sept /libpath:"../../lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"DebugDll\DllPlugInTesterd_dll.exe" /pdbtype:sept /libpath:"../../lib"
# Begin Special Build Tool
TargetPath=.\DebugDll\DllPlugInTesterd_dll.exe
TargetName=DllPlugInTesterd_dll
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\lib\$(TargetName).exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DllPlugInTester - Win32 Release Unicode"
# Name "DllPlugInTester - Win32 Debug Unicode"
# Name "DllPlugInTester - Win32 Release Static"
# Name "DllPlugInTester - Win32 Debug Static"
# Name "DllPlugInTester - Win32 Release"
# Name "DllPlugInTester - Win32 Debug"
# Begin Source File

SOURCE=.\CommandLineParser.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandLineParser.h
# End Source File
# Begin Source File

SOURCE=.\DllPlugInTester.cpp
# End Source File
# Begin Source File

SOURCE=.\Makefile.am
# End Source File
# End Target
# End Project
