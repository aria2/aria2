# Microsoft Developer Studio Project File - Name="TestPlugInRunner" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TestPlugInRunner - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TestPlugInRunner.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestPlugInRunner.mak" CFG="TestPlugInRunner - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestPlugInRunner - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TestPlugInRunner - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TestPlugInRunner - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "../../include" /I "../TestRunner" /I "..\..\..\include" /I "..\..\..\include\msvc6" /I "..\TestRunner" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D "CPPUNIT_SUBCLASSING_TESTRUNNERDLG_BUILD" /D "CPPUNIT_DLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 cppunit_dll.lib winmm.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../../lib/"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\Release\TestPlugInRunner.exe
TargetName=TestPlugInRunner
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\..\lib\$(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TestPlugInRunner - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\include" /I "..\..\..\include\msvc6" /I "..\TestRunner" /D "_DEBUG" /D "CPPUNIT_TESTPLUGINRUNNER_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D "CPPUNIT_SUBCLASSING_TESTRUNNERDLG_BUILD" /D "CPPUNIT_DLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppunitd_dll.lib winmm.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Debug/TestPlugInRunnerd.exe" /pdbtype:sept /libpath:"../../../lib/"
# Begin Special Build Tool
TargetPath=.\Debug\TestPlugInRunnerd.exe
TargetName=TestPlugInRunnerd
SOURCE="$(InputPath)"
PostBuild_Desc=Copying target to lib/
PostBuild_Cmds=copy "$(TargetPath)" ..\..\..\lib\$(TargetName).exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "TestPlugInRunner - Win32 Release"
# Name "TestPlugInRunner - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\errortype.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=..\testrunner\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=..\testrunner\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=..\testrunner\res\idr_test.ico
# End Source File
# Begin Source File

SOURCE=.\res\test_type.bmp
# End Source File
# Begin Source File

SOURCE=..\testrunner\res\test_type.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TestPlugInRunner.ico
# End Source File
# Begin Source File

SOURCE=.\res\TestPlugInRunner.rc2
# End Source File
# Begin Source File

SOURCE=..\testrunner\res\tfwkui_r.bmp
# End Source File
# End Group
# Begin Group "Gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunner.rc
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerApp.h
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerDlg.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\include\msvc6\testrunner\TestPlugInInterface.h
# End Source File
# End Group
# Begin Group "Models"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TestPlugIn.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPlugIn.h
# End Source File
# Begin Source File

SOURCE=.\TestPlugInException.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPlugInException.h
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerModel.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPlugInRunnerModel.h
# End Source File
# End Group
# Begin Group "DLL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\lib\cppunit_dll.dll

!IF  "$(CFG)" == "TestPlugInRunner - Win32 Release"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\Release
InputPath=..\..\..\lib\cppunit_dll.dll
InputName=cppunit_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "TestPlugInRunner - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\cppunitd_dll.dll

!IF  "$(CFG)" == "TestPlugInRunner - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TestPlugInRunner - Win32 Debug"

# Begin Custom Build - Updating $(InputPath)
IntDir=.\Debug
InputPath=..\..\..\lib\cppunitd_dll.dll
InputName=cppunitd_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunner.dll
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunnerd.dll
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "TestRunner-Was-In-Dll"

# PROP Default_Filter ""
# Begin Group "UserInterface"

# PROP Default_Filter ""
# Begin Group "DynamicWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicBar.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicBar.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicControlsManager.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicControlsManager.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicDialog.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicFormView.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicFormView.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicPropSheet.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicPropSheet.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicWnd.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicWndEx.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCDynamicWndEx.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCSizeIconCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\cdxCSizeIconCtrl.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\SizeCBar.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\testrunner\DynamicWindow\SizeCBar.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\testrunner\ListCtrlFormatter.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\ListCtrlFormatter.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\ListCtrlSetter.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\ListCtrlSetter.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\MsDevCallerListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\MsDevCallerListCtrl.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\ProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\ProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\ResourceLoaders.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\ResourceLoaders.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\TestRunnerDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\TestRunnerDlg.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\TreeHierarchyDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\TreeHierarchyDlg.h
# End Source File
# End Group
# Begin Group "Components"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\testrunner\ActiveTest.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\ActiveTest.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\MfcSynchronizationObject.h
# End Source File
# Begin Source File

SOURCE=..\testrunner\TestRunnerModel.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\TestRunnerModel.h
# End Source File
# End Group
# Begin Group "NewFiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\testrunner\MostRecentTests.cpp
# End Source File
# Begin Source File

SOURCE=..\testrunner\MostRecentTests.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\msvc6\DSPlugin\TestRunnerDSPluginVC6_i.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
