# Microsoft Developer Studio Project File - Name="LibOpenJPEG" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibOpenJPEG - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBOPENJPEG.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBOPENJPEG.MAK" CFG="LibOpenJPEG - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibOpenJPEG - Win32 DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "LibOpenJPEG - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "LibOpenJPEG - Win32 DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\"
# PROP BASE Intermediate_Dir "..\"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "..\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "NDEBUG" /D "WIN32" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "DLL_EXPORT" /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "NDEBUG" /D "WIN32" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "WIN32_LEAN_AND_MEAN" /D "DLL_EXPORT" /FD /Oxs /GF /GL /c
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /c 1252
# ADD RSC /l 0x409 /d "NDEBUG" /c 1252
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x794E0000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /out:"..\openjp2.dll" /osversion:5.2 /opt:ref,icf,nowin98 /release /ltcg
# ADD LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x794E0000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /out:"..\openjp2.dll" /osversion:5.2 /opt:ref,icf,nowin98 /release /ltcg

!ELSEIF  "$(CFG)" == "LibOpenJPEG - Win32 LIB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\"
# PROP BASE Intermediate_Dir "..\"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "..\"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "NDEBUG" /D "WIN32" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "OPJ_STATIC" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "NDEBUG" /D "WIN32" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "OPJ_STATIC" /Zl /FD /Oxs /GF /GL /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg

!ENDIF 

# Begin Target

# Name "LibOpenJPEG - Win32 DLL Release"
# Name "LibOpenJPEG - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\BIO.C
# End Source File
# Begin Source File

SOURCE=.\CIDX_MANAGER.C
# End Source File
# Begin Source File

SOURCE=.\CIO.C
# End Source File
# Begin Source File

SOURCE=.\DWT.C
# End Source File
# Begin Source File

SOURCE=.\EVENT.C
# End Source File
# Begin Source File

SOURCE=.\FUNCTION_LIST.C
# End Source File
# Begin Source File

SOURCE=.\HT_DEC.C
# End Source File
# Begin Source File

SOURCE=.\IMAGE.C
# End Source File
# Begin Source File

SOURCE=.\INVERT.C
# End Source File
# Begin Source File

SOURCE=.\J2K.C
# End Source File
# Begin Source File

SOURCE=.\JP2.C
# End Source File
# Begin Source File

SOURCE=.\MCT.C
# End Source File
# Begin Source File

SOURCE=.\MQC.C
# End Source File
# Begin Source File

SOURCE=.\OPENJPEG.C
# End Source File
# Begin Source File

SOURCE=.\OPJ_CLOCK.C
# End Source File
# Begin Source File

SOURCE=.\OPJ_MALLOC.C
# End Source File
# Begin Source File

SOURCE=.\PHIX_MANAGER.C
# End Source File
# Begin Source File

SOURCE=.\PI.C
# End Source File
# Begin Source File

SOURCE=.\PPIX_MANAGER.C
# End Source File
# Begin Source File

SOURCE=.\SPARSE_ARRAY.C
# End Source File
# Begin Source File

SOURCE=.\T1.C
# End Source File
# Begin Source File

SOURCE=.\T2.C
# End Source File
# Begin Source File

SOURCE=.\TCD.C
# End Source File
# Begin Source File

SOURCE=.\TGT.C
# End Source File
# Begin Source File

SOURCE=.\THIX_MANAGER.C
# End Source File
# Begin Source File

SOURCE=.\THREAD.C
# End Source File
# Begin Source File

SOURCE=.\TPIX_MANAGER.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\bit.h
# End Source File
# Begin Source File

SOURCE=.\decoder.h
# End Source File
# Begin Source File

SOURCE=.\fixed.h
# End Source File
# Begin Source File

SOURCE=.\frame.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\huffman.h
# End Source File
# Begin Source File

SOURCE=.\layer12.h
# End Source File
# Begin Source File

SOURCE=.\layer3.h
# End Source File
# Begin Source File

SOURCE=.\stream.h
# End Source File
# Begin Source File

SOURCE=.\synth.h
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\openjp2.rc

!IF  "$(CFG)" == "LibOpenJPEG - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "LibOpenJPEG - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
