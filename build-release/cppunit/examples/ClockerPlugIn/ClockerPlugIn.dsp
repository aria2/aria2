# Microsoft Developer Studio Project File - Name="ClockerPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ClockerPlugIn - Win32 Debug NtTimer
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClockerPlugIn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClockerPlugIn.mak" CFG="ClockerPlugIn - Win32 Debug NtTimer"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClockerPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClockerPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClockerPlugIn - Win32 Debug NtTimer" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClockerPlugIn - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLOCKERPLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNIT_DLL" /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /dll /machine:I386 /out:"../../lib/ClockerPlugIn.dll" /libpath:"../../lib/"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLOCKERPLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNIT_DLL" /FD /GZ /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /dll /debug /machine:I386 /out:"../../lib/ClockerPlugInd.dll" /pdbtype:sept /libpath:"../../lib/"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug NtTimer"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug NtTimer"
# PROP BASE Intermediate_Dir "Debug NtTimer"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugNtTimer"
# PROP Intermediate_Dir "DebugNtTimer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNIT_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNIT_DLL" /D "CLOCKER_USE_WINNTTIMER" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /dll /debug /machine:I386 /out:"../../lib/ClockerPlugInd.dll" /pdbtype:sept /libpath:"../../lib/"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /dll /debug /machine:I386 /out:"../../lib/ClockerPlugInNtd.dll" /pdbtype:sept /libpath:"../../lib/"

!ENDIF 

# Begin Target

# Name "ClockerPlugIn - Win32 Release"
# Name "ClockerPlugIn - Win32 Debug"
# Name "ClockerPlugIn - Win32 Debug NtTimer"
# Begin Source File

SOURCE=.\ClockerListener.cpp
# End Source File
# Begin Source File

SOURCE=.\ClockerListener.h
# End Source File
# Begin Source File

SOURCE=.\ClockerModel.cpp
# End Source File
# Begin Source File

SOURCE=.\ClockerModel.h
# End Source File
# Begin Source File

SOURCE=.\ClockerPlugIn.cpp
# End Source File
# Begin Source File

SOURCE=.\ClockerXmlHook.cpp
# End Source File
# Begin Source File

SOURCE=.\ClockerXmlHook.h
# End Source File
# Begin Source File

SOURCE=.\Makefile.am
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\Timer.cpp

!IF  "$(CFG)" == "ClockerPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug NtTimer"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Timer.h

!IF  "$(CFG)" == "ClockerPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug NtTimer"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinNtTimer.cpp

!IF  "$(CFG)" == "ClockerPlugIn - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug NtTimer"

# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "DebugNtTimer"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinNtTimer.h

!IF  "$(CFG)" == "ClockerPlugIn - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClockerPlugIn - Win32 Debug NtTimer"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
