# Microsoft Developer Studio Project File - Name="libjxr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libjxr - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBJXR.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBJXR.MAK" CFG="libjxr - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libjxr - Win32 LIB Release" (based on "Win32 (x86) Static Library")
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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /I ".\image" /I ".\jxrgluelib" /I ".\jxrtestlib" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /I ".\image" /I ".\jxrgluelib" /I ".\jxrtestlib" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /Zl /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg
# Begin Target

# Name "libjxr - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\image\sys\adapthuff.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\decode.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\encode.c
# End Source File
# Begin Source File

SOURCE=.\image\sys\image.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\JXRTranscode.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\postprocess.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\segdec.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\segenc.c
# End Source File
# Begin Source File

SOURCE=.\image\sys\strcodec.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\strdec.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\strdec_x86.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\strenc.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\strenc_x86.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\strFwdTransform.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\strInvTransform.c
# End Source File
# Begin Source File

SOURCE=.\image\sys\strPredQuant.c
# End Source File
# Begin Source File

SOURCE=.\image\decode\strPredQuantDec.c
# End Source File
# Begin Source File

SOURCE=.\image\encode\strPredQuantEnc.c
# End Source File
# Begin Source File

SOURCE=.\image\sys\strTransform.c
# End Source File
# End Group
# Begin Group "jxrgluelib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jxrgluelib\JXRGlue.c
# End Source File
# Begin Source File

SOURCE=.\jxrgluelib\JXRGlueJxr.c
# End Source File
# Begin Source File

SOURCE=.\jxrgluelib\JXRGluePFC.c
# End Source File
# Begin Source File

SOURCE=.\jxrgluelib\JXRMeta.c
# End Source File
# End Group
# Begin Group "jxrtestlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jxrtestlib\JXRTest.c
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTestBmp.c
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTestHdr.c
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTestPnm.c
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTestTif.c
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTestYUV.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\image\sys\ansi.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\common.h
# End Source File
# Begin Source File

SOURCE=.\image\decode\decode.h
# End Source File
# Begin Source File

SOURCE=.\image\encode\encode.h
# End Source File
# Begin Source File

SOURCE=.\jxrgluelib\JXRGlue.h
# End Source File
# Begin Source File

SOURCE=.\jxrgluelib\JXRMeta.h
# End Source File
# Begin Source File

SOURCE=.\jxrtestlib\JXRTest.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\perfTimer.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\strcodec.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\strTransform.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\windowsmediaphoto.h
# End Source File
# Begin Source File

SOURCE=.\image\x86\x86.h
# End Source File
# Begin Source File

SOURCE=.\image\sys\xplatform_image.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
