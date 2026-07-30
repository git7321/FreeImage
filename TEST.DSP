# Microsoft Developer Studio Project File - Name="test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TEST.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TEST.MAK" CFG="test - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test - Win32 Release" (based on "Win32 (x86) Console Application")
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
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /I ".\test" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Oxs /GF /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /I ".\test" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Oxs /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib freeimage.lib /nologo /version:5.2 /subsystem:console /debug /machine:IX86 /osversion:5.2 /opt:ref,icf,nowin98 /release
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib freeimage.lib /nologo /version:5.2 /subsystem:console /debug /machine:IX86 /osversion:5.2 /opt:ref,icf,nowin98 /release
# Begin Target

# Name "test - Win32 Release"
# Begin Source File

SOURCE=.\test\MainTestSuite.cpp
DEP_CPP_MAINT=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testChannels.cpp
DEP_CPP_TESTC=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testHeaderOnly.cpp
DEP_CPP_TESTH=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testImageType.cpp
DEP_CPP_TESTI=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testJPEG.cpp
DEP_CPP_TESTJ=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testMemIO.cpp
DEP_CPP_TESTM=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testMPage.cpp
DEP_CPP_TESTMP=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testMPageMemory.cpp
DEP_CPP_TESTMPA=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testMPageStream.cpp
DEP_CPP_TESTMPAG=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testPlugins.cpp
DEP_CPP_TESTP=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\TestSuite.h
# End Source File
# Begin Source File

SOURCE=.\test\testThumbnail.cpp
DEP_CPP_TESTT=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testTools.cpp
DEP_CPP_TESTTO=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\test\testWrappedBuffer.cpp
DEP_CPP_TESTW=\
	".\FreeImage.h"\
	".\test\TestSuite.h"\
	
# End Source File
# End Target
# End Project
