# Microsoft Developer Studio Project File - Name="CppUnitTestApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CppUnitTestApp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestApp.mak" CFG="CppUnitTestApp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CppUnitTestApp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CppUnitTestApp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CppUnitTestApp - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "../../../include" /I "../.." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CPPUNIT_USE_TYPEINFO" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ../../../Lib/cppunit.lib ../../../Lib/testrunner.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "CppUnitTestApp - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../../include" /I "../.." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CPPUNIT_USE_TYPEINFO" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../../../Lib/cppunitd.lib ../../../Lib/testrunnerd.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CppUnitTestApp - Win32 Release"
# Name "CppUnitTestApp - Win32 Debug"
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CppUnitTestApp.cpp
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestApp.h
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestApp.rc
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestAppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestAppDlg.h
# End Source File
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
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CppUnitTestApp.ico
# End Source File
# Begin Source File

SOURCE=.\res\CppUnitTestApp.rc2
# End Source File
# End Group
# Begin Group "DLL Dependencies"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\lib\testrunner.dll

!IF  "$(CFG)" == "CppUnitTestApp - Win32 Release"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\Release
InputPath=..\..\..\lib\testrunner.dll
InputName=testrunner

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "CppUnitTestApp - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\testrunnerd.dll

!IF  "$(CFG)" == "CppUnitTestApp - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestApp - Win32 Debug"

# Begin Custom Build - $(IntDir)\$(InputName).dll
IntDir=.\Debug
InputPath=..\..\..\lib\testrunnerd.dll
InputName=testrunnerd

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(IntDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Tests"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\cppunittest\BaseTestCase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\BaseTestCase.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\CoreSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\CppUnitTestSuite.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ExceptionTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ExceptionTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ExceptionTestCaseDecoratorTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ExceptionTestCaseDecoratorTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ExtensionSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\FailureException.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\HelperMacrosTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\HelperMacrosTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\HelperSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MessageTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MessageTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockFunctor.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockProtector.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockTestCase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockTestCase.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockTestListener.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\MockTestListener.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\OrthodoxTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\OrthodoxTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\OutputSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\RepeatedTestTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\RepeatedTestTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\StringToolsTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\StringToolsTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\SubclassedTestCase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\SubclassedTestCase.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\SynchronizedTestResult.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestAssertTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestAssertTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestCallerTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestCallerTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestCaseTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestCaseTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestDecoratorTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestDecoratorTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestFailureTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestFailureTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestPathTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestPathTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestResultCollectorTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestResultCollectorTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestResultTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestResultTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestSetUpTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestSetUpTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestSuiteTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestSuiteTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TestTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\ToolsSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TrackedTestCase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\TrackedTestCase.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\UnitTestToolSuite.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlElementTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlElementTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlOutputterTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlOutputterTest.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlUniformiser.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlUniformiser.h
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlUniformiserTest.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\cppunittest\XmlUniformiserTest.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\cppunittest\Makefile.am
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
