# Microsoft Developer Studio Project File - Name="freeimage" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FREEIMAGE - WIN32 DLL RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "freeimage.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "freeimage.mak" CFG="FREEIMAGE - WIN32 DLL RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "freeimage - Win32 DLL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
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
# ADD BASE CPP /nologo /MD /W3 /GX /Gy /I "." /I ".\jpegl" /I ".\LibTIFF4" /I ".\FreeImage" /I ".\FreeImageToolkit" /I ".\Metadata" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "OPJ_STATIC" /D "FREEIMAGE_EXPORTS" /D "LIBRAW_NODLL" /D "ASMV" /D "__SSE2__" /D "_STATIC_CPPLIB" /D "JPEG_HAS_READ_ICC_PROFILE" /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /GX /Gy /I "." /I ".\jpegl" /I ".\LibTIFF4" /I ".\FreeImage" /I ".\FreeImageToolkit" /I ".\Metadata" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "OPJ_STATIC" /D "FREEIMAGE_EXPORTS" /D "LIBRAW_NODLL" /D "ASMV" /D "__SSE2__" /D "_STATIC_CPPLIB" /D "JPEG_HAS_READ_ICC_PROFILE" /FD /Oxs /GF /GL /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /c 1252
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32" /c 1252
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib .\JPEGl\LIBJPEG.lib .\JPEGl\LIBJPEG12.lib .\JPEGl\LIBJPEG16.lib .\LIBJXR\LIBJXR.lib .\LibOpenJPEG\LibOpenJPEG.lib .\LibRawLite\LibRawLite.lib .\openexr\openexr.lib .\PNG\libPNG.lib .\LibTIFF4\libTIFF.lib .\WEBP\libWEBP.lib .\zlib\zlib.lib libcpmt.lib ws2_32.lib msvcrt.lib /nologo /base:"0x787F0000" /version:5.2 /dll /debug /machine:I386 /out:"freeimage.dll" /subsystem:windows,4.0 /opt:ref,icf,nowin98 /osversion:5.2 /release /ltcg
# ADD LINK32 kernel32.lib .\JPEGl\LIBJPEG.lib .\JPEGl\LIBJPEG12.lib .\JPEGl\LIBJPEG16.lib .\LIBJXR\LIBJXR.lib .\LibOpenJPEG\LibOpenJPEG.lib .\LibRawLite\LibRawLite.lib .\openexr\openexr.lib .\PNG\libPNG.lib .\LibTIFF4\libTIFF.lib .\WEBP\libWEBP.lib .\zlib\zlib.lib libcpmt.lib ws2_32.lib msvcxp.lib /nologo /base:"0x787F0000" /version:5.2 /dll /debug /machine:I386 /out:"freeimage.dll" /subsystem:windows,4.0 /opt:ref,icf,nowin98 /osversion:5.2 /release /ltcg
# Begin Target

# Name "freeimage - Win32 DLL Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\FreeImageToolkit\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\BitmapAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\BSplineRotate.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\CacheFile.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Channels.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\ClassicRotate.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ColorLookup.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Colors.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion16_555.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion16_565.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion24.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion32.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion4.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Conversion8.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionFloat.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionRGB16.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionRGBA16.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionRGBAF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionRGBF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionType.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ConversionUINT16.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\CopyPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\Exif.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\FIRational.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\FIRational.h
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Flip.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\FreeImage.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\FreeImageC.c
# End Source File
# Begin Source File

SOURCE=.\FreeImage\FreeImageIO.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\FreeImageTag.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\FreeImageTag.h
# End Source File
# Begin Source File

SOURCE=.\FreeImage\GetType.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Halftoning.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\IPTC.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\J2KHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\JPEGTransform.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\LFPQuantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\MemoryIO.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\MNGHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\MultigridPoissonSolver.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\MultiPage.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\NNQuantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PixelAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginCUT.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginDDS.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginEXR.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginG3.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginGIF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginHDR.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginICO.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginIFF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginJ2K.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginJNG.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginJP2.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginJPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginJXR.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginKOALA.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginMNG.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPCD.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPCX.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPFM.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPICT.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPNG.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPNM.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginPSD.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginRAS.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginRAW.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginSGI.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginTARGA.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginTIFF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginWBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginWebP.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginXBM.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PluginXPM.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PSDParser.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\PSDParser.h
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Rescale.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImageToolkit\Resize.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\TagConversion.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\TagLib.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\TIFFLogLuv.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\tmoColorConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\tmoDrago03.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\tmoFattal02.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\tmoReinhard05.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ToneMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\WuQuantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Metadata\XTIFF.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage\ZLibInterface.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\freeimage.RC
# End Source File
# End Group
# End Target
# End Project
