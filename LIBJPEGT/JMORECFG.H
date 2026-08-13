/*
 * jmorecfg.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2009, 2011, 2014-2015, 2018, 2020, 2022, 2026,
 *           D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains additional configuration options that customize the
 * JPEG software for special applications or support machine-dependent
 * optimizations.  Most users will not need to touch this file.
 */

/*
 * Maximum number of components (color channels) allowed in JPEG image.
 * To meet the letter of Rec. ITU-T T.81 | ISO/IEC 10918-1, set this to 255.
 * However, darn few applications need more than 4 channels (maybe 5 for CMYK +
 * alpha mask).  We recommend 10 as a reasonable compromise; use 4 if you are
 * really short on memory.  (Each allowed component costs a hundred or so
 * bytes of storage, whether actually used in an image or not.)
 */

#define MAX_COMPONENTS  10

/* JSAMPLE should be the smallest type that will hold the values 0..255. */

typedef unsigned char JSAMPLE;
#define GETJSAMPLE(value)  ((int)(value))

#define MAXJSAMPLE       255
#define CENTERJSAMPLE    128

/* J12SAMPLE should be the smallest type that will hold the values 0..4095. */

typedef short J12SAMPLE;

#define MAXJ12SAMPLE     4095
#define CENTERJ12SAMPLE  2048

/* J16SAMPLE should be the smallest type that will hold the values 0..65535. */

typedef unsigned short J16SAMPLE;

#define MAXJ16SAMPLE     65535
#define CENTERJ16SAMPLE  32768

/* Representation of a DCT frequency coefficient.
 * This should be a signed value of at least 16 bits; "short" is usually OK.
 * Again, we allocate large arrays of these, but you can change to int
 * if you have memory to burn and "short" is really slow.
 */

typedef short JCOEF;

/* Compressed datastreams are represented as arrays of JOCTET.
 * These must be EXACTLY 8 bits wide, at least once they are written to
 * external storage.  Note that when using the stdio data source/destination
 * managers, this is also the data type passed to fread/fwrite.
 */

typedef unsigned char JOCTET;
#define GETJOCTET(value)  (value)

/* These typedefs are used for various table entries and so forth.
 * They must be at least as wide as specified; but making them too big
 * won't cost a huge amount of memory, so we don't provide special
 * extraction code like we did for JSAMPLE.  (In other words, these
 * typedefs live at a different point on the speed/space tradeoff curve.)
 */

/* UINT8 must hold at least the values 0..255. */

typedef unsigned char UINT8;

/* UINT16 must hold at least the values 0..65535. */

typedef unsigned short UINT16;

/* INT16 must hold at least the values -32768..32767. */

#ifndef XMD_H
typedef short INT16;
#endif

#ifndef XMD_H                   /* X11/xmd.h correctly defines INT32 */
#ifndef _BASETSD_H_             /* Microsoft defines it in basetsd.h */
#ifndef _BASETSD_H              /* MinGW is slightly different */
#ifndef QGLOBAL_H               /* Qt defines it in qglobal.h */
typedef long INT32;
#endif
#endif
#endif
#endif

typedef unsigned int JDIMENSION;

#define JPEG_MAX_DIMENSION  65500L

#ifndef JPGAPI
#if defined(WIN32) || defined(_WIN32)
#define JPGAPI __cdecl
#else
#define JPGAPI _cdecl
#endif
#endif

/* a function called through method pointers: */
#define METHODDEF(type)         static type JPGAPI
/* a function used only in its module: */
#define LOCAL(type)             static type JPGAPI
/* a function referenced thru EXTERNs: */
#define GLOBAL(type)            type JPGAPI
/* a reference to a GLOBAL function: */
#define EXTERN(type)            extern type JPGAPI

#define JMETHOD(type, methodname, arglist)  type (JPGAPI *methodname) arglist

#undef FAR
#define FAR

#ifndef HAVE_BOOLEAN
typedef int boolean;
#endif
#ifndef FALSE                   /* in case these macros already exist */
#define FALSE   0               /* values of boolean */
#endif
#ifndef TRUE
#define TRUE    1
#endif

#ifdef JPEG_INTERNALS
#define JPEG_INTERNAL_OPTIONS
#endif

#ifdef JPEG_INTERNAL_OPTIONS

/* Capability options common to encoder and decoder: */

#define DCT_ISLOW_SUPPORTED     /* accurate integer method */
#define DCT_IFAST_SUPPORTED     /* less accurate int method [legacy feature] */
#define DCT_FLOAT_SUPPORTED     /* floating-point method [legacy feature] */

/* Encoder capability options: */

#define C_MULTISCAN_FILES_SUPPORTED /* Multiple-scan JPEG files? */
#define C_PROGRESSIVE_SUPPORTED     /* Progressive JPEG?  (Requires
                                       C_MULTISCAN_FILES_SUPPORTED and
                                       ENTROPY_OPT_SUPPORTED) */
#define C_LOSSLESS_SUPPORTED        /* Lossless JPEG? */
#define ENTROPY_OPT_SUPPORTED       /* Optimization of entropy coding parms? */
#define INPUT_SMOOTHING_SUPPORTED   /* Input image smoothing option? */

