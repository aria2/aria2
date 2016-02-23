# Microsoft Developer Studio Project File - Name="HostApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=HostApp - Win32 Debug No Type Info Name
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HostApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HostApp.mak" CFG="HostApp - Win32 Debug No Type Info Name"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HostApp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "HostApp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "HostApp - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "HostApp - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "HostApp - Win32 Debug No Type Info Name" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HostApp - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ../../../Lib/cppunit.lib ../../../Lib/testrunner.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\..\lib\cppunitd.lib ..\..\..\lib\testrunnerd.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib

!ELSEIF  "$(CFG)" == "HostApp - Win32 Release Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "HostApp___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "HostApp___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUnicode"
# PROP Intermediate_Dir "ReleaseUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../../../Lib/cppunit.lib ../../../Lib/testrunner.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ../../../Lib/cppunit.lib ../../../Lib/testrunneru.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug Unicode"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "HostApp___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "HostApp___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugUnicode"
# PROP Intermediate_Dir "DebugUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\..\..\lib\cppunitd.lib ..\..\..\lib\testrunnerd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /map /nodefaultlib
# ADD LINK32 ..\..\..\lib\cppunitd.lib ..\..\..\lib\testrunnerud.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug No Type Info Name"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "HostApp___Win32_Debug_No_Type_Info_Name"
# PROP BASE Intermediate_Dir "HostApp___Win32_Debug_No_Type_Info_Name"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugNoTypeInfoName"
# PROP Intermediate_Dir "DebugNoTypeInfoName"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D CPPUNIT_USE_TYPEINFO_NAME=0 /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\..\..\lib\cppunitd.lib ..\..\..\lib\testrunnerd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /map /nodefaultlib
# ADD LINK32 ..\..\..\lib\cppunitd.lib ..\..\..\lib\testrunnerd.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib

!ENDIF 

# Begin Target

# Name "HostApp - Win32 Release"
# Name "HostApp - Win32 Debug"
# Name "HostApp - Win32 Release Unicode"
# Name "HostApp - Win32 Debug Unicode"
# Name "HostApp - Win32 Debug No Type Info Name"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ExampleTestCase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\HostApp.cpp
# End Source File
# Begin Source File

SOURCE=.\HostApp.rc
# End Source File
# Begin Source File

SOURCE=.\HostAppDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\HostAppView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ExampleTestCase.h
# End Source File
# Begin Source File

SOURCE=.\HostApp.h
# End Source File
# Begin Source File

SOURCE=.\HostAppDoc.h
# End Source File
# Begin Source File

SOURCE=.\HostAppView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\HostApp.ico
# End Source File
# Begin Source File

SOURCE=.\res\HostApp.rc2
# End Source File
# Begin Source File

SOURCE=.\res\HostAppDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "DLL Dependencies"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\lib\testrunner.dll

!IF  "$(CFG)" == "HostApp - Win32 Release"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\Release
InputPath=..\..\..\lib\testrunner.dll
InputName=testrunner

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Release Unicode"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\ReleaseUnicode
InputPath=..\..\..\lib\testrunner.dll
InputName=testrunner

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug Unicode"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug No Type Info Name"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunnerd.dll

!IF  "$(CFG)" == "HostApp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\Debug
InputPath=..\..\..\lib\testrunnerd.dll
InputName=testrunnerd

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Release Unicode"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug Unicode"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\DebugUnicode
InputPath=..\..\..\lib\testrunnerd.dll
InputName=testrunnerd

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug No Type Info Name"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\DebugNoTypeInfoName
InputPath=..\..\..\lib\testrunnerd.dll
InputName=testrunnerd

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunneru.dll

!IF  "$(CFG)" == "HostApp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Release Unicode"

# Begin Custom Build - Updating DLL $(InputPath)
IntDir=.\ReleaseUnicode
InputPath=..\..\..\lib\testrunneru.dll
InputName=testrunneru

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug No Type Info Name"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunnerud.dll

!IF  "$(CFG)" == "HostApp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug Unicode"

# Begin Custom Build - Updating DLL $(InputPath)
IntDir=.\DebugUnicode
InputPath=..\..\..\lib\testrunnerud.dll
InputName=testrunnerud

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "HostApp - Win32 Debug No Type Info Name"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
