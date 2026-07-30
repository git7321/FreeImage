/*
 * jquant1.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2009, 2015, 2022-2023, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains 1-pass color quantization (color mapping) routines.
 * These routines provide mapping to a fixed color map using equally spaced
 * color values.  Optional Floyd-Steinberg or ordered dithering is available.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jsamplecomp.h"

#if defined(QUANT_1PASS_SUPPORTED) && BITS_IN_JSAMPLE != 16

#define ODITHER_SIZE  16        /* dimension of dither matrix */
#define ODITHER_CELLS  (ODITHER_SIZE * ODITHER_SIZE) /* # cells in matrix */
#define ODITHER_MASK  (ODITHER_SIZE - 1) /* mask for wrapping around
                                            counters */

typedef int ODITHER_MATRIX[ODITHER_SIZE][ODITHER_SIZE];
typedef int (*ODITHER_MATRIX_PTR)[ODITHER_SIZE];

static const UINT8 base_dither_matrix[ODITHER_SIZE][ODITHER_SIZE] = {
  {   0,192, 48,240, 12,204, 60,252,  3,195, 51,243, 15,207, 63,255 },
  { 128, 64,176,112,140, 76,188,124,131, 67,179,115,143, 79,191,127 },
  {  32,224, 16,208, 44,236, 28,220, 35,227, 19,211, 47,239, 31,223 },
  { 160, 96,144, 80,172,108,156, 92,163, 99,147, 83,175,111,159, 95 },
  {   8,200, 56,248,  4,196, 52,244, 11,203, 59,251,  7,199, 55,247 },
  { 136, 72,184,120,132, 68,180,116,139, 75,187,123,135, 71,183,119 },
  {  40,232, 24,216, 36,228, 20,212, 43,235, 27,219, 39,231, 23,215 },
  { 168,104,152, 88,164,100,148, 84,171,107,155, 91,167,103,151, 87 },
  {   2,194, 50,242, 14,206, 62,254,  1,193, 49,241, 13,205, 61,253 },
  { 130, 66,178,114,142, 78,190,126,129, 65,177,113,141, 77,189,125 },
  {  34,226, 18,210, 46,238, 30,222, 33,225, 17,209, 45,237, 29,221 },
  { 162, 98,146, 82,174,110,158, 94,161, 97,145, 81,173,109,157, 93 },
  {  10,202, 58,250,  6,198, 54,246,  9,201, 57,249,  5,197, 53,245 },
  { 138, 74,186,122,134, 70,182,118,137, 73,185,121,133, 69,181,117 },
  {  42,234, 26,218, 38,230, 22,214, 41,233, 25,217, 37,229, 21,213 },
  { 170,106,154, 90,166,102,150, 86,169,105,153, 89,165,101,149, 85 }
};

#if BITS_IN_JSAMPLE == 8
typedef INT16 FSERROR;          /* 16 bits should be enough */
typedef int LOCFSERROR;         /* use 'int' for calculation temps */
#else
typedef JLONG FSERROR;          /* may need more than 16 bits */
typedef JLONG LOCFSERROR;       /* be sure calculation temps are big enough */
#endif

typedef FSERROR *FSERRPTR;      /* pointer to error array */

#define MAX_Q_COMPS  4          /* max components I can handle */

typedef struct {
  struct jpeg_color_quantizer pub; /* public fields */

  /* Initially allocated colormap is saved here */
  _JSAMPARRAY sv_colormap;      /* The color map as a 2-D pixel array */
  int sv_actual;                /* number of entries in use */

  _JSAMPARRAY colorindex;       /* Precomputed mapping for speed */
  boolean is_padded;            /* is the colorindex padded for odither? */

  int Ncolors[MAX_Q_COMPS];     /* # of values allocated to each component */

  /* Variables for ordered dithering */
  int row_index;                /* cur row's vertical index in dither matrix */
  ODITHER_MATRIX_PTR odither[MAX_Q_COMPS]; /* one dither array per component */

  /* Variables for Floyd-Steinberg dithering */
  FSERRPTR fserrors[MAX_Q_COMPS]; /* accumulated errors */
  boolean on_odd_row;           /* flag to remember which row we are on */
} my_cquantizer;

