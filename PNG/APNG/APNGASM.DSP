# Microsoft Developer Studio Project File - Name="apngasm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=apngasm - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "APNGASM.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "APNGASM.MAK" CFG="apngasm - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "apngasm - Win32 Release" (based on "Win32 (x86) Console Application")
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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gy /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 zlib.lib libpng.lib kernel32.lib /nologo /version:4.0 /subsystem:console /debug /machine:I386 /opt:ref,icf,nowin98 /release /ltcg
# ADD LINK32 zlib.lib libpng.lib kernel32.lib /nologo /version:4.0 /subsystem:console /debug /machine:I386 /opt:ref,icf,nowin98 /release /ltcg
# Begin Target

# Name "apngasm - Win32 Release"
# Begin Group "Zopfli"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zopfli\src\blocksplitter.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\blocksplitter.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\cache.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\cache.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\deflate.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\gzip_container.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\gzip_container.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\hash.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\hash.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\katajainen.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\katajainen.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\lz77.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\lz77.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\squeeze.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\squeeze.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\symbols.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\tree.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\tree.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\util.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\util.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\zlib_container.c
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\zlib_container.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\zopfli.h
# End Source File
# Begin Source File

SOURCE=.\zopfli\src\zopfli_lib.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\apngasm.cpp
# End Source File
# End Target
# End Project
