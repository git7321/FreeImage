/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef ZCONF_H
#define ZCONF_H

/* If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 */
#ifdef Z_PREFIX
# define Z_PREFIX_SET

/* all linked symbols */
# define _dist_code           z__dist_code
# define _length_code         z__length_code
# define _tr_align            z__tr_align
# define _tr_flush_bits       z__tr_flush_bits
# define _tr_flush_block      z__tr_flush_block
# define _tr_init             z__tr_init
# define _tr_stored_block     z__tr_stored_block
# define _tr_tally            z__tr_tally
# define adler32              z_adler32
# define adler32_combine      z_adler32_combine
# define compress             z_compress
# define compress2            z_compress2
# define compressBound        z_compressBound
# define crc32                z_crc32
# define crc32_combine        z_crc32_combine
# define deflate              z_deflate
# define deflateBound         z_deflateBound
# define deflateCopy          z_deflateCopy
# define deflateEnd           z_deflateEnd
# define deflateInit2_        z_deflateInit2_
# define deflateInit_         z_deflateInit_
# define deflateParams        z_deflateParams
# define deflatePending       z_deflatePending
# define deflatePrime         z_deflatePrime
# define deflateReset         z_deflateReset
# define deflateResetKeep     z_deflateResetKeep
# define deflateSetDictionary z_deflateSetDictionary
# define deflateSetHeader     z_deflateSetHeader
# define deflateTune          z_deflateTune
# define deflate_copyright    z_deflate_copyright
# define get_crc_table        z_get_crc_table
# define gz_strwinerror       z_gz_strwinerror
# define gzclearerr           z_gzclearerr
# define gzclose              z_gzclose
# define gzdirect             z_gzdirect
# define gzdopen              z_gzdopen
# define gzeof                z_gzeof
# define gzerror              z_gzerror
# define gzflush              z_gzflush
# define gzgetc               z_gzgetc
# define gzgets               z_gzgets
# define gzopen               z_gzopen
# define gzprintf             z_gzprintf
# define gzputc               z_gzputc
# define gzputs               z_gzputs
# define gzread               z_gzread
# define gzrewind             z_gzrewind
# define gzseek               z_gzseek
# define gzsetparams          z_gzsetparams
# define gztell               z_gztell
# define gzungetc             z_gzungetc
# define gzwrite              z_gzwrite
# define inflate              z_inflate
# define inflateBack          z_inflateBack
# define inflateBackEnd       z_inflateBackEnd
# define inflateBackInit_     z_inflateBackInit_
# define inflateCopy          z_inflateCopy
# define inflateEnd           z_inflateEnd
# define inflateGetHeader     z_inflateGetHeader
# define inflateInit2_        z_inflateInit2_
# define inflateInit_         z_inflateInit_
# define inflateMark          z_inflateMark
# define inflatePrime         z_inflatePrime
# define inflateReset         z_inflateReset
# define inflateReset2        z_inflateReset2
# define inflateSetDictionary z_inflateSetDictionary
# define inflateSync          z_inflateSync
# define inflateSyncPoint     z_inflateSyncPoint
# define inflateUndermine     z_inflateUndermine
# define inflateValidate      z_inflateValidate
# define inflateResetKeep     z_inflateResetKeep
# define inflate_copyright    z_inflate_copyright
# define inflate_fast         z_inflate_fast
# define inflate_table        z_inflate_table
# define uncompress           z_uncompress
# define zError               z_zError
# define zcalloc              z_zcalloc
# define zcfree               z_zcfree
# define zlibCompileFlags     z_zlibCompileFlags
# define zlibVersion          z_zlibVersion

/* all zlib typedefs in zlib.h and zconf.h */
# define Byte                 z_Byte
# define Bytef                z_Bytef
# define alloc_func           z_alloc_func
# define charf                z_charf
# define free_func            z_free_func
# define gzFile               z_gzFile
# define gz_header            z_gz_header
# define gz_headerp           z_gz_headerp
# define in_func              z_in_func
# define intf                 z_intf
# define out_func             z_out_func
# define uInt                 z_uInt
# define uIntf                z_uIntf
# define uLong                z_uLong
# define uLongf               z_uLongf
# define voidp                z_voidp
# define voidpc               z_voidpc
# define voidpf               z_voidpf

