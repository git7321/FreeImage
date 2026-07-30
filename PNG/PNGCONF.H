/* pngconf.h - machine configurable file for libpng
 *
 * libpng version 1.5.20
 * This code is released under the libpng license.
 */

#ifndef PNGCONF_H
#define PNGCONF_H

#include <setjmp.h>
#include <time.h>
#include <zlib.h>

/* options */
#define PNG_16BIT_SUPPORTED
#define PNG_ALIGNED_MEMORY_SUPPORTED
#define PNG_BENIGN_ERRORS_SUPPORTED
#define PNG_BUILD_GRAYSCALE_PALETTE_SUPPORTED
#define PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED
#define PNG_CHECK_cHRM_SUPPORTED
#define PNG_CONVERT_tIME_SUPPORTED
#define PNG_EASY_ACCESS_SUPPORTED
#define PNG_ERROR_TEXT_SUPPORTED
#define PNG_FIXED_POINT_SUPPORTED
#define PNG_FLOATING_ARITHMETIC_SUPPORTED
#define PNG_FLOATING_POINT_SUPPORTED
#define PNG_GET_PALETTE_MAX_SUPPORTED
#define PNG_HANDLE_AS_UNKNOWN_SUPPORTED
#define PNG_INCH_CONVERSIONS_SUPPORTED
#define PNG_INFO_IMAGE_SUPPORTED
#define PNG_IO_STATE_SUPPORTED
#define PNG_MNG_FEATURES_SUPPORTED
#define PNG_POINTER_INDEXING_SUPPORTED
#define PNG_PROGRESSIVE_READ_SUPPORTED
#define PNG_READ_16BIT_SUPPORTED
#define PNG_READ_ALPHA_MODE_SUPPORTED
#define PNG_READ_ANCILLARY_CHUNKS_SUPPORTED
#define PNG_READ_BACKGROUND_SUPPORTED
#define PNG_READ_BGR_SUPPORTED
#define PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
#define PNG_READ_COMPOSITE_NODIV_SUPPORTED
#define PNG_READ_COMPRESSED_TEXT_SUPPORTED
#define PNG_READ_EXPAND_16_SUPPORTED
#define PNG_READ_EXPAND_SUPPORTED
#define PNG_READ_FILLER_SUPPORTED
#define PNG_READ_GAMMA_SUPPORTED
#define PNG_READ_GET_PALETTE_MAX_SUPPORTED
#define PNG_READ_GRAY_TO_RGB_SUPPORTED
#define PNG_READ_INTERLACING_SUPPORTED
#define PNG_READ_INT_FUNCTIONS_SUPPORTED
#define PNG_READ_INVERT_ALPHA_SUPPORTED
#define PNG_READ_INVERT_SUPPORTED
#define PNG_READ_OPT_PLTE_SUPPORTED
#define PNG_READ_PACKSWAP_SUPPORTED
#define PNG_READ_PACK_SUPPORTED
#define PNG_READ_QUANTIZE_SUPPORTED
#define PNG_READ_RGB_TO_GRAY_SUPPORTED
#define PNG_READ_SCALE_16_TO_8_SUPPORTED
#define PNG_READ_SHIFT_SUPPORTED
#define PNG_READ_STRIP_16_TO_8_SUPPORTED
#define PNG_READ_STRIP_ALPHA_SUPPORTED
#define PNG_READ_SUPPORTED
#define PNG_READ_SWAP_ALPHA_SUPPORTED
#define PNG_READ_SWAP_SUPPORTED
#define PNG_READ_TEXT_SUPPORTED
#define PNG_READ_TRANSFORMS_SUPPORTED
#define PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
#define PNG_READ_USER_CHUNKS_SUPPORTED
#define PNG_READ_USER_TRANSFORM_SUPPORTED
#define PNG_READ_bKGD_SUPPORTED
#define PNG_READ_cHRM_SUPPORTED
#define PNG_READ_gAMA_SUPPORTED
#define PNG_READ_hIST_SUPPORTED
#define PNG_READ_iCCP_SUPPORTED
#define PNG_READ_iTXt_SUPPORTED
#define PNG_READ_oFFs_SUPPORTED
#define PNG_READ_pCAL_SUPPORTED
#define PNG_READ_pHYs_SUPPORTED
#define PNG_READ_sBIT_SUPPORTED
#define PNG_READ_sCAL_SUPPORTED
#define PNG_READ_sPLT_SUPPORTED
#define PNG_READ_sRGB_SUPPORTED
#define PNG_READ_tEXt_SUPPORTED
#define PNG_READ_tIME_SUPPORTED
#define PNG_READ_tRNS_SUPPORTED
#define PNG_READ_zTXt_SUPPORTED
#define PNG_SAVE_INT_32_SUPPORTED
#define PNG_SEQUENTIAL_READ_SUPPORTED
#define PNG_SETJMP_SUPPORTED
#define PNG_SET_CHUNK_CACHE_LIMIT_SUPPORTED
#define PNG_SET_CHUNK_MALLOC_LIMIT_SUPPORTED
#define PNG_ERROR_NUMBERS_SUPPORTED
#define PNG_SET_OPTION_SUPPORTED
#define PNG_SET_USER_LIMITS_SUPPORTED
#define PNG_STDIO_SUPPORTED
#define PNG_TEXT_SUPPORTED
#define PNG_TIME_RFC1123_SUPPORTED
#define PNG_UNKNOWN_CHUNKS_SUPPORTED
#define PNG_USER_CHUNKS_SUPPORTED
#define PNG_USER_LIMITS_SUPPORTED
#define PNG_USER_MEM_SUPPORTED
#define PNG_USER_TRANSFORM_INFO_SUPPORTED
#define PNG_USER_TRANSFORM_PTR_SUPPORTED
#define PNG_WARNINGS_SUPPORTED
#define PNG_WRITE_16BIT_SUPPORTED
#define PNG_WRITE_ANCILLARY_CHUNKS_SUPPORTED
#define PNG_WRITE_BGR_SUPPORTED
#define PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED
#define PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
#define PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
#define PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
#define PNG_WRITE_FILLER_SUPPORTED
#define PNG_WRITE_FILTER_SUPPORTED
#define PNG_WRITE_FLUSH_SUPPORTED
#define PNG_WRITE_GET_PALETTE_MAX_SUPPORTED
#define PNG_WRITE_INTERLACING_SUPPORTED
#define PNG_WRITE_INT_FUNCTIONS_SUPPORTED
#define PNG_WRITE_INVERT_ALPHA_SUPPORTED
#define PNG_WRITE_INVERT_SUPPORTED
#define PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
#define PNG_WRITE_PACKSWAP_SUPPORTED
#define PNG_WRITE_PACK_SUPPORTED
#define PNG_WRITE_SHIFT_SUPPORTED
#define PNG_WRITE_SUPPORTED
#define PNG_WRITE_SWAP_ALPHA_SUPPORTED
#define PNG_WRITE_SWAP_SUPPORTED
#define PNG_WRITE_TEXT_SUPPORTED
#define PNG_WRITE_TRANSFORMS_SUPPORTED
#define PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
#define PNG_WRITE_USER_TRANSFORM_SUPPORTED
#define PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
#define PNG_WRITE_bKGD_SUPPORTED
#define PNG_WRITE_cHRM_SUPPORTED
#define PNG_WRITE_gAMA_SUPPORTED
#define PNG_WRITE_hIST_SUPPORTED
#define PNG_WRITE_iCCP_SUPPORTED
#define PNG_WRITE_iTXt_SUPPORTED
#define PNG_WRITE_oFFs_SUPPORTED
#define PNG_WRITE_pCAL_SUPPORTED
#define PNG_WRITE_pHYs_SUPPORTED
#define PNG_WRITE_sBIT_SUPPORTED
#define PNG_WRITE_sCAL_SUPPORTED
#define PNG_WRITE_sPLT_SUPPORTED
#define PNG_WRITE_sRGB_SUPPORTED
#define PNG_WRITE_tEXt_SUPPORTED
#define PNG_WRITE_tIME_SUPPORTED
#define PNG_WRITE_tRNS_SUPPORTED
#define PNG_WRITE_zTXt_SUPPORTED
#define PNG_bKGD_SUPPORTED
#define PNG_cHRM_SUPPORTED
#define PNG_gAMA_SUPPORTED
#define PNG_hIST_SUPPORTED
#define PNG_iCCP_SUPPORTED
#define PNG_iTXt_SUPPORTED
#define PNG_oFFs_SUPPORTED
#define PNG_pCAL_SUPPORTED
#define PNG_pHYs_SUPPORTED
#define PNG_sBIT_SUPPORTED
#define PNG_sCAL_SUPPORTED
#define PNG_sPLT_SUPPORTED
#define PNG_sRGB_SUPPORTED
#define PNG_tEXt_SUPPORTED
#define PNG_tIME_SUPPORTED
#define PNG_tRNS_SUPPORTED
#define PNG_zTXt_SUPPORTED
/* end of options */

