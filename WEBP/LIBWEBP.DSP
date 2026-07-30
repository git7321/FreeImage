# Microsoft Developer Studio Project File - Name="libwebp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libwebp - Win32 LIB Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBWEBP.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBWEBP.MAK" CFG="libwebp - Win32 LIB Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libwebp - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\"
# PROP BASE Intermediate_Dir ".\"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\"
# PROP Intermediate_Dir ".\"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "NOCRYPT" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
# ADD CPP /nologo /MD /W3 /Gy /I "." /D "WIN32" /D "NDEBUG" /D WINVER=0x0400 /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "NOCRYPT" /D "NOGDI" /D "NOSERVICE" /D "NOMCX" /D "NOIME" /Zl /FD /Oxs /GF /GL /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /ltcg
# ADD LIB32 /nologo /ltcg
# Begin Target

# Name "libwebp - Win32 LIB Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\dec\alpha_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\alpha_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\alpha_processing.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\alpha_processing_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\analysis_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\demux\anim_decode.c
# End Source File
# Begin Source File

SOURCE=.\src\mux\anim_encode.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\backward_references_cost_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\backward_references_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\bit_reader_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\bit_writer_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\buffer_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\color_cache_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\config_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\cost.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\cost_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\cost_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\cpu.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\dec.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\dec_clip_tables.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\dec_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\demux\demux.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\enc_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\filter_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\filters.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\filters_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\filters_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\frame_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\frame_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\histogram_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\huffman_encode_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\huffman_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\idec_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\io_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\iterator_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless_enc_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\mux\muxedit.c
# End Source File
# Begin Source File

SOURCE=.\src\mux\muxinternal.c
# End Source File
# Begin Source File

SOURCE=.\src\mux\muxread.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\near_lossless_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\picture_csp_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\picture_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\picture_psnr_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\picture_rescale_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\picture_tools_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\predictor_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\quant_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\quant_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\quant_levels_dec_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\quant_levels_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\random_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\rescaler.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\rescaler_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\rescaler_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\ssim.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\ssim_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\syntax_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\thread_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\token_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\tree_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\tree_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\upsampling.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\upsampling_sse2.c
# End Source File
# Begin Source File

SOURCE=.\src\utils\utils.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\vp8_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\vp8l_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\vp8l_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dec\webp_dec.c
# End Source File
# Begin Source File

SOURCE=.\src\enc\webp_enc.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\yuv.c
# End Source File
# Begin Source File

SOURCE=.\src\dsp\yuv_sse2.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\dec\alphai_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\mux\animi.h
# End Source File
# Begin Source File

SOURCE=.\src\enc\backward_references_enc.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\bit_reader_inl_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\bit_reader_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\bit_writer_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\color_cache_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\dec\common_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\common_sse2.h
# End Source File
# Begin Source File

SOURCE=.\src\enc\cost_enc.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\decode.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\demux.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\dsp.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\encode.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\endian_inl_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\filters_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\format_constants.h
# End Source File
# Begin Source File

SOURCE=.\src\enc\histogram_enc.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\huffman_encode_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\huffman_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\lossless_common.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\mux.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\mux_types.h
# End Source File
# Begin Source File

SOURCE=.\src\mux\muxi.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\quant.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\quant_levels_dec_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\quant_levels_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\random_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\rescaler_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\thread_utils.h
# End Source File
# Begin Source File

SOURCE=.\src\webp\types.h
# End Source File
# Begin Source File

SOURCE=.\src\utils\utils.h
# End Source File
# Begin Source File

SOURCE=.\src\dec\vp8_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\dec\vp8i_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\enc\vp8i_enc.h
# End Source File
# Begin Source File

SOURCE=.\src\dec\vp8li_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\enc\vp8li_enc.h
# End Source File
# Begin Source File

SOURCE=.\src\dec\webpi_dec.h
# End Source File
# Begin Source File

SOURCE=.\src\dsp\yuv.h
# End Source File
# End Group
# End Target
# End Project
