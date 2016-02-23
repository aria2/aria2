# Microsoft Developer Studio Project File - Name="CppUnitTestMain" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CppUnitTestMain - Win32 Debug DLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestMain.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestMain.mak" CFG="CppUnitTestMain - Win32 Debug DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CppUnitTestMain - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CppUnitTestMain - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "CppUnitTestMain - Win32 Release DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "CppUnitTestMain - Win32 Debug DLL" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CppUnitTestMain - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib/"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\Release\CppUnitTestMain.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Self test
PostBuild_Cmds="$(TargetPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"../../lib/"
# Begin Special Build Tool
TargetPath=.\Debug\CppUnitTestMain.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Self test
PostBuild_Cmds="$(TargetPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Release DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CppUnitTestMain___Win32_Release_DLL"
# PROP BASE Intermediate_Dir "CppUnitTestMain___Win32_Release_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDLL"
# PROP Intermediate_Dir "ReleaseDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_USE_TYPEINFO" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /Ox /Ot /Oa /Ow /Og /Oi /Op /Ob0 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../lib/cppunit.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit_dll.lib /nologo /subsystem:console /machine:I386 /libpath:"../../lib/"
# SUBTRACT LINK32 /incremental:yes
# Begin Special Build Tool
TargetPath=.\ReleaseDLL\CppUnitTestMain.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Self test
PostBuild_Cmds=$(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CppUnitTestMain___Win32_Debug_DLL"
# PROP BASE Intermediate_Dir "CppUnitTestMain___Win32_Debug_DLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDLL"
# PROP Intermediate_Dir "DebugDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "CPPUNIT_DLL" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../lib/cppunitd.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd_dll.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"../../lib/"
# Begin Special Build Tool
TargetPath=.\DebugDLL\CppUnitTestMain.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Self test
PostBuild_Cmds=$(TargetPath)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "CppUnitTestMain - Win32 Release"
# Name "CppUnitTestMain - Win32 Debug"
# Name "CppUnitTestMain - Win32 Release DLL"
# Name "CppUnitTestMain - Win32 Debug DLL"
# Begin Group "Tests"

# PROP Default_Filter ""
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\assertion_traitsTest.cpp
# End Source File
# Begin Source File

SOURCE=.\assertion_traitsTest.h
# End Source File
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
# Begin Group "Suites"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CoreSuite.h
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestSuite.cpp
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
# Begin Source File

SOURCE=..\..\lib\cppunit_dll.dll

!IF  "$(CFG)" == "CppUnitTestMain - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Release DLL"

# Begin Custom Build - Updating DLL: $(InputPath)
IntDir=.\ReleaseDLL
InputPath=..\..\lib\cppunit_dll.dll
InputName=cppunit_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(InputPath)" "$(IntDir)\$(InputName).dll"

# End Custom Build

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug DLL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\lib\cppunitd_dll.dll

!IF  "$(CFG)" == "CppUnitTestMain - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Release DLL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "CppUnitTestMain - Win32 Debug DLL"

# Begin Custom Build - Updating DLL: $(InputPath)
IntDir=.\DebugDLL
InputPath=..\..\lib\cppunitd_dll.dll
InputName=cppunitd_dll

"$(IntDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(InputPath)" "$(IntDir)\$(InputName).dll"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CppUnitTestMain.cpp
# End Source File
# Begin Source File

SOURCE=.\Makefile.am
# End Source File
# End Target
# End Project
