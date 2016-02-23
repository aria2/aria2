# Microsoft Developer Studio Project File - Name="expat_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=expat_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "expat_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "expat_static.mak" CFG="expat_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "expat_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "expat_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "expat_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "expat_static___Win32_Release"
# PROP BASE Intermediate_Dir "expat_static___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\win32\bin\Release"
# PROP Intermediate_Dir "..\win32\tmp\Release_static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\win32\bin\Release\libexpatMT.lib"

!ELSEIF  "$(CFG)" == "expat_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "expat_static___Win32_Debug"
# PROP BASE Intermediate_Dir "expat_static___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\win32\bin\Debug"
# PROP Intermediate_Dir "..\win32\tmp\Debug_static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\win32\bin\Debug\libexpatMT.lib"

!ENDIF 

# Begin Target

# Name "expat_static - Win32 Release"
# Name "expat_static - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\xmlparse.c
# End Source File
# Begin Source File

SOURCE=.\xmlrole.c
# End Source File
# Begin Source File

SOURCE=.\xmltok.c
# End Source File
# Begin Source File

SOURCE=.\xmltok_impl.c
# End Source File
# Begin Source File

SOURCE=.\xmltok_ns.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ascii.h
# End Source File
# Begin Source File

SOURCE=.\asciitab.h
# End Source File
# Begin Source File

SOURCE=.\expat.h
# End Source File
# Begin Source File

SOURCE=.\expat_external.h
# End Source File
# Begin Source File

SOURCE=.\iasciitab.h
# End Source File
# Begin Source File

SOURCE=.\internal.h
# End Source File
# Begin Source File

SOURCE=.\latin1tab.h
# End Source File
# Begin Source File

SOURCE=.\nametab.h
# End Source File
# Begin Source File

SOURCE=.\utf8tab.h
# End Source File
# Begin Source File

SOURCE=.\xmlrole.h
# End Source File
# Begin Source File

SOURCE=.\xmltok.h
# End Source File
# Begin Source File

SOURCE=.\xmltok_impl.h
# End Source File
# End Group
# End Target
# End Project