typedef my_cquantizer *my_cquantize_ptr;

LOCAL(int)
select_ncolors(j_decompress_ptr cinfo, int Ncolors[])
{
  int nc = cinfo->out_color_components; /* number of color components */
  int max_colors = cinfo->desired_number_of_colors;
  int total_colors, iroot, i, j;
  boolean changed;
  long temp;
  int RGB_order[3] = { RGB_GREEN, RGB_RED, RGB_BLUE };
  RGB_order[0] = rgb_green[cinfo->out_color_space];
  RGB_order[1] = rgb_red[cinfo->out_color_space];
  RGB_order[2] = rgb_blue[cinfo->out_color_space];

  iroot = 1;
  do {
    iroot++;
    temp = iroot;               /* set temp = iroot ** nc */
    for (i = 1; i < nc; i++)
      temp *= iroot;
  } while (temp <= (long)max_colors); /* repeat till iroot exceeds root */
  iroot--;                      /* now iroot = floor(root) */

  /* Must have at least 2 color values per component */
  if (iroot < 2)
    ERREXIT1(cinfo, JERR_QUANT_FEW_COLORS, (int)temp);

  /* Initialize to iroot color values for each component */
  total_colors = 1;
  for (i = 0; i < nc; i++) {
    Ncolors[i] = iroot;
    total_colors *= iroot;
  }
  do {
    changed = FALSE;
    for (i = 0; i < nc; i++) {
      j = (cinfo->out_color_space == JCS_RGB ? RGB_order[i] : i);
      /* calculate new total_colors if Ncolors[j] is incremented */
      temp = total_colors / Ncolors[j];
      temp *= Ncolors[j] + 1;   /* done in long arith to avoid oflo */
      if (temp > (long)max_colors)
        break;                  /* won't fit, done with this pass */
      Ncolors[j]++;             /* OK, apply the increment */
      total_colors = (int)temp;
      changed = TRUE;
    }
  } while (changed);

  return total_colors;
}

LOCAL(int)
output_value(j_decompress_ptr cinfo, int ci, int j, int maxj)
{
  return (int)(((JLONG)j * _MAXJSAMPLE + maxj / 2) / maxj);
}

LOCAL(int)
largest_input_value(j_decompress_ptr cinfo, int ci, int j, int maxj)
{
  return (int)(((JLONG)(2 * j + 1) * _MAXJSAMPLE + maxj) / (2 * maxj));
}

LOCAL(void)
create_colormap(j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  _JSAMPARRAY colormap;         /* Created colormap */
  int total_colors;             /* Number of distinct output colors */
  int i, j, k, nci, blksize, blkdist, ptr, val;

  /* Select number of colors for each component */
  total_colors = select_ncolors(cinfo, cquantize->Ncolors);

  /* Report selected color counts */
  if (cinfo->out_color_components == 3)
    TRACEMS4(cinfo, 1, JTRC_QUANT_3_NCOLORS, total_colors,
             cquantize->Ncolors[0], cquantize->Ncolors[1],
             cquantize->Ncolors[2]);
  else
    TRACEMS1(cinfo, 1, JTRC_QUANT_NCOLORS, total_colors);

  colormap = (_JSAMPARRAY)(*cinfo->mem->alloc_sarray)
    ((j_common_ptr)cinfo, JPOOL_IMAGE,
     (JDIMENSION)total_colors, (JDIMENSION)cinfo->out_color_components);

  blkdist = total_colors;

  for (i = 0; i < cinfo->out_color_components; i++) {
    /* fill in colormap entries for i'th color component */
    nci = cquantize->Ncolors[i];
    blksize = blkdist / nci;
    for (j = 0; j < nci; j++) {
      /* Compute j'th output value (out of nci) for component */
      val = output_value(cinfo, i, j, nci - 1);
      /* Fill in all colormap entries that have this value of this component */
      for (ptr = j * blksize; ptr < total_colors; ptr += blkdist) {
        /* fill in blksize entries beginning at ptr */
        for (k = 0; k < blksize; k++)
          colormap[i][ptr + k] = (_JSAMPLE)val;
      }
    }
    blkdist = blksize;
  }

  cquantize->sv_colormap = colormap;
  cquantize->sv_actual = total_colors;
}

