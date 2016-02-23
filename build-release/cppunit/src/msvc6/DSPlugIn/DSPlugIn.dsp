# Microsoft Developer Studio Project File - Name="DSPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DSPlugIn - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DSPlugIn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DSPlugIn.mak" CFG="DSPlugIn - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DSPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSPlugIn - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSPlugIn - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "../../../../lib" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"Release/TestRunnerDSPlugIn.dll"
# Begin Custom Build - Performing Registration
OutDir=.\Release
TargetPath=.\Release\TestRunnerDSPlugIn.dll
InputPath=.\Release\TestRunnerDSPlugIn.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rem echo Automatically done when the add-in is registered with VC++ 
	rem regsvr32 "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	rem echo Server registration done! 
	
# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=duplicating DLL to lib directory
PostBuild_Cmds=echo The following command may fail if you have already registered the add-in	copy Release\TestRunnerDSPlugIn.dll ..\..\..\lib\TestRunnerDSPlugIn.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "../../../../lib" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/TestRunnerDSPlugInD.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Performing Registration
OutDir=.\Debug
TargetPath=.\Debug\TestRunnerDSPlugInD.dll
InputPath=.\Debug\TestRunnerDSPlugInD.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rem echo Automatically done when the add-in is registered with VC++ 
	rem regsvr32 "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	rem echo Server registration done! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DSPlugIn___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "DSPlugIn___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUnicode"
# PROP Intermediate_Dir "ReleaseUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "WIN32" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /i "../../../../lib" /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "../../../../lib" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"Release/TestRunnerDSPlugIn.dll"
# ADD LINK32 /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /dll /machine:I386 /out:"ReleaseUnicode/TestRunnerDSPlugIn.dll"
# Begin Custom Build - Performing Registration
OutDir=.\ReleaseUnicode
TargetPath=.\ReleaseUnicode\TestRunnerDSPlugIn.dll
InputPath=.\ReleaseUnicode\TestRunnerDSPlugIn.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rem echo Automatically done when the add-in is registered with VC++ 
	rem regsvr32 "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	rem echo Server registration done! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DSPlugIn___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "DSPlugIn___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugUnicode"
# PROP Intermediate_Dir "DebugUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "_DEBUG" /D "_MBCS" /D "_USRDLL" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "WIN32" /D "_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /i "../../../../lib" /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "../../../../lib" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/TestRunnerDSPlugInD.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"DebugUnicode/TestRunnerDSPlugInD.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Performing Registration
OutDir=.\DebugUnicode
TargetPath=.\DebugUnicode\TestRunnerDSPlugInD.dll
InputPath=.\DebugUnicode\TestRunnerDSPlugInD.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rem echo Automatically done when the add-in is registered with VC++ 
	rem regsvr32 "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	rem echo Server registration done! 
	
# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=duplicating DLL to lib directory
PostBuild_Cmds=echo The following command may fail if you have already registered the add-in	copy Debug\TestRunnerDSPlugInD.dll ..\..\..\lib\TestRunnerDSPlugInD.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DSPlugIn - Win32 Release"
# Name "DSPlugIn - Win32 Debug"
# Name "DSPlugIn - Win32 Release Unicode"
# Name "DSPlugIn - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DSAddIn.cpp

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSPlugIn.cpp

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSPlugIn.def

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSPlugIn.rc

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1
# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1
# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestRunnerDSPlugin.idl

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

# ADD MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1
# ADD MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c" /Oicf

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1
# ADD BASE MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c"
# ADD MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1
# ADD BASE MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c" /Oicf
# ADD MTL /tlb "TestRunnerDSPlugin.tlb" /h "ToAddToDistribution/TestRunnerDSPluginVC6.h" /iid "ToAddToDistribution/TestRunnerDSPluginVC6_i.c" /Oicf

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ToAddToDistribution\TestRunnerDSPluginVC6_i.c

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\COMHelper.h

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSAddIn.h

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DSPlugIn.h

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Resource.h

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.h

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\DSPlugIn.rc2
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\DSPlugIn.rgs

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\TBarLrge.bmp

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\TBarMedm.bmp

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestRunnerDSPlugin.tlb

!IF  "$(CFG)" == "DSPlugIn - Win32 Release"

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Release Unicode"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DSPlugIn - Win32 Debug Unicode"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
