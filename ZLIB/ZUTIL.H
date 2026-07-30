/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef ZUTIL_H
#define ZUTIL_H

#define ZLIB_INTERNAL
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

typedef unsigned char  uch;
typedef uch FAR  uchf;
typedef unsigned short ush;
typedef ush FAR  ushf;
typedef unsigned long  ulg;

extern const char *z_errmsg[10];
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm,err) \
   return (strm->msg = ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH 3
#define MAX_MATCH 258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ifdef WIN32
#  define OS_CODE 0x0b
#else
#  define OS_CODE 0x00
#endif

#if defined(_MSC_VER) && (_MSC_VER > 600)
#  ifdef _WIN32_WCE
#   define fdopen(fd,mode) NULL
#  else
#   define fdopen(fd,type) _fdopen(fd,type)
#  endif
#endif

        /* functions */

#ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#  define zmemcpy _fmemcpy
#  define zmemcmp _fmemcmp
#  define zmemzero(dest, len) _fmemset(dest, 0, len)
#else
#  define zmemcpy memcpy
#  define zmemcmp memcmp
#  define zmemzero(dest, len) memset(dest, 0, len)
#endif

        /* Diagnostic functions */

#ifdef DEBUG
   extern int z_verbose;
   extern void z_error OF((char *m));
#  define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#  define Trace(x) {if (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {if (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {if (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

voidpf ZEXPORTVA zcalloc OF((voidpf opaque, uInt items,
          uInt size));
void ZEXPORTVA zcfree OF((voidpf opaque, voidpf ptr));

#define ZALLOC(strm, items, size) \
          (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr) (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef __cplusplus
}
#endif

#endif /* ZUTIL_H */