LOCAL(void)
create_colorindex(j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  _JSAMPROW indexptr;
  int i, j, k, nci, blksize, val, pad;

  if (cinfo->dither_mode == JDITHER_ORDERED) {
    pad = _MAXJSAMPLE * 2;
    cquantize->is_padded = TRUE;
  } else {
    pad = 0;
    cquantize->is_padded = FALSE;
  }

  cquantize->colorindex = (_JSAMPARRAY)(*cinfo->mem->alloc_sarray)
    ((j_common_ptr)cinfo, JPOOL_IMAGE,
     (JDIMENSION)(_MAXJSAMPLE + 1 + pad),
     (JDIMENSION)cinfo->out_color_components);

  /* blksize is number of adjacent repeated entries for a component */
  blksize = cquantize->sv_actual;

  for (i = 0; i < cinfo->out_color_components; i++) {
    /* fill in colorindex entries for i'th color component */
    nci = cquantize->Ncolors[i];
    blksize = blksize / nci;

    /* adjust colorindex pointers to provide padding at negative indexes. */
    if (pad)
      cquantize->colorindex[i] += _MAXJSAMPLE;

    indexptr = cquantize->colorindex[i];
    val = 0;
    k = largest_input_value(cinfo, i, 0, nci - 1);
    for (j = 0; j <= _MAXJSAMPLE; j++) {
      while (j > k)
        k = largest_input_value(cinfo, i, ++val, nci - 1);
      indexptr[j] = (_JSAMPLE)(val * blksize);
    }
    if (pad)
      for (j = 1; j <= _MAXJSAMPLE; j++) {
        indexptr[-j] = indexptr[0];
        indexptr[_MAXJSAMPLE + j] = indexptr[_MAXJSAMPLE];
      }
  }
}

LOCAL(ODITHER_MATRIX_PTR)
make_odither_array(j_decompress_ptr cinfo, int ncolors)
{
  ODITHER_MATRIX_PTR odither;
  int j, k;
  JLONG num, den;

  odither = (ODITHER_MATRIX_PTR)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(ODITHER_MATRIX));
  den = 2 * ODITHER_CELLS * ((JLONG)(ncolors - 1));
  for (j = 0; j < ODITHER_SIZE; j++) {
    for (k = 0; k < ODITHER_SIZE; k++) {
      num = ((JLONG)(ODITHER_CELLS - 1 -
                     2 * ((int)base_dither_matrix[j][k]))) * _MAXJSAMPLE;
      odither[j][k] = (int)(num < 0 ? -((-num) / den) : num / den);
    }
  }
  return odither;
}

LOCAL(void)
create_odither_tables(j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  ODITHER_MATRIX_PTR odither;
  int i, j, nci;

  for (i = 0; i < cinfo->out_color_components; i++) {
    nci = cquantize->Ncolors[i];
    odither = NULL;
    for (j = 0; j < i; j++) {
      if (nci == cquantize->Ncolors[j]) {
        odither = cquantize->odither[j];
        break;
      }
    }
    if (odither == NULL)
      odither = make_odither_array(cinfo, nci);
    cquantize->odither[i] = odither;
  }
}

