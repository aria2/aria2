# Microsoft Developer Studio Project File - Name="xmlwf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=xmlwf - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xmlwf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xmlwf.mak" CFG="xmlwf - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xmlwf - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "xmlwf - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xmlwf - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\win32\bin\Release"
# PROP Intermediate_Dir "..\win32\tmp\Release-xmlwf"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\lib" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /machine:I386
# ADD LINK32 libexpat.lib setargv.obj /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"..\win32\bin\Release" /out:"..\win32\bin\Release\xmlwf.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "xmlwf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\win32\bin\Debug"
# PROP Intermediate_Dir "..\win32\tmp\Debug-xmlwf"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\lib" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 libexpat.lib setargv.obj /nologo /subsystem:console /pdb:none /debug /machine:I386 /libpath:"..\win32\bin\Debug" /out:"..\win32\bin\Debug\xmlwf.exe"

!ENDIF 

# Begin Target

# Name "xmlwf - Win32 Release"
# Name "xmlwf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\codepage.c
# End Source File
# Begin Source File

SOURCE=.\readfilemap.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\unixfilemap.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\win32filemap.c
# End Source File
# Begin Source File

SOURCE=.\xmlfile.c
# End Source File
# Begin Source File

SOURCE=.\xmlwf.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\codepage.h
# End Source File
# Begin Source File

SOURCE=.\xmlfile.h
# End Source File
# Begin Source File

SOURCE=.\xmltchar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