/* Decoder capability options: */

#define D_MULTISCAN_FILES_SUPPORTED /* Multiple-scan JPEG files? */
#define D_PROGRESSIVE_SUPPORTED     /* Progressive JPEG?  (Requires
                                       D_MULTISCAN_FILES_SUPPORTED) */
#define D_LOSSLESS_SUPPORTED        /* Lossless JPEG?  (Requires
                                       D_MULTISCAN_FILES_SUPPORTED) */
#define SAVE_MARKERS_SUPPORTED      /* jpeg_save_markers() needed? */
#define BLOCK_SMOOTHING_SUPPORTED   /* Block smoothing? (Progressive only) */
#define IDCT_SCALING_SUPPORTED      /* Output rescaling via IDCT?  (Requires
                                       DCT_ISLOW_SUPPORTED) */
#define UPSAMPLE_MERGING_SUPPORTED  /* Fast path for sloppy upsampling? */
#define QUANT_1PASS_SUPPORTED       /* 1-pass color quantization? */
#define QUANT_2PASS_SUPPORTED       /* 2-pass color quantization? */

#define RGB_RED         0       /* Offset of Red in an RGB scanline element */
#define RGB_GREEN       1       /* Offset of Green */
#define RGB_BLUE        2       /* Offset of Blue */
#define RGB_PIXELSIZE   3       /* JSAMPLEs per RGB scanline element */

#define JPEG_NUMCS  17

#define EXT_RGB_RED         0
#define EXT_RGB_GREEN       1
#define EXT_RGB_BLUE        2
#define EXT_RGB_PIXELSIZE   3

#define EXT_RGBX_RED        0
#define EXT_RGBX_GREEN      1
#define EXT_RGBX_BLUE       2
#define EXT_RGBX_PIXELSIZE  4

#define EXT_BGR_RED         2
#define EXT_BGR_GREEN       1
#define EXT_BGR_BLUE        0
#define EXT_BGR_PIXELSIZE   3

#define EXT_BGRX_RED        2
#define EXT_BGRX_GREEN      1
#define EXT_BGRX_BLUE       0
#define EXT_BGRX_PIXELSIZE  4

#define EXT_XBGR_RED        3
#define EXT_XBGR_GREEN      2
#define EXT_XBGR_BLUE       1
#define EXT_XBGR_PIXELSIZE  4

#define EXT_XRGB_RED        1
#define EXT_XRGB_GREEN      2
#define EXT_XRGB_BLUE       3
#define EXT_XRGB_PIXELSIZE  4

static const int rgb_red[JPEG_NUMCS] = {
  -1, -1, RGB_RED, -1, -1, -1, EXT_RGB_RED, EXT_RGBX_RED,
  EXT_BGR_RED, EXT_BGRX_RED, EXT_XBGR_RED, EXT_XRGB_RED,
  EXT_RGBX_RED, EXT_BGRX_RED, EXT_XBGR_RED, EXT_XRGB_RED,
  -1
};

static const int rgb_green[JPEG_NUMCS] = {
  -1, -1, RGB_GREEN, -1, -1, -1, EXT_RGB_GREEN, EXT_RGBX_GREEN,
  EXT_BGR_GREEN, EXT_BGRX_GREEN, EXT_XBGR_GREEN, EXT_XRGB_GREEN,
  EXT_RGBX_GREEN, EXT_BGRX_GREEN, EXT_XBGR_GREEN, EXT_XRGB_GREEN,
  -1
};

static const int rgb_blue[JPEG_NUMCS] = {
  -1, -1, RGB_BLUE, -1, -1, -1, EXT_RGB_BLUE, EXT_RGBX_BLUE,
  EXT_BGR_BLUE, EXT_BGRX_BLUE, EXT_XBGR_BLUE, EXT_XRGB_BLUE,
  EXT_RGBX_BLUE, EXT_BGRX_BLUE, EXT_XBGR_BLUE, EXT_XRGB_BLUE,
  -1
};

static const int rgb_pixelsize[JPEG_NUMCS] = {
  -1, -1, RGB_PIXELSIZE, -1, -1, -1, EXT_RGB_PIXELSIZE, EXT_RGBX_PIXELSIZE,
  EXT_BGR_PIXELSIZE, EXT_BGRX_PIXELSIZE, EXT_XBGR_PIXELSIZE, EXT_XRGB_PIXELSIZE,
  EXT_RGBX_PIXELSIZE, EXT_BGRX_PIXELSIZE, EXT_XBGR_PIXELSIZE, EXT_XRGB_PIXELSIZE,
  -1
};

#ifndef MULTIPLIER
#ifndef WITH_SIMD
#define MULTIPLIER  int      /* type for fastest integer multiply */
#else
#define MULTIPLIER  short    /* prefer 16-bit with SIMD for parellelism */
#endif
#endif

#ifndef FAST_FLOAT
#define FAST_FLOAT  float
#endif

#endif /* JPEG_INTERNAL_OPTIONS */
