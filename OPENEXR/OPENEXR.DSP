# Microsoft Developer Studio Project File - Name="openexr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=openexr - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OPENEXR.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OPENEXR.MAK" CFG="openexr - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "openexr - Win32 LIB Release" (based on "Win32 (x86) Static Library")
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
# ADD BASE CPP /nologo /MD /W3 /GR /Gy /I "." /I ".\IlmImf" /I ".\Imath" /I ".\IexMath" /I ".\Iex" /I ".\Half" /I ".\IlmThread" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "__SSE2__" /D "_STATIC_CPPLIB" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /GR /Gy /I "." /I ".\IlmImf" /I ".\Imath" /I ".\IexMath" /I ".\Iex" /I ".\Half" /I ".\IlmThread" /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "__SSE2__" /D "_STATIC_CPPLIB" /Zl /FD /Oxs /GF /GL /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg
# Begin Target

# Name "openexr - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Half\half.cpp
# End Source File
# Begin Source File

SOURCE=.\Iex\IexBaseExc.cpp
# End Source File
# Begin Source File

SOURCE=.\IexMath\IexMathFloatExc.cpp
# End Source File
# Begin Source File

SOURCE=.\IexMath\IexMathFpu.cpp
# End Source File
# Begin Source File

SOURCE=.\Iex\IexThrowErrnoExc.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadMutexWin32.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadSemaphoreWin32.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadWin32.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathColorAlgo.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathFun.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathMatrixAlgo.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathRandom.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathShear.cpp
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathVec.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfAcesFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfB44Compressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfBoxAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChannelList.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChannelListAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChromaticities.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChromaticitiesAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompositeDeepScanLine.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompressionAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCRgbaFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepCompositing.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepFrameBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepImageStateAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineInputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineOutputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledInputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledOutputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDoubleAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDwaCompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfEnvmap.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfEnvmapAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFastHuf.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFloatAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFloatVectorAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFrameBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFramesPerSecond.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfGenericInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfGenericOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfHuf.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputPartData.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfIntAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfIO.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfKeyCode.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfKeyCodeAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfLineOrderAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfLut.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMatrixAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiPartInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiPartOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiView.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOpaqueAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputPartData.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPartType.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPizCompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPreviewImage.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPreviewImageAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPxr24Compressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRational.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRationalAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRgbaFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRgbaYca.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRle.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRleCompressor.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfScanLineInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStandardAttributes.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStdIO.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStringAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStringVectorAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfSystemSpecific.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTestFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfThreading.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTileDescriptionAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledInputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledInputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledOutputFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledOutputPart.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledRgbaFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTileOffsets.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTimeCode.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTimeCodeAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfVecAttribute.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfWav.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfZip.cpp
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfZipCompressor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\IlmImf\b44ExpLogTable.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\dwaLookups.h
# End Source File
# Begin Source File

SOURCE=.\Half\eLut.h
# End Source File
# Begin Source File

SOURCE=.\Half\half.h
# End Source File
# Begin Source File

SOURCE=.\Half\halfExport.h
# End Source File
# Begin Source File

SOURCE=.\Half\halfFunction.h
# End Source File
# Begin Source File

SOURCE=.\Half\halfLimits.h
# End Source File
# Begin Source File

SOURCE=.\Iex\Iex.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexBaseExc.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexErrnoExc.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexExport.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexForward.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexMacros.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexMathExc.h
# End Source File
# Begin Source File

SOURCE=.\IexMath\IexMathFloatExc.h
# End Source File
# Begin Source File

SOURCE=.\IexMath\IexMathFpu.h
# End Source File
# Begin Source File

SOURCE=.\IexMath\IexMathIeeeExc.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexNamespace.h
# End Source File
# Begin Source File

SOURCE=.\Iex\IexThrowErrnoExc.h
# End Source File
# Begin Source File

SOURCE=.\IlmBaseConfig.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThread.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadExport.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadForward.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadMutex.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadNamespace.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadPool.h
# End Source File
# Begin Source File

SOURCE=.\IlmThread\IlmThreadSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathBox.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathBoxAlgo.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathColor.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathColorAlgo.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathEuler.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathExc.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathExport.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathForward.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathFrame.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathFrustum.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathFrustumTest.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathFun.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathGL.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathGLU.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathHalfLimits.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathInt64.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathInterval.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathLimits.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathLine.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathLineAlgo.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathMath.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathMatrix.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathMatrixAlgo.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathNamespace.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathPlane.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathPlatform.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathQuat.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathRandom.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathRoots.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathShear.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathSphere.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathVec.h
# End Source File
# Begin Source File

SOURCE=.\Imath\ImathVecAlgo.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfAcesFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfArray.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfAutoArray.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfB44Compressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfBoxAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChannelList.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChannelListAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCheckedArithmetic.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChromaticities.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfChromaticitiesAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompositeDeepScanLine.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompression.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompressionAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCompressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfConvert.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfCRgbaFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepCompositing.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepFrameBuffer.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepImageState.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepImageStateAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineInputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepScanLineOutputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledInputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDeepTiledOutputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDoubleAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDwaCompressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfDwaCompressorSimd.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfEnvmap.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfEnvmapAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfExport.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFastHuf.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFloatAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFloatVectorAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfForward.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFrameBuffer.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfFramesPerSecond.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfGenericInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfGenericOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfHeader.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfHuf.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputPartData.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInputStreamMutex.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfInt64.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfIntAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfIO.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfKeyCode.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfKeyCodeAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfLineOrder.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfLineOrderAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfLut.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMatrixAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMisc.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiPartInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiPartOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfMultiView.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfName.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfNamespace.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOpaqueAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOptimizedPixelReading.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputPartData.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfOutputStreamMutex.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPartHelper.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPartType.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPixelType.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPizCompressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPreviewImage.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPreviewImageAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfPxr24Compressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRational.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRationalAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRgba.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRgbaFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRgbaYca.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRle.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfRleCompressor.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfScanLineInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfSimd.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStandardAttributes.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStdIO.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStringAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfStringVectorAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfSystemSpecific.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTestFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfThreading.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTileDescription.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTileDescriptionAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledInputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledInputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledMisc.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledOutputFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledOutputPart.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTiledRgbaFile.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTileOffsets.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTimeCode.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfTimeCodeAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfVecAttribute.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfVersion.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfWav.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfXdr.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfZip.h
# End Source File
# Begin Source File

SOURCE=.\IlmImf\ImfZipCompressor.h
# End Source File
# Begin Source File

SOURCE=.\OpenEXRConfig.h
# End Source File
# Begin Source File

SOURCE=.\Half\toFloat.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
