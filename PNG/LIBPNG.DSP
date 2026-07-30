# Microsoft Developer Studio Project File - Name="libpng" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpng - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBPNG.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBPNG.MAK" CFG="libpng - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpng - Win32 DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libpng - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "libpng - Win32 DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "."
# PROP BASE Intermediate_Dir "."
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "BUILD_DLL" /D "ZLIB_WINAPI" /D "ASMV" /D "ZLIB_DLL" /FD /Oxs /GF /GS- /Zc:wchar_t- /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "BUILD_DLL" /D "ZLIB_WINAPI" /D "ASMV" /D "ZLIB_DLL" /FD /Oxs /GF /GS- /Zc:wchar_t- /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /c 1252 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /c 1252 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib ..\zlib\zlib.lib msvcrt.lib /nologo /base:"0x78740000" /version:5.2 /dll /debug /machine:I386 /subsystem:windows,4.0 /out:"libpng.dll" /opt:ref,icf,nowin98 /osversion:5.2 /release
# ADD LINK32 kernel32.lib ..\zlib\zlib.lib msvcrt.lib /nologo /base:"0x78740000" /version:5.2 /dll /debug /machine:I386 /subsystem:windows,4.0 /out:"libpng.dll" /opt:ref,icf,nowin98 /osversion:5.2 /release

!ELSEIF  "$(CFG)" == "libpng - Win32 LIB Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "NOCRYPT" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "NOCRYPT" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg

!ENDIF 

# Begin Target

# Name "libpng - Win32 DLL Release"
# Name "libpng - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\png.c
# End Source File
# Begin Source File

SOURCE=.\pngerror.c
# End Source File
# Begin Source File

SOURCE=.\pngget.c
# End Source File
# Begin Source File

SOURCE=.\pngmem.c
# End Source File
# Begin Source File

SOURCE=.\pngpread.c
# End Source File
# Begin Source File

SOURCE=.\pngread.c
# End Source File
# Begin Source File

SOURCE=.\pngrio.c
# End Source File
# Begin Source File

SOURCE=.\pngrtran.c
# End Source File
# Begin Source File

SOURCE=.\pngrutil.c
# End Source File
# Begin Source File

SOURCE=.\pngset.c
# End Source File
# Begin Source File

SOURCE=.\pngtrans.c
# End Source File
# Begin Source File

SOURCE=.\pngwio.c
# End Source File
# Begin Source File

SOURCE=.\pngwrite.c
# End Source File
# Begin Source File

SOURCE=.\pngwtran.c
# End Source File
# Begin Source File

SOURCE=.\pngwutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\pngconf.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\pngdll.rc

!IF  "$(CFG)" == "libpng - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "libpng - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
