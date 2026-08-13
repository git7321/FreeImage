/*
 * jcomapi.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2024-2026, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains application interface routines that are used for both
 * compression and decompression.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#ifdef WITH_PROFILE
#include <stdio.h>
#include "tjutil.h"
#endif

GLOBAL(void)
jpeg_abort(j_common_ptr cinfo)
{
  int pool;

  if (cinfo->mem == NULL)
    return;

  for (pool = JPOOL_NUMPOOLS - 1; pool > JPOOL_PERMANENT; pool--) {
    (*cinfo->mem->free_pool) (cinfo, pool);
  }

  if (cinfo->is_decompressor) {
    cinfo->global_state = DSTATE_START;
    ((j_decompress_ptr)cinfo)->marker_list = NULL;
    ((j_decompress_ptr)cinfo)->master->marker_list_end = NULL;
  } else {
    cinfo->global_state = CSTATE_START;
  }

#ifdef WITH_PROFILE
  if (cinfo->is_decompressor) {
    if (((j_decompress_ptr)cinfo)->master->total_start > 0.0)
      ((j_decompress_ptr)cinfo)->master->total_elapsed +=
        getTime() - ((j_decompress_ptr)cinfo)->master->total_start;
  } else {
    if (((j_compress_ptr)cinfo)->master->total_start > 0.0)
      ((j_compress_ptr)cinfo)->master->total_elapsed +=
        getTime() - ((j_compress_ptr)cinfo)->master->total_start;
  }
#endif
}

GLOBAL(void)
jpeg_destroy(j_common_ptr cinfo)
{
#ifdef WITH_PROFILE
  if (cinfo->is_decompressor) {
    j_decompress_ptr dinfo = (j_decompress_ptr)cinfo;

    if (dinfo->master->entropy_mcoeffs > 0.0) {
      fprintf(stderr, "Entropy decoding:    %14.6f Mcoefficients/sec",
              dinfo->master->entropy_mcoeffs / dinfo->master->entropy_elapsed);
      if (dinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "  (%5.2f%% of total time)",
                dinfo->master->entropy_elapsed * 100.0 /
                  dinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (dinfo->master->idct_mcoeffs > 0.0) {
      fprintf(stderr, "Inverse DCT:         %14.6f Mcoefficients/sec",
              dinfo->master->idct_mcoeffs / dinfo->master->idct_elapsed);
      if (dinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "  (%5.2f%% of total time)",
                dinfo->master->idct_elapsed * 100.0 /
                  dinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (dinfo->master->merged_upsample_mpixels > 0.0) {
      fprintf(stderr, "Merged upsampling:   %14.6f Mpixels/sec",
              dinfo->master->merged_upsample_mpixels /
                dinfo->master->merged_upsample_elapsed);
      if (dinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "        (%5.2f%% of total time)",
                dinfo->master->merged_upsample_elapsed * 100.0 /
                  dinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (dinfo->master->upsample_msamples > 0.0) {
      fprintf(stderr, "Upsampling:          %14.6f Msamples/sec",
              dinfo->master->upsample_msamples /
                dinfo->master->upsample_elapsed);
      if (dinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "       (%5.2f%% of total time)",
                dinfo->master->upsample_elapsed * 100.0 /
                  dinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (dinfo->master->cconvert_mpixels > 0.0) {
      fprintf(stderr, "Color deconversion:  %14.6f Mpixels/sec",
              dinfo->master->cconvert_mpixels /
                dinfo->master->cconvert_elapsed);
      if (dinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "        (%5.2f%% of total time)",
                dinfo->master->cconvert_elapsed * 100.0 /
                  dinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
  } else {
    j_compress_ptr _cinfo = (j_compress_ptr)cinfo;

    if (_cinfo->master->cconvert_mpixels > 0.0) {
      fprintf(stderr, "Color conversion:    %14.6f Mpixels/sec",
              _cinfo->master->cconvert_mpixels /
                _cinfo->master->cconvert_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "        (%5.2f%% of total time)",
                _cinfo->master->cconvert_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (_cinfo->master->downsample_msamples > 0.0) {
      fprintf(stderr, "Downsampling:        %14.6f Msamples/sec",
              _cinfo->master->downsample_msamples /
                _cinfo->master->downsample_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "       (%5.2f%% of total time)",
                _cinfo->master->downsample_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (_cinfo->master->convsamp_msamples > 0.0) {
      fprintf(stderr, "Sample conversion:   %14.6f Msamples/sec",
              _cinfo->master->convsamp_msamples /
                _cinfo->master->convsamp_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "       (%5.2f%% of total time)",
                _cinfo->master->convsamp_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (_cinfo->master->fdct_mcoeffs > 0.0) {
      fprintf(stderr, "Forward DCT:         %14.6f Mcoefficients/sec",
              _cinfo->master->fdct_mcoeffs / _cinfo->master->fdct_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "  (%5.2f%% of total time)",
                _cinfo->master->fdct_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (_cinfo->master->quantize_mcoeffs > 0.0) {
      fprintf(stderr, "Quantization:        %14.6f Mcoefficients/sec",
              _cinfo->master->quantize_mcoeffs /
                _cinfo->master->quantize_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "  (%5.2f%% of total time)",
                _cinfo->master->quantize_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
    if (_cinfo->master->entropy_mcoeffs > 0.0) {
      fprintf(stderr, "Entropy encoding:    %14.6f Mcoefficients/sec",
              _cinfo->master->entropy_mcoeffs /
                _cinfo->master->entropy_elapsed);
      if (_cinfo->master->total_elapsed > 0.0)
        fprintf(stderr, "  (%5.2f%% of total time)",
                _cinfo->master->entropy_elapsed * 100.0 /
                  _cinfo->master->total_elapsed);
      fprintf(stderr, "\n");
    }
  }
#endif

  if (cinfo->mem != NULL)
    (*cinfo->mem->self_destruct) (cinfo);
  cinfo->mem = NULL;
  cinfo->global_state = 0;
}

GLOBAL(JQUANT_TBL *)
jpeg_alloc_quant_table(j_common_ptr cinfo)
{
  JQUANT_TBL *tbl;

  tbl = (JQUANT_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, sizeof(JQUANT_TBL));
  tbl->sent_table = FALSE;
  return tbl;
}

GLOBAL(JHUFF_TBL *)
jpeg_alloc_huff_table(j_common_ptr cinfo)
{
  JHUFF_TBL *tbl;

  tbl = (JHUFF_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, sizeof(JHUFF_TBL));
  tbl->sent_table = FALSE;
  return tbl;
}
