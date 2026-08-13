/*
 * jccolor.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2009-2012, 2015, 2022, 2024-2025, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains input colorspace conversion routines.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#ifdef WITH_SIMD
#include "../simd/jsimd.h"
#endif
#include "jsamplecomp.h"

#if BITS_IN_JSAMPLE != 16 || defined(C_LOSSLESS_SUPPORTED)

typedef struct {
  struct jpeg_color_converter pub;

#if BITS_IN_JSAMPLE != 16
  JLONG *rgb_ycc_tab;
#endif
} my_color_converter;

typedef my_color_converter *my_cconvert_ptr;

#define SCALEBITS       16
#define CBCR_OFFSET     ((JLONG)_CENTERJSAMPLE << SCALEBITS)
#define ONE_HALF        ((JLONG)1 << (SCALEBITS - 1))
#define FIX(x)          ((JLONG)((x) * (1L << SCALEBITS) + 0.5))

#define R_Y_OFF         0
#define G_Y_OFF         (1 * (_MAXJSAMPLE + 1))
#define B_Y_OFF         (2 * (_MAXJSAMPLE + 1))
#define R_CB_OFF        (3 * (_MAXJSAMPLE + 1))
#define G_CB_OFF        (4 * (_MAXJSAMPLE + 1))
#define B_CB_OFF        (5 * (_MAXJSAMPLE + 1))
#define R_CR_OFF        B_CB_OFF
#define G_CR_OFF        (6 * (_MAXJSAMPLE + 1))
#define B_CR_OFF        (7 * (_MAXJSAMPLE + 1))
#define TABLE_SIZE      (8 * (_MAXJSAMPLE + 1))

#if BITS_IN_JSAMPLE == 12
#define RANGE_LIMIT(value)  ((value) & 0xFFF)
#else
#define RANGE_LIMIT(value)  (value)
#endif

#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE

#define RGB_RED  EXT_RGB_RED
#define RGB_GREEN  EXT_RGB_GREEN
#define RGB_BLUE  EXT_RGB_BLUE
#define RGB_PIXELSIZE  EXT_RGB_PIXELSIZE
#define rgb_ycc_convert  extrgb_ycc_convert
#define rgb_gray_convert  extrgb_gray_convert
#define rgb_rgb_convert  extrgb_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

#define RGB_RED  EXT_RGBX_RED
#define RGB_GREEN  EXT_RGBX_GREEN
#define RGB_BLUE  EXT_RGBX_BLUE
#define RGB_PIXELSIZE  EXT_RGBX_PIXELSIZE
#define rgb_ycc_convert  extrgbx_ycc_convert
#define rgb_gray_convert  extrgbx_gray_convert
#define rgb_rgb_convert  extrgbx_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

#define RGB_RED  EXT_BGR_RED
#define RGB_GREEN  EXT_BGR_GREEN
#define RGB_BLUE  EXT_BGR_BLUE
#define RGB_PIXELSIZE  EXT_BGR_PIXELSIZE
#define rgb_ycc_convert  extbgr_ycc_convert
#define rgb_gray_convert  extbgr_gray_convert
#define rgb_rgb_convert  extbgr_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

#define RGB_RED  EXT_BGRX_RED
#define RGB_GREEN  EXT_BGRX_GREEN
#define RGB_BLUE  EXT_BGRX_BLUE
#define RGB_PIXELSIZE  EXT_BGRX_PIXELSIZE
#define rgb_ycc_convert  extbgrx_ycc_convert
#define rgb_gray_convert  extbgrx_gray_convert
#define rgb_rgb_convert  extbgrx_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

#define RGB_RED  EXT_XBGR_RED
#define RGB_GREEN  EXT_XBGR_GREEN
#define RGB_BLUE  EXT_XBGR_BLUE
#define RGB_PIXELSIZE  EXT_XBGR_PIXELSIZE
#define rgb_ycc_convert  extxbgr_ycc_convert
#define rgb_gray_convert  extxbgr_gray_convert
#define rgb_rgb_convert  extxbgr_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

#define RGB_RED  EXT_XRGB_RED
#define RGB_GREEN  EXT_XRGB_GREEN
#define RGB_BLUE  EXT_XRGB_BLUE
#define RGB_PIXELSIZE  EXT_XRGB_PIXELSIZE
#define rgb_ycc_convert  extxrgb_ycc_convert
#define rgb_gray_convert  extxrgb_gray_convert
#define rgb_rgb_convert  extxrgb_rgb_convert
#include "jccolext.c"
#undef RGB_RED
#undef RGB_GREEN
#undef RGB_BLUE
#undef RGB_PIXELSIZE
#undef rgb_ycc_convert
#undef rgb_gray_convert
#undef rgb_rgb_convert

METHODDEF(void)
rgb_ycc_start(j_compress_ptr cinfo)
{
#if BITS_IN_JSAMPLE != 16
  my_cconvert_ptr cconvert = (my_cconvert_ptr)cinfo->cconvert;
  JLONG *rgb_ycc_tab;
  JLONG i;

  cconvert->rgb_ycc_tab = rgb_ycc_tab = (JLONG *)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                (TABLE_SIZE * sizeof(JLONG)));

  for (i = 0; i <= _MAXJSAMPLE; i++) {
    rgb_ycc_tab[i + R_Y_OFF] = FIX(0.29900) * i;
    rgb_ycc_tab[i + G_Y_OFF] = FIX(0.58700) * i;
    rgb_ycc_tab[i + B_Y_OFF] = FIX(0.11400) * i   + ONE_HALF;
    rgb_ycc_tab[i + R_CB_OFF] = (-FIX(0.16874)) * i;
    rgb_ycc_tab[i + G_CB_OFF] = (-FIX(0.33126)) * i;
    rgb_ycc_tab[i + B_CB_OFF] = FIX(0.50000) * i  + CBCR_OFFSET + ONE_HALF - 1;
    rgb_ycc_tab[i + G_CR_OFF] = (-FIX(0.41869)) * i;
    rgb_ycc_tab[i + B_CR_OFF] = (-FIX(0.08131)) * i;
  }
#else
  ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
}

METHODDEF(void)
cmyk_ycck_convert(j_compress_ptr cinfo, _JSAMPARRAY input_buf,
                  _JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows)
{
#if BITS_IN_JSAMPLE != 16
  my_cconvert_ptr cconvert = (my_cconvert_ptr)cinfo->cconvert;
  register int r, g, b;
  register JLONG *ctab = cconvert->rgb_ycc_tab;
  register _JSAMPROW inptr;
  register _JSAMPROW outptr0, outptr1, outptr2, outptr3;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr0 = output_buf[0][output_row];
    outptr1 = output_buf[1][output_row];
    outptr2 = output_buf[2][output_row];
    outptr3 = output_buf[3][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {
      r = _MAXJSAMPLE - RANGE_LIMIT(inptr[0]);
      g = _MAXJSAMPLE - RANGE_LIMIT(inptr[1]);
      b = _MAXJSAMPLE - RANGE_LIMIT(inptr[2]);
      outptr3[col] = inptr[3];
      inptr += 4;
      /* Y */
      outptr0[col] = (_JSAMPLE)((ctab[r + R_Y_OFF] + ctab[g + G_Y_OFF] +
                                 ctab[b + B_Y_OFF]) >> SCALEBITS);
      /* Cb */
      outptr1[col] = (_JSAMPLE)((ctab[r + R_CB_OFF] + ctab[g + G_CB_OFF] +
                                 ctab[b + B_CB_OFF]) >> SCALEBITS);
      /* Cr */
      outptr2[col] = (_JSAMPLE)((ctab[r + R_CR_OFF] + ctab[g + G_CR_OFF] +
                                 ctab[b + B_CR_OFF]) >> SCALEBITS);
    }
  }