#ifndef PNG_READ_INT_FUNCTIONS_SUPPORTED
# define PNG_USE_READ_MACROS
#endif

#ifndef PNGARG
# define PNGARG(arglist) OF(arglist)
#endif

#ifndef PNGCAPI
# ifdef WIN32
#  define PNGCAPI __cdecl
# else
#  define PNGCAPI _cdecl
# endif
#endif

#ifdef WIN32
# define PNG_DLL_EXPORT __declspec(dllexport)
# define PNG_DLL_IMPORT __declspec(dllimport)
#else
# define PNG_DLL_EXPORT
# define PNG_DLL_IMPORT
#endif

#ifndef PNGCBAPI
# define PNGCBAPI PNGCAPI
#endif
#ifndef PNGAPI
# define PNGAPI   PNGCAPI
#endif

#ifndef PNG_IMPEXP
# ifdef BUILD_DLL
#  define PNG_IMPEXP  PNG_DLL_EXPORT
# else
#  ifdef PNG_DLL
#   define PNG_IMPEXP PNG_DLL_IMPORT
#  else
#   define PNG_IMPEXP
#  endif
# endif
#endif

#ifndef PNG_FUNCTION
# define PNG_FUNCTION(type, name, args, attributes) attributes type name args
#endif

#ifndef PNG_EXPORT_TYPE
# define PNG_EXPORT_TYPE(type) PNG_IMPEXP type
#endif