METHODDEF(void)
color_quantize(j_decompress_ptr cinfo, _JSAMPARRAY input_buf,
               _JSAMPARRAY output_buf, int num_rows)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  _JSAMPARRAY colorindex = cquantize->colorindex;
  register int pixcode, ci;
  register _JSAMPROW ptrin, ptrout;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  register int nc = cinfo->out_color_components;

  for (row = 0; row < num_rows; row++) {
    ptrin = input_buf[row];
    ptrout = output_buf[row];
    for (col = width; col > 0; col--) {
      pixcode = 0;
      for (ci = 0; ci < nc; ci++) {
        pixcode += colorindex[ci][*ptrin++];
      }
      *ptrout++ = (_JSAMPLE)pixcode;
    }
  }
}

METHODDEF(void)
color_quantize3(j_decompress_ptr cinfo, _JSAMPARRAY input_buf,
                _JSAMPARRAY output_buf, int num_rows)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  register int pixcode;
  register _JSAMPROW ptrin, ptrout;
  _JSAMPROW colorindex0 = cquantize->colorindex[0];
  _JSAMPROW colorindex1 = cquantize->colorindex[1];
  _JSAMPROW colorindex2 = cquantize->colorindex[2];
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    ptrin = input_buf[row];
    ptrout = output_buf[row];
    for (col = width; col > 0; col--) {
      pixcode  = colorindex0[*ptrin++];
      pixcode += colorindex1[*ptrin++];
      pixcode += colorindex2[*ptrin++];
      *ptrout++ = (_JSAMPLE)pixcode;
    }
  }
}

METHODDEF(void)
quantize_ord_dither(j_decompress_ptr cinfo, _JSAMPARRAY input_buf,
                    _JSAMPARRAY output_buf, int num_rows)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  register _JSAMPROW input_ptr;
  register _JSAMPROW output_ptr;
  _JSAMPROW colorindex_ci;
  int *dither;                  /* points to active row of dither matrix */
  int row_index, col_index;     /* current indexes into dither matrix */
  int nc = cinfo->out_color_components;
  int ci;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    /* Initialize output values to 0 so can process components separately */
    jzero_far((void *)output_buf[row], (size_t)(width * sizeof(_JSAMPLE)));
    row_index = cquantize->row_index;
    for (ci = 0; ci < nc; ci++) {
      input_ptr = input_buf[row] + ci;
      output_ptr = output_buf[row];
      colorindex_ci = cquantize->colorindex[ci];
      dither = cquantize->odither[ci][row_index];
      col_index = 0;

      for (col = width; col > 0; col--) {
        *output_ptr +=
          colorindex_ci[*input_ptr + dither[col_index]];
        input_ptr += nc;
        output_ptr++;
        col_index = (col_index + 1) & ODITHER_MASK;
      }
    }
    /* Advance row index for next row */
    row_index = (row_index + 1) & ODITHER_MASK;
    cquantize->row_index = row_index;
  }
}

METHODDEF(void)
quantize3_ord_dither(j_decompress_ptr cinfo, _JSAMPARRAY input_buf,
                     _JSAMPARRAY output_buf, int num_rows)
/* Fast path for out_color_components==3, with ordered dithering */
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  register int pixcode;
  register _JSAMPROW input_ptr;
  register _JSAMPROW output_ptr;
  _JSAMPROW colorindex0 = cquantize->colorindex[0];
  _JSAMPROW colorindex1 = cquantize->colorindex[1];
  _JSAMPROW colorindex2 = cquantize->colorindex[2];
  int *dither0;                 /* points to active row of dither matrix */
  int *dither1;
  int *dither2;
  int row_index, col_index;     /* current indexes into dither matrix */
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;

  for (row = 0; row < num_rows; row++) {
    row_index = cquantize->row_index;
    input_ptr = input_buf[row];
    output_ptr = output_buf[row];
    dither0 = cquantize->odither[0][row_index];
    dither1 = cquantize->odither[1][row_index];
    dither2 = cquantize->odither[2][row_index];
    col_index = 0;

    for (col = width; col > 0; col--) {
      pixcode  = colorindex0[(*input_ptr++) + dither0[col_index]];
      pixcode += colorindex1[(*input_ptr++) + dither1[col_index]];
      pixcode += colorindex2[(*input_ptr++) + dither2[col_index]];
      *output_ptr++ = (_JSAMPLE)pixcode;
      col_index = (col_index + 1) & ODITHER_MASK;
    }
    row_index = (row_index + 1) & ODITHER_MASK;
    cquantize->row_index = row_index;
  }
}