#else
  ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
}

METHODDEF(void)
grayscale_convert(j_compress_ptr cinfo, _JSAMPARRAY input_buf,
                  _JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows)
{
  register _JSAMPROW inptr;
  register _JSAMPROW outptr;
  register JDIMENSION col;
  JDIMENSION num_cols = cinfo->image_width;
  int instride = cinfo->input_components;

  while (--num_rows >= 0) {
    inptr = *input_buf++;
    outptr = output_buf[0][output_row];
    output_row++;
    for (col = 0; col < num_cols; col++) {
      outptr[col] = inptr[0];
      inptr += instride;
    }
  }
}

METHODDEF(void)
null_convert(j_compress_ptr cinfo, _JSAMPARRAY input_buf,
             _JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows)
{
  register _JSAMPROW inptr;
  register _JSAMPROW outptr, outptr0, outptr1, outptr2, outptr3;
  register JDIMENSION col;
  register int ci;
  int nc = cinfo->num_components;
  JDIMENSION num_cols = cinfo->image_width;

  if (nc == 3) {
    while (--num_rows >= 0) {
      inptr = *input_buf++;
      outptr0 = output_buf[0][output_row];
      outptr1 = output_buf[1][output_row];
      outptr2 = output_buf[2][output_row];
      output_row++;
      for (col = 0; col < num_cols; col++) {
        outptr0[col] = *inptr++;
        outptr1[col] = *inptr++;
        outptr2[col] = *inptr++;
      }
    }
  } else if (nc == 4) {
    while (--num_rows >= 0) {
      inptr = *input_buf++;
      outptr0 = output_buf[0][output_row];
      outptr1 = output_buf[1][output_row];
      outptr2 = output_buf[2][output_row];
      outptr3 = output_buf[3][output_row];
      output_row++;
      for (col = 0; col < num_cols; col++) {
        outptr0[col] = *inptr++;
        outptr1[col] = *inptr++;
        outptr2[col] = *inptr++;
        outptr3[col] = *inptr++;
      }
    }
  } else {
    while (--num_rows >= 0) {
      for (ci = 0; ci < nc; ci++) {
        inptr = *input_buf;
        outptr = output_buf[ci][output_row];
        for (col = 0; col < num_cols; col++) {
          outptr[col] = inptr[ci];
          inptr += nc;
        }
      }
      input_buf++;
      output_row++;
    }
  }
}