#ifndef PNG_EXPORTA
# define PNG_EXPORTA(ordinal, type, name, args, attributes) \
   PNG_FUNCTION(PNG_EXPORT_TYPE(type),(PNGAPI name),PNGARG(args), \
   extern attributes)
#endif

#define PNG_EMPTY
#define PNG_EXPORT(ordinal, type, name, args) \
   PNG_EXPORTA(ordinal, type, name, args, PNG_EMPTY)

#ifndef PNG_REMOVED
# define PNG_REMOVED(ordinal, type, name, args, attributes)
#endif

#ifndef PNG_CALLBACK
# define PNG_CALLBACK(type, name, args) type (PNGCBAPI name) PNGARG(args)
#endif

#ifndef PNG_PEDANTIC_WARNINGS_SUPPORTED
# define PNG_PEDANTIC_WARNINGS_SUPPORTED
#endif

#ifdef PNG_PEDANTIC_WARNINGS_SUPPORTED
# if defined(_MSC_VER) && (_MSC_VER >= 1200)
#  define PNG_NORETURN   __declspec(noreturn)
# endif
# if defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define PNG_DEPRECATED __declspec(deprecated)
#  define PNG_PRIVATE    __declspec(deprecated)
# endif
#endif

#ifndef PNG_DEPRECATED
# define PNG_DEPRECATED
#endif
#ifndef PNG_USE_RESULT
# define PNG_USE_RESULT
#endif
#ifndef PNG_NORETURN
# define PNG_NORETURN
#endif
#ifndef PNG_ALLOCATED
# define PNG_ALLOCATED
#endif
#ifndef PNG_PRIVATE
# define PNG_PRIVATE
#endif
#ifndef PNG_RESTRICT
# define PNG_RESTRICT
#endif

