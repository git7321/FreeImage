# Microsoft Developer Studio Project File - Name="libjpeg16" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libjpeg16 - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBJPEG16.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBJPEG16.MAK" CFG="libjpeg16 - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libjpeg16 - Win32 LIB Release" (based on "Win32 (x86) Static Library")
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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D BITS_IN_JSAMPLE=16 /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D BITS_IN_JSAMPLE=16 /Zl /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /NODEFAULTLIB /ltcg
# ADD LIB32 /nologo /NODEFAULTLIB /ltcg
# Begin Target

# Name "libjpeg16 - Win32 LIB Release"
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\jcapistd.c
# End Source File
# Begin Source File

SOURCE=.\jccolor.c
# End Source File
# Begin Source File

SOURCE=.\jcdiffct.c
# End Source File
# Begin Source File

SOURCE=.\jclossls.c
# End Source File
# Begin Source File

SOURCE=.\jcmainct.c
# End Source File
# Begin Source File

SOURCE=.\jcprepct.c
# End Source File
# Begin Source File

SOURCE=.\jcsample.c
# End Source File
# Begin Source File

SOURCE=.\jdapistd.c
# End Source File
# Begin Source File

SOURCE=.\jdcolor.c
# End Source File
# Begin Source File

SOURCE=.\jddiffct.c
# End Source File
# Begin Source File

SOURCE=.\jdlossls.c
# End Source File
# Begin Source File

SOURCE=.\jdmainct.c
# End Source File
# Begin Source File

SOURCE=.\jdpostct.c
# End Source File
# Begin Source File

SOURCE=.\jdsample.c
# End Source File
# Begin Source File

SOURCE=.\jutils.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# End Group
# End Target
# End Project