METHODDEF(void)
quantize_fs_dither(j_decompress_ptr cinfo, _JSAMPARRAY input_buf,
                   _JSAMPARRAY output_buf, int num_rows)
/* General case, with Floyd-Steinberg dithering */
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  register LOCFSERROR cur;      /* current error or pixel value */
  LOCFSERROR belowerr;          /* error for pixel below cur */
  LOCFSERROR bpreverr;          /* error for below/prev col */
  LOCFSERROR bnexterr;          /* error for below/next col */
  LOCFSERROR delta;
  register FSERRPTR errorptr;   /* => fserrors[] at column before current */
  register _JSAMPROW input_ptr;
  register _JSAMPROW output_ptr;
  _JSAMPROW colorindex_ci;
  _JSAMPROW colormap_ci;
  int pixcode;
  int nc = cinfo->out_color_components;
  int dir;                      /* 1 for left-to-right, -1 for right-to-left */
  int dirnc;                    /* dir * nc */
  int ci;
  int row;
  JDIMENSION col;
  JDIMENSION width = cinfo->output_width;
  _JSAMPLE *range_limit = (_JSAMPLE *)cinfo->sample_range_limit;
  SHIFT_TEMPS

  for (row = 0; row < num_rows; row++) {
    /* Initialize output values to 0 so can process components separately */
    jzero_far((void *)output_buf[row], (size_t)(width * sizeof(_JSAMPLE)));
    for (ci = 0; ci < nc; ci++) {
      input_ptr = input_buf[row] + ci;
      output_ptr = output_buf[row];
      if (cquantize->on_odd_row) {
        /* work right to left in this row */
        input_ptr += (width - 1) * nc;
        output_ptr += width - 1;
        dir = -1;
        dirnc = -nc;
        errorptr = cquantize->fserrors[ci] + (width + 1);
      } else {
        /* work left to right in this row */
        dir = 1;
        dirnc = nc;
        errorptr = cquantize->fserrors[ci];
      }
      colorindex_ci = cquantize->colorindex[ci];
      colormap_ci = cquantize->sv_colormap[ci];
      /* Preset error values: no error propagated to first pixel from left */
      cur = 0;
      /* and no error propagated to row below yet */
      belowerr = bpreverr = 0;

      for (col = width; col > 0; col--) {
        cur = RIGHT_SHIFT(cur + errorptr[dir] + 8, 4);
        cur += *input_ptr;
        cur = range_limit[cur];
        /* Select output value, accumulate into output code for this pixel */
        pixcode = colorindex_ci[cur];
        *output_ptr += (_JSAMPLE)pixcode;
        cur -= colormap_ci[pixcode];
        bnexterr = cur;
        delta = cur * 2;
        cur += delta;           /* form error * 3 */
        errorptr[0] = (FSERROR)(bpreverr + cur);
        cur += delta;           /* form error * 5 */
        bpreverr = belowerr + cur;
        belowerr = bnexterr;
        cur += delta;           /* form error * 7 */
        input_ptr += dirnc;     /* advance input ptr to next column */
        output_ptr += dir;      /* advance output ptr to next column */
        errorptr += dir;        /* advance errorptr to current column */
      }
      errorptr[0] = (FSERROR)bpreverr; /* unload prev err into array */
    }
    cquantize->on_odd_row = (cquantize->on_odd_row ? FALSE : TRUE);
  }
}

