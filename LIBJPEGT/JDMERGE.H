/*
 * jdmerge.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2020, 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 */

#define JPEG_INTERNALS
#include "jpeglib.h"
#include "jsamplecomp.h"

#ifdef UPSAMPLE_MERGING_SUPPORTED

typedef struct {
  struct jpeg_upsampler pub;

  void (*upmethod) (j_decompress_ptr cinfo, _JSAMPIMAGE input_buf,
                    JDIMENSION in_row_group_ctr, _JSAMPARRAY output_buf);

  int *Cr_r_tab;                /* => table for Cr to R conversion */
  int *Cb_b_tab;                /* => table for Cb to B conversion */
  JLONG *Cr_g_tab;              /* => table for Cr to G conversion */
  JLONG *Cb_g_tab;              /* => table for Cb to G conversion */

  _JSAMPROW spare_row;
  boolean spare_full;           /* T if spare buffer is occupied */

  JDIMENSION out_row_width;     /* samples per output row */
  JDIMENSION rows_to_go;        /* counts rows remaining in image */
} my_merged_upsampler;

typedef my_merged_upsampler *my_merged_upsample_ptr;

#endif /* UPSAMPLE_MERGING_SUPPORTED */
