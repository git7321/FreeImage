# Microsoft Developer Studio Project File - Name="LibOpenJPEG" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibOpenJPEG - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibOpenJPEG.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibOpenJPEG.mak" CFG="LibOpenJPEG - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibOpenJPEG - Win32 LIB Release" (based on "Win32 (x86) Static Library")
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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "OPJ_STATIC" /D "WIN32_LEAN_AND_MEAN" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "OPJ_STATIC" /D "WIN32_LEAN_AND_MEAN" /Zl /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg
# Begin Target

# Name "LibOpenJPEG - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bio.c
# End Source File
# Begin Source File

SOURCE=.\cidx_manager.c
# End Source File
# Begin Source File

SOURCE=.\cio.c
# End Source File
# Begin Source File

SOURCE=.\dwt.c
# End Source File
# Begin Source File

SOURCE=.\event.c
# End Source File
# Begin Source File

SOURCE=.\function_list.c
# End Source File
# Begin Source File

SOURCE=.\ht_dec.c
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\invert.c
# End Source File
# Begin Source File

SOURCE=.\j2k.c
# End Source File
# Begin Source File

SOURCE=.\jp2.c
# End Source File
# Begin Source File

SOURCE=.\mct.c
# End Source File
# Begin Source File

SOURCE=.\mqc.c
# End Source File
# Begin Source File

SOURCE=.\openjpeg.c
# End Source File
# Begin Source File

SOURCE=.\opj_clock.c
# End Source File
# Begin Source File

SOURCE=.\opj_malloc.c
# End Source File
# Begin Source File

SOURCE=.\phix_manager.c
# End Source File
# Begin Source File

SOURCE=.\pi.c
# End Source File
# Begin Source File

SOURCE=.\ppix_manager.c
# End Source File
# Begin Source File

SOURCE=.\sparse_array.c
# End Source File
# Begin Source File

SOURCE=.\t1.c
# End Source File
# Begin Source File

SOURCE=.\t2.c
# End Source File
# Begin Source File

SOURCE=.\tcd.c
# End Source File
# Begin Source File

SOURCE=.\tgt.c
# End Source File
# Begin Source File

SOURCE=.\thix_manager.c
# End Source File
# Begin Source File

SOURCE=.\thread.c
# End Source File
# Begin Source File

SOURCE=.\tpix_manager.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\opj_includes.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