LOCAL(void)
alloc_fs_workspace(j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  size_t arraysize;
  int i;

  arraysize = (size_t)((cinfo->output_width + 2) * sizeof(FSERROR));
  for (i = 0; i < cinfo->out_color_components; i++) {
    cquantize->fserrors[i] = (FSERRPTR)
      (*cinfo->mem->alloc_large) ((j_common_ptr)cinfo, JPOOL_IMAGE, arraysize);
  }
}

METHODDEF(void)
start_pass_1_quant(j_decompress_ptr cinfo, boolean is_pre_scan)
{
  my_cquantize_ptr cquantize = (my_cquantize_ptr)cinfo->cquantize;
  size_t arraysize;
  int i;

  /* Install my colormap. */
  cinfo->colormap = (JSAMPARRAY)cquantize->sv_colormap;
  cinfo->actual_number_of_colors = cquantize->sv_actual;

  /* Initialize for desired dithering mode. */
  switch (cinfo->dither_mode) {
  case JDITHER_NONE:
    if (cinfo->out_color_components == 3)
      cquantize->pub._color_quantize = color_quantize3;
    else
      cquantize->pub._color_quantize = color_quantize;
    break;
  case JDITHER_ORDERED:
    if (cinfo->out_color_components == 3)
      cquantize->pub._color_quantize = quantize3_ord_dither;
    else
      cquantize->pub._color_quantize = quantize_ord_dither;
    cquantize->row_index = 0;
    if (!cquantize->is_padded)
      create_colorindex(cinfo);
    /* Create ordered-dither tables if we didn't already. */
    if (cquantize->odither[0] == NULL)
      create_odither_tables(cinfo);
    break;
  case JDITHER_FS:
    cquantize->pub._color_quantize = quantize_fs_dither;
    cquantize->on_odd_row = FALSE;
    /* Allocate Floyd-Steinberg workspace if didn't already. */
    if (cquantize->fserrors[0] == NULL)
      alloc_fs_workspace(cinfo);
    /* Initialize the propagated errors to zero. */
    arraysize = (size_t)((cinfo->output_width + 2) * sizeof(FSERROR));
    for (i = 0; i < cinfo->out_color_components; i++)
      jzero_far((void *)cquantize->fserrors[i], arraysize);
    break;
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
    break;
  }
}

METHODDEF(void)
finish_pass_1_quant(j_decompress_ptr cinfo)
{
}

METHODDEF(void)
new_color_map_1_quant(j_decompress_ptr cinfo)
{
  ERREXIT(cinfo, JERR_MODE_CHANGE);
}

GLOBAL(void)
_jinit_1pass_quantizer(j_decompress_ptr cinfo)
{
  my_cquantize_ptr cquantize;

  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  /* Color quantization is not supported with lossless JPEG images */
  if (cinfo->master->lossless)
    ERREXIT(cinfo, JERR_NOTIMPL);

  cquantize = (my_cquantize_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(my_cquantizer));
  cinfo->cquantize = (struct jpeg_color_quantizer *)cquantize;
  cquantize->pub.start_pass = start_pass_1_quant;
  cquantize->pub.finish_pass = finish_pass_1_quant;
  cquantize->pub.new_color_map = new_color_map_1_quant;
  cquantize->fserrors[0] = NULL;
  cquantize->odither[0] = NULL;

  /* Make sure my internal arrays won't overflow */
  if (cinfo->out_color_components > MAX_Q_COMPS)
    ERREXIT1(cinfo, JERR_QUANT_COMPONENTS, MAX_Q_COMPS);
  /* Make sure colormap indexes can be represented by _JSAMPLEs */
  if (cinfo->desired_number_of_colors > (_MAXJSAMPLE + 1))
    ERREXIT1(cinfo, JERR_QUANT_MANY_COLORS, _MAXJSAMPLE + 1);

  /* Create the colormap and color index table. */
  create_colormap(cinfo);
  create_colorindex(cinfo);

  if (cinfo->dither_mode == JDITHER_FS)
    alloc_fs_workspace(cinfo);
}

#endif /* defined(QUANT_1PASS_SUPPORTED) && BITS_IN_JSAMPLE != 16 */
