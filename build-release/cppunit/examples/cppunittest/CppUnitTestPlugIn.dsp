# Microsoft Developer Studio Project File - Name="CppUnitTestPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CppUnitTestPlugIn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestPlugIn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestPlugIn.mak" CFG="CppUnitTestPlugIn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CppUnitTestPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CppUnitTestPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CppUnitTestPlugIn - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleasePlugIn"
# PROP Intermediate_Dir "ReleasePlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNITTESTPLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "../../include" /D "NDEBUG" /D "CPPUNITTESTPLUGIN_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNIT_DLL" /YX /FD /c
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

!ELSEIF  "$(CFG)" == "CppUnitTestPlugIn - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugPlugIn"
# PROP Intermediate_Dir "DebugPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPPUNITTESTPLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "CPPUNIT_DLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"DebugPlugIn/CppUnitTestPlugInd.dll" /pdbtype:sept /libpath:"../../lib/"

!ENDIF 

# Begin Target

# Name "CppUnitTestPlugIn - Win32 Release"
# Name "CppUnitTestPlugIn - Win32 Debug"
# Begin Group "Suites"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CoreSuite.h
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestSuite.cpp
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestSuite.h
# End Source File
# Begin Source File

SOURCE=.\ExtensionSuite.h
# End Source File
# Begin Source File

SOURCE=.\HelperSuite.h
# End Source File
# Begin Source File

SOURCE=.\OutputSuite.h
# End Source File
# Begin Source File

SOURCE=.\ToolsSuite.h
# End Source File
# Begin Source File

SOURCE=.\UnitTestToolSuite.h
# End Source File
# End Group
# Begin Group "TestSupport"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseTestCase.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseTestCase.h
# End Source File
# Begin Source File

SOURCE=.\FailureException.h
# End Source File
# Begin Source File

SOURCE=.\MockFunctor.h
# End Source File
# Begin Source File

SOURCE=.\MockProtector.h
# End Source File
# Begin Source File

SOURCE=.\MockTestCase.cpp
# End Source File
# Begin Source File

SOURCE=.\MockTestCase.h
# End Source File
# Begin Source File

SOURCE=.\MockTestListener.cpp
# End Source File
# Begin Source File

SOURCE=.\MockTestListener.h
# End Source File
# Begin Source File

SOURCE=.\SubclassedTestCase.cpp
# End Source File
# Begin Source File

SOURCE=.\SubclassedTestCase.h
# End Source File
# Begin Source File

SOURCE=.\SynchronizedTestResult.h
# End Source File
# Begin Source File

SOURCE=.\TrackedTestCase.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackedTestCase.h
# End Source File
# End Group
# Begin Group "Tests"

# PROP Default_Filter ""
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExceptionTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionTest.h
# End Source File
# Begin Source File

SOURCE=.\MessageTest.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageTest.h
# End Source File
# Begin Source File

SOURCE=.\TestAssertTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestAssertTest.h
# End Source File
# Begin Source File

SOURCE=.\TestCallerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestCallerTest.h
# End Source File
# Begin Source File

SOURCE=.\TestCaseTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestCaseTest.h
# End Source File
# Begin Source File

SOURCE=.\TestFailureTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestFailureTest.h
# End Source File
# Begin Source File

SOURCE=.\TestPathTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestPathTest.h
# End Source File
# Begin Source File

SOURCE=.\TestResultTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestResultTest.h
# End Source File
# Begin Source File

SOURCE=.\TestSuiteTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestSuiteTest.h
# End Source File
# Begin Source File

SOURCE=.\TestTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestTest.h
# End Source File
# End Group
# Begin Group "UnitTestTools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XmlUniformiser.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlUniformiser.h
# End Source File
# Begin Source File

SOURCE=.\XmlUniformiserTest.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlUniformiserTest.h
# End Source File
# End Group
# Begin Group "Helper"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HelperMacrosTest.cpp
# End Source File
# Begin Source File

SOURCE=.\HelperMacrosTest.h
# End Source File
# End Group
# Begin Group "Extension"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExceptionTestCaseDecoratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionTestCaseDecoratorTest.h
# End Source File
# Begin Source File

SOURCE=.\OrthodoxTest.cpp
# End Source File
# Begin Source File

SOURCE=.\OrthodoxTest.h
# End Source File
# Begin Source File

SOURCE=.\RepeatedTestTest.cpp
# End Source File
# Begin Source File

SOURCE=.\RepeatedTestTest.h
# End Source File
# Begin Source File

SOURCE=.\TestDecoratorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestDecoratorTest.h
# End Source File
# Begin Source File

SOURCE=.\TestSetUpTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestSetUpTest.h
# End Source File
# End Group
# Begin Group "Output"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TestResultCollectorTest.cpp
# End Source File
# Begin Source File

SOURCE=.\TestResultCollectorTest.h
# End Source File
# Begin Source File

SOURCE=.\XmlOutputterTest.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlOutputterTest.h
# End Source File
# End Group
# Begin Group "Tools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\StringToolsTest.cpp
# End Source File
# Begin Source File

SOURCE=.\StringToolsTest.h
# End Source File
# Begin Source File

SOURCE=.\XmlElementTest.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlElementTest.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\CppUnitTestPlugIn.cpp
# End Source File
# End Target
# End Project