#ifndef PNG_FP_EXPORT
# ifdef PNG_FLOATING_POINT_SUPPORTED
#  define PNG_FP_EXPORT(ordinal, type, name, args) \
   PNG_EXPORT(ordinal, type, name, args);
# else
#  define PNG_FP_EXPORT(ordinal, type, name, args)
# endif
#endif
#ifndef PNG_FIXED_EXPORT
# ifdef PNG_FIXED_POINT_SUPPORTED
#  define PNG_FIXED_EXPORT(ordinal, type, name, args) \
   PNG_EXPORT(ordinal, type, name, args);
# else
#  define PNG_FIXED_EXPORT(ordinal, type, name, args)
# endif
#endif

#ifndef PNG_CONST
# define PNG_CONST const
#endif

#if defined(INT_MAX) && (INT_MAX > 0x7ffffffeL)
 typedef unsigned int  png_uint_32;
 typedef int           png_int_32;
#else
 typedef unsigned long png_uint_32;
 typedef long          png_int_32;
#endif
typedef unsigned short png_uint_16;
typedef short          png_int_16;
typedef unsigned char  png_byte;
typedef png_int_32     png_fixed_point;

typedef unsigned int   png_size_t;
typedef png_uint_32    png_alloc_size_t;
#define png_sizeof(x) (sizeof(x))

#if defined(MAXSEG_64K) && !defined(PNG_MAX_MALLOC_64K)
# define PNG_MAX_MALLOC_64K
#endif

#ifndef PNG_MAX_MALLOC_64K
# define PNG_USE_COMPILE_TIME_MASKS
#endif

#ifdef FAR
# ifdef M_I86MM
#  define USE_FAR_KEYWORD
#  define FARDATA FAR
#  include <dos.h>
# endif
#endif

#ifndef FARDATA
# define FARDATA
#endif

#ifdef PNG_STDIO_SUPPORTED
 typedef FILE           * png_FILE_p;
#endif

typedef void                      FAR * png_voidp;
typedef PNG_CONST void            FAR * png_const_voidp;
typedef png_byte                  FAR * png_bytep;
typedef PNG_CONST png_byte        FAR * png_const_bytep;
typedef png_uint_32               FAR * png_uint_32p;
typedef PNG_CONST png_uint_32     FAR * png_const_uint_32p;
typedef png_int_32                FAR * png_int_32p;
typedef PNG_CONST png_int_32      FAR * png_const_int_32p;
typedef png_uint_16               FAR * png_uint_16p;
typedef PNG_CONST png_uint_16     FAR * png_const_uint_16p;
typedef png_int_16                FAR * png_int_16p;
typedef PNG_CONST png_int_16      FAR * png_const_int_16p;
typedef char                      FAR * png_charp;
typedef PNG_CONST char            FAR * png_const_charp;
typedef png_fixed_point           FAR * png_fixed_point_p;
typedef PNG_CONST png_fixed_point FAR * png_const_fixed_point_p;
typedef png_size_t                FAR * png_size_tp;
typedef PNG_CONST png_size_t      FAR * png_const_size_tp;
typedef double                    FAR * png_doublep;
typedef PNG_CONST double          FAR * png_const_doublep;

typedef png_byte        FAR * FAR * png_bytepp;
typedef png_uint_32     FAR * FAR * png_uint_32pp;
typedef png_int_32      FAR * FAR * png_int_32pp;
typedef png_uint_16     FAR * FAR * png_uint_16pp;
typedef png_int_16      FAR * FAR * png_int_16pp;
typedef PNG_CONST char  FAR * FAR * png_const_charpp;
typedef char            FAR * FAR * png_charpp;
typedef png_fixed_point FAR * FAR * png_fixed_point_pp;
typedef double          FAR * FAR * png_doublepp;
typedef char            FAR * FAR * FAR * png_charppp;

#endif /* PNGCONF_H */