METHODDEF(void)
null_method(j_compress_ptr cinfo)
{
}

GLOBAL(void)
_jinit_color_converter(j_compress_ptr cinfo)
{
  my_cconvert_ptr cconvert;

#ifdef C_LOSSLESS_SUPPORTED
  if (cinfo->master->lossless) {
#if BITS_IN_JSAMPLE == 8
    if (cinfo->data_precision > BITS_IN_JSAMPLE || cinfo->data_precision < 2)
#else
    if (cinfo->data_precision > BITS_IN_JSAMPLE ||
        cinfo->data_precision < BITS_IN_JSAMPLE - 3)
#endif
      ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
  } else
#endif
  {
    if (cinfo->data_precision != BITS_IN_JSAMPLE)
      ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
  }

  cconvert = (my_cconvert_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(my_color_converter));
  cinfo->cconvert = (struct jpeg_color_converter *)cconvert;
  cconvert->pub.start_pass = null_method;

  switch (cinfo->in_color_space) {
  case JCS_GRAYSCALE:
    if (cinfo->input_components != 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_RGB:
  case JCS_EXT_RGB:
  case JCS_EXT_RGBX:
  case JCS_EXT_BGR:
  case JCS_EXT_BGRX:
  case JCS_EXT_XBGR:
  case JCS_EXT_XRGB:
  case JCS_EXT_RGBA:
  case JCS_EXT_BGRA:
  case JCS_EXT_ABGR:
  case JCS_EXT_ARGB:
    if (cinfo->input_components != rgb_pixelsize[cinfo->in_color_space])
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_YCbCr:
    if (cinfo->input_components != 3)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  case JCS_CMYK:
  case JCS_YCCK:
    if (cinfo->input_components != 4)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;

  default:
    if (cinfo->input_components < 1)
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    break;
  }

  switch (cinfo->jpeg_color_space) {
  case JCS_GRAYSCALE:
#ifdef C_LOSSLESS_SUPPORTED
    if (cinfo->master->lossless &&
        cinfo->in_color_space != cinfo->jpeg_color_space)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
    if (cinfo->num_components != 1)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_GRAYSCALE)
      cconvert->pub._color_convert = grayscale_convert;
    else if (IsExtRGB(cinfo->in_color_space)) {
#ifdef WITH_SIMD
      if (jsimd_set_rgb_gray(cinfo))
        cconvert->pub._color_convert = jsimd_color_convert;
      else
#endif
      {
        cconvert->pub.start_pass = rgb_ycc_start;
        switch (cinfo->in_color_space) {
        case JCS_EXT_RGB:
          cconvert->pub._color_convert = extrgb_gray_convert;
          break;
        case JCS_EXT_RGBX:
        case JCS_EXT_RGBA:
          cconvert->pub._color_convert = extrgbx_gray_convert;
          break;
        case JCS_EXT_BGR:
          cconvert->pub._color_convert = extbgr_gray_convert;
          break;
        case JCS_EXT_BGRX:
        case JCS_EXT_BGRA:
          cconvert->pub._color_convert = extbgrx_gray_convert;
          break;
        case JCS_EXT_XBGR:
        case JCS_EXT_ABGR:
          cconvert->pub._color_convert = extxbgr_gray_convert;
          break;
        case JCS_EXT_XRGB:
        case JCS_EXT_ARGB:
          cconvert->pub._color_convert = extxrgb_gray_convert;
          break;
        default:
          cconvert->pub._color_convert = rgb_gray_convert;
        }
      }
    } else if (cinfo->in_color_space == JCS_YCbCr)
      cconvert->pub._color_convert = grayscale_convert;
    else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_RGB:
#ifdef C_LOSSLESS_SUPPORTED
    if (cinfo->master->lossless && !IsExtRGB(cinfo->in_color_space))
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (rgb_red[cinfo->in_color_space] == 0 &&
        rgb_green[cinfo->in_color_space] == 1 &&
        rgb_blue[cinfo->in_color_space] == 2 &&
        rgb_pixelsize[cinfo->in_color_space] == 3) {
      cconvert->pub._color_convert = null_convert;
    } else if (IsExtRGB(cinfo->in_color_space)) {
      switch (cinfo->in_color_space) {
      case JCS_EXT_RGB:
        cconvert->pub._color_convert = extrgb_rgb_convert;
        break;
      case JCS_EXT_RGBX:
      case JCS_EXT_RGBA:
        cconvert->pub._color_convert = extrgbx_rgb_convert;
        break;
      case JCS_EXT_BGR:
        cconvert->pub._color_convert = extbgr_rgb_convert;
        break;
      case JCS_EXT_BGRX:
      case JCS_EXT_BGRA:
        cconvert->pub._color_convert = extbgrx_rgb_convert;
        break;
      case JCS_EXT_XBGR:
      case JCS_EXT_ABGR:
        cconvert->pub._color_convert = extxbgr_rgb_convert;
        break;
      case JCS_EXT_XRGB:
      case JCS_EXT_ARGB:
        cconvert->pub._color_convert = extxrgb_rgb_convert;
        break;
      default:
        cconvert->pub._color_convert = rgb_rgb_convert;
      }
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCbCr:
#ifdef C_LOSSLESS_SUPPORTED
    if (cinfo->master->lossless &&
        cinfo->in_color_space != cinfo->jpeg_color_space)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
    if (cinfo->num_components != 3)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (IsExtRGB(cinfo->in_color_space)) {
#ifdef WITH_SIMD
      if (jsimd_set_rgb_ycc(cinfo))
        cconvert->pub._color_convert = jsimd_color_convert;
      else
#endif
      {
        cconvert->pub.start_pass = rgb_ycc_start;
        switch (cinfo->in_color_space) {
        case JCS_EXT_RGB:
          cconvert->pub._color_convert = extrgb_ycc_convert;
          break;
        case JCS_EXT_RGBX:
        case JCS_EXT_RGBA:
          cconvert->pub._color_convert = extrgbx_ycc_convert;
          break;
        case JCS_EXT_BGR:
          cconvert->pub._color_convert = extbgr_ycc_convert;
          break;
        case JCS_EXT_BGRX:
        case JCS_EXT_BGRA:
          cconvert->pub._color_convert = extbgrx_ycc_convert;
          break;
        case JCS_EXT_XBGR:
        case JCS_EXT_ABGR:
          cconvert->pub._color_convert = extxbgr_ycc_convert;
          break;
        case JCS_EXT_XRGB:
        case JCS_EXT_ARGB:
          cconvert->pub._color_convert = extxrgb_ycc_convert;
          break;
        default:
          cconvert->pub._color_convert = rgb_ycc_convert;
        }
      }
    } else if (cinfo->in_color_space == JCS_YCbCr) {
      cconvert->pub._color_convert = null_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_CMYK:
#ifdef C_LOSSLESS_SUPPORTED
    if (cinfo->master->lossless &&
        cinfo->in_color_space != cinfo->jpeg_color_space)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK) {
      cconvert->pub._color_convert = null_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  case JCS_YCCK:
#ifdef C_LOSSLESS_SUPPORTED
    if (cinfo->master->lossless &&
        cinfo->in_color_space != cinfo->jpeg_color_space)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
#endif
    if (cinfo->num_components != 4)
      ERREXIT(cinfo, JERR_BAD_J_COLORSPACE);
    if (cinfo->in_color_space == JCS_CMYK) {
      cconvert->pub.start_pass = rgb_ycc_start;
      cconvert->pub._color_convert = cmyk_ycck_convert;
    } else if (cinfo->in_color_space == JCS_YCCK) {
      cconvert->pub._color_convert = null_convert;
    } else
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    break;

  default:
    if (cinfo->jpeg_color_space != cinfo->in_color_space ||
        cinfo->num_components != cinfo->input_components)
      ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);
    cconvert->pub._color_convert = null_convert;
    break;
  }
}

#endif /* BITS_IN_JSAMPLE != 16 || defined(C_LOSSLESS_SUPPORTED) */
