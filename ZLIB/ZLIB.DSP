# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ZLIB - WIN32 DLL RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZLIB.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZLIB.MAK" CFG="ZLIB - WIN32 DLL RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - WAPI DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - WAPI LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "ASMV" /D "ZLIB_DLL" /D "BUILD_DLL" /FD /Ox /GF /GS- /Zc:wchar_t- /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "ASMV" /D "ZLIB_DLL" /D "BUILD_DLL" /FD /Ox /GF /GS- /Zc:wchar_t- /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x78800000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /subsystem:windows,4.0 /out:".\zlib1.dll" /opt:ref,icf,nowin98 /osversion:5.2 /largeaddressaware /release
# ADD LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x78800000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /subsystem:windows,4.0 /out:".\zlib1.dll" /opt:ref,icf,nowin98 /osversion:5.2 /largeaddressaware /release

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "ASMV" /D "ZLIB_DLL" /D "BUILD_DLL" /FD /Oxs /GF /GS- /Zc:wchar_t- /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "ZLIB_WINAPI" /D "ASMV" /D "ZLIB_DLL" /D "BUILD_DLL" /FD /Oxs /GF /GS- /Zc:wchar_t- /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "ZLIB_WINAPI" /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "ZLIB_WINAPI" /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x78700000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /subsystem:windows,4.0 /opt:ref,icf,nowin98 /osversion:5.2 /largeaddressaware /release
# ADD LINK32 kernel32.lib msvcrt.lib /nologo /base:"0x78700000" /version:5.2 /dll /debug /machine:I386 /nodefaultlib /subsystem:windows,4.0 /opt:ref,icf,nowin98 /osversion:5.2 /largeaddressaware /release

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "ASMV" /D "WIN32_LEAN_AND_MEAN" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "ASMV" /D "WIN32_LEAN_AND_MEAN" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /NODEFAULTLIB /ltcg
# ADD LIB32 /nologo /NODEFAULTLIB /ltcg

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

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
# ADD BASE CPP /nologo /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "ZLIB_WINAPI" /D "ASMV" /D "WIN32_LEAN_AND_MEAN" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /c
# ADD CPP /nologo /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "ZLIB_WINAPI" /D "ASMV" /D "WIN32_LEAN_AND_MEAN" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\zlibstat.lib" /NODEFAULTLIB
# ADD LIB32 /nologo /out:".\zlibstat.lib" /NODEFAULTLIB

!ENDIF 

# Begin Target

# Name "zlib - Win32 DLL Release"
# Name "zlib - WAPI DLL Release"
# Name "zlib - Win32 LIB Release"
# Name "zlib - WAPI LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\adler32.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\compress.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\crc32.c
# End Source File
# Begin Source File

SOURCE=.\deflate.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\dllversion.c
# ADD CPP /Oxs

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gzio.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\infback.c
# End Source File
# Begin Source File

SOURCE=.\inffast.c
# End Source File
# Begin Source File

SOURCE=.\inflate.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\minizip\ioapi.c
# ADD CPP /Oxs

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\minizip\iowin32.c
# ADD CPP /Oxs

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trees.c
# End Source File
# Begin Source File

SOURCE=.\uncompr.c
# ADD CPP /Oxs
# End Source File
# Begin Source File

SOURCE=.\minizip\unzip.c
# ADD CPP /Oxs

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\minizip\zip.c
# ADD CPP /Oxs

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zlib.def

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zlib1.def

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zutil.c
# ADD CPP /Oxs
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\crc32.h
# End Source File
# Begin Source File

SOURCE=.\minizip\crypt.h
# End Source File
# Begin Source File

SOURCE=.\deflate.h
# End Source File
# Begin Source File

SOURCE=.\inffast.h
# End Source File
# Begin Source File

SOURCE=.\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\inflate.h
# End Source File
# Begin Source File

SOURCE=.\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\minizip\ioapi.h
# End Source File
# Begin Source File

SOURCE=.\minizip\iowin32.h
# End Source File
# Begin Source File

SOURCE=.\trees.h
# End Source File
# Begin Source File

SOURCE=.\minizip\unzip.h
# End Source File
# Begin Source File

SOURCE=.\zconf.h
# End Source File
# Begin Source File

SOURCE=.\minizip\zip.h
# End Source File
# Begin Source File

SOURCE=.\zutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\zlib.rc

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembler Files"

# PROP Default_Filter "asm;obj;c;cpp;cxx;h;hpp;hxx"
# Begin Source File

SOURCE=.\masmx86\match32.asm

!IF  "$(CFG)" == "zlib - Win32 DLL Release"

# Begin Custom Build - Assembling...
IntDir=.\.
InputPath=.\masmx86\match32.asm
InputName=match32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm.exe -f win32 -o "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - WAPI DLL Release"

# Begin Custom Build - Assembling...
IntDir=.\.
InputPath=.\masmx86\match32.asm
InputName=match32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm.exe -f win32 -o "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Release"

# Begin Custom Build - Assembling...
IntDir=.\.
InputPath=.\masmx86\match32.asm
InputName=match32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm.exe -f win32 -o "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - WAPI LIB Release"

# Begin Custom Build - Assembling...
IntDir=.\.
InputPath=.\masmx86\match32.asm
InputName=match32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm.exe -f win32 -o "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
