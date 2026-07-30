# Microsoft Developer Studio Project File - Name="LibRawLite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibRawLite - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBRAWLITE.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBRAWLITE.MAK" CFG="LibRawLite - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibRawLite - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "."
# PROP BASE Intermediate_Dir "."
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Gy /I "." /I ".\internal" /I ".\libraw" /I ".\src" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "LIBRAW_NODLL" /D "WIN32_LEAN_AND_MEAN" /D "_STATIC_CPPLIB" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /GX /Gy /I "." /I ".\internal" /I ".\libraw" /I ".\src" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "LIBRAW_NODLL" /D "WIN32_LEAN_AND_MEAN" /D "_STATIC_CPPLIB" /Zl /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg
# Begin Target

# Name "LibRawLite - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\internal\dcraw_common.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\dcraw_fileio.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\demosaic_packs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libraw_c_api.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libraw_cxx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libraw_datastream.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