/* all zlib structs in zlib.h and zconf.h */
# define gz_header_s          z_gz_header_s
# define internal_state       z_internal_state
#endif

#if defined(__MSDOS__) && !defined(MSDOS)
# define MSDOS
#endif
#if defined(_WINDOWS) && !defined(WINDOWS)
# define WINDOWS
#endif
#if defined(_WIN32) || defined(_WIN32_WCE)
# ifndef WIN32
#  define WIN32
# endif
#endif
#if (defined(MSDOS) || defined(WINDOWS)) && !defined(WIN32)
# ifndef SYS16BIT
#  define SYS16BIT
# endif
#endif

/* Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ifdef SYS16BIT
# define MAXSEG_64K
# define UNALIGNED_OK
#endif

#if !defined(STDC) && defined(_MSC_VER)
# define STDC
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
# ifdef MAXSEG_64K
#  define MAX_MEM_LEVEL 8
# else
#  define MAX_MEM_LEVEL 9
# endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2. */
#ifndef MAX_WBITS
# define MAX_WBITS 15
#endif

#ifndef OF
# ifdef STDC
#  define OF(args) args
# else
#  define OF(args) ()
# endif
#endif

#ifndef Z_ARG
# define Z_ARG OF
#endif

#ifndef z_const
# ifdef STDC
#  define z_const const
# else
#  define z_const
# endif
#endif

#ifdef STDC
# ifndef _WIN32_WCE
#  include <stddef.h>
# endif
# include <limits.h>
# include <malloc.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdarg.h>
# include <string.h>
# include <sys/types.h>
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 */
#ifdef SYS16BIT
# if defined(M_I86SM) || defined(M_I86MM)
#  define SMALL_MEDIUM
#  ifndef FAR
#   ifdef _MSC_VER
#    define FAR _far
#   else
#    define FAR far
#   endif
#  endif
# elif (defined(__SMALL__) || defined(__MEDIUM__))
#  define SMALL_MEDIUM
#  ifndef FAR
#   ifdef __BORLANDC__
#    define FAR _far
#   else
#    define FAR far
#   endif
#  endif
# endif
#endif

/* If building or using zlib as a DLL, define ZLIB_DLL. */
#ifndef ZEXTERN
# if defined(WIN32) && defined(ZLIB_DLL)
#  ifdef BUILD_DLL
#   define ZEXTERN extern __declspec(dllexport)
#  else
#   define ZEXTERN extern __declspec(dllimport)
#  endif
# else
#  define ZEXTERN extern
# endif
#endif

/* For complete Windows compatibility, use WINAPI, not __stdcall. */
#ifndef ZEXPORT
# if defined (ZLIB_WINAPI)
# include <windows.h>
#  ifdef WIN32
#   define ZEXPORT   WINAPI
#   define ZEXPORTVA WINAPIV
#  else
#   define ZEXPORT   WINAPI
#   define ZEXPORTVA FAR CDECL
#  endif
# elif defined(WIN32)
#  define ZEXPORT   __cdecl
#  define ZEXPORTVA __cdecl
# else
#  define ZEXPORT   _cdecl
#  define ZEXPORTVA _cdecl
# endif
#endif

#ifndef FAR
# define FAR
#endif

typedef unsigned char Byte;
typedef unsigned int  uInt;
typedef unsigned long uLong;

#ifdef SMALL_MEDIUM
# define Bytef Byte FAR
#else
 typedef Byte FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
 typedef void z_const *voidpc;
 typedef void FAR     *voidpf;
 typedef void         *voidp;
#else
 typedef Byte z_const *voidpc;
 typedef Byte FAR     *voidpf;
 typedef Byte         *voidp;
#endif

#ifndef Z_U4
# if (UINT_MAX == 0xffffffffUL)
#  define Z_U4 unsigned int
# elif (ULONG_MAX == 0xffffffffUL)
#  define Z_U4 unsigned long
# elif (USHRT_MAX == 0xffffffffUL)
#  define Z_U4 unsigned short
# endif
#endif

#ifdef Z_U4
 typedef Z_U4 z_crc_t;
#else
 typedef unsigned long z_crc_t;
#endif

#ifndef z_off_t
# define z_off_t long
#endif

#ifndef z_off64_t
# ifdef WIN32
#  define z_off64_t __int64
# else
#  define z_off64_t z_off_t
# endif
#endif

#endif /* ZCONF_H */
