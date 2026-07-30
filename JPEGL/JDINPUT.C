/*
 * jdinput.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2010, 2016, 2018, 2022, D. R. Commander.
 * Copyright (C) 2015, Google, Inc.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains input control logic for the JPEG decompressor.
 * These routines are concerned with controlling the decompressor's input
 * processing (marker reading and coefficient/difference decoding).
 * The actual input reading is done in jdmarker.c, jdhuff.c, jdphuff.c,
 * and jdlhuff.c.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jpegapicomp.h"

typedef struct {
  struct jpeg_input_controller pub;

  boolean inheaders;
} my_input_controller;

typedef my_input_controller *my_inputctl_ptr;

/* Forward declarations */
METHODDEF(int) consume_markers(j_decompress_ptr cinfo);

LOCAL(void)
initial_setup(j_decompress_ptr cinfo)
/* Called once, when first SOS marker is reached */
{
  int ci;
  jpeg_component_info *compptr;
  int data_unit = cinfo->master->lossless ? 1 : DCTSIZE;

  /* Make sure image isn't bigger than I can handle */
  if ((long)cinfo->image_height > (long)JPEG_MAX_DIMENSION ||
      (long)cinfo->image_width > (long)JPEG_MAX_DIMENSION)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int)JPEG_MAX_DIMENSION);

  /* For now, precision must match compiled-in value... */
#ifdef D_LOSSLESS_SUPPORTED
  if (cinfo->data_precision != 8 && cinfo->data_precision != 12 &&
      cinfo->data_precision != 16)
#else
  if (cinfo->data_precision != 8 && cinfo->data_precision != 12)
#endif
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  /* Check that number of components won't exceed internal array sizes */
  if (cinfo->num_components > MAX_COMPONENTS)
    ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
             MAX_COMPONENTS);

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (compptr->h_samp_factor <= 0 ||
        compptr->h_samp_factor > MAX_SAMP_FACTOR ||
        compptr->v_samp_factor <= 0 ||
        compptr->v_samp_factor > MAX_SAMP_FACTOR)
      ERREXIT(cinfo, JERR_BAD_SAMPLING);
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
                                   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
                                   compptr->v_samp_factor);
  }

#if JPEG_LIB_VERSION >= 80
  cinfo->block_size = data_unit;
  cinfo->natural_order = jpeg_natural_order;
  cinfo->lim_Se = DCTSIZE2 - 1;
#endif

#if JPEG_LIB_VERSION >= 70
  cinfo->min_DCT_h_scaled_size = cinfo->min_DCT_v_scaled_size = data_unit;
#else
  cinfo->min_DCT_scaled_size = data_unit;
#endif

  /* Compute dimensions of components */
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
#if JPEG_LIB_VERSION >= 70
    compptr->DCT_h_scaled_size = compptr->DCT_v_scaled_size = data_unit;
#else
    compptr->DCT_scaled_size = data_unit;
#endif
    /* Size in data units */
    compptr->width_in_blocks = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_width * (long)compptr->h_samp_factor,
                    (long)(cinfo->max_h_samp_factor * data_unit));
    compptr->height_in_blocks = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_height * (long)compptr->v_samp_factor,
                    (long)(cinfo->max_v_samp_factor * data_unit));
    cinfo->master->first_MCU_col[ci] = 0;
    cinfo->master->last_MCU_col[ci] = compptr->width_in_blocks - 1;
    /* Size in samples */
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_width * (long)compptr->h_samp_factor,
                    (long)cinfo->max_h_samp_factor);
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_height * (long)compptr->v_samp_factor,
                    (long)cinfo->max_v_samp_factor);
    /* Mark component needed, until color conversion says otherwise */
    compptr->component_needed = TRUE;
    /* Mark no quantization table yet saved for component */
    compptr->quant_table = NULL;
  }

  /* Compute number of fully interleaved MCU rows. */
  cinfo->total_iMCU_rows = (JDIMENSION)
    jdiv_round_up((long)cinfo->image_height,
                  (long)(cinfo->max_v_samp_factor * data_unit));

  /* Decide whether file contains multiple scans */
  if (cinfo->comps_in_scan < cinfo->num_components || cinfo->progressive_mode)
    cinfo->inputctl->has_multiple_scans = TRUE;
  else
    cinfo->inputctl->has_multiple_scans = FALSE;
}

LOCAL(void)
per_scan_setup(j_decompress_ptr cinfo)
{
  int ci, mcublks, tmp;
  jpeg_component_info *compptr;
  int data_unit = cinfo->master->lossless ? 1 : DCTSIZE;

  if (cinfo->comps_in_scan == 1) {

    /* Noninterleaved (single-component) scan */
    compptr = cinfo->cur_comp_info[0];

    /* Overall image size in MCUs */
    cinfo->MCUs_per_row = compptr->width_in_blocks;
    cinfo->MCU_rows_in_scan = compptr->height_in_blocks;

    /* For noninterleaved scan, always one data unit per MCU */
    compptr->MCU_width = 1;
    compptr->MCU_height = 1;
    compptr->MCU_blocks = 1;
    compptr->MCU_sample_width = compptr->_DCT_scaled_size;
    compptr->last_col_width = 1;
    tmp = (int)(compptr->height_in_blocks % compptr->v_samp_factor);
    if (tmp == 0) tmp = compptr->v_samp_factor;
    compptr->last_row_height = tmp;

    /* Prepare array describing MCU composition */
    cinfo->blocks_in_MCU = 1;
    cinfo->MCU_membership[0] = 0;

  } else {

    /* Interleaved (multi-component) scan */
    if (cinfo->comps_in_scan <= 0 || cinfo->comps_in_scan > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->comps_in_scan,
               MAX_COMPS_IN_SCAN);

    /* Overall image size in MCUs */
    cinfo->MCUs_per_row = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_width,
                    (long)(cinfo->max_h_samp_factor * data_unit));
    cinfo->MCU_rows_in_scan = (JDIMENSION)
      jdiv_round_up((long)cinfo->image_height,
                    (long)(cinfo->max_v_samp_factor * data_unit));

    cinfo->blocks_in_MCU = 0;

    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
      /* Sampling factors give # of data units of component in each MCU */
      compptr->MCU_width = compptr->h_samp_factor;
      compptr->MCU_height = compptr->v_samp_factor;
      compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
      compptr->MCU_sample_width = compptr->MCU_width *
                                  compptr->_DCT_scaled_size;
      /* Figure number of non-dummy data units in last MCU column & row */
      tmp = (int)(compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int)(compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
      /* Prepare array describing MCU composition */
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > D_MAX_BLOCKS_IN_MCU)
        ERREXIT(cinfo, JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
        cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
  }
}

LOCAL(void)
latch_quant_tables(j_decompress_ptr cinfo)
{
  int ci, qtblno;
  jpeg_component_info *compptr;
  JQUANT_TBL *qtbl;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* No work if we already saved Q-table for this component */
    if (compptr->quant_table != NULL)
      continue;
    /* Make sure specified quantization table is present */
    qtblno = compptr->quant_tbl_no;
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
        cinfo->quant_tbl_ptrs[qtblno] == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, qtblno);
    /* OK, save away the quantization table */
    qtbl = (JQUANT_TBL *)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                  sizeof(JQUANT_TBL));
    memcpy(qtbl, cinfo->quant_tbl_ptrs[qtblno], sizeof(JQUANT_TBL));
    compptr->quant_table = qtbl;
  }
}

METHODDEF(void)
start_input_pass(j_decompress_ptr cinfo)
{
  per_scan_setup(cinfo);
  if (!cinfo->master->lossless)
    latch_quant_tables(cinfo);
  (*cinfo->entropy->start_pass) (cinfo);
  (*cinfo->coef->start_input_pass) (cinfo);
  cinfo->inputctl->consume_input = cinfo->coef->consume_data;
}

METHODDEF(void)
finish_input_pass(j_decompress_ptr cinfo)
{
  cinfo->inputctl->consume_input = consume_markers;
}

METHODDEF(int)
consume_markers(j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr)cinfo->inputctl;
  int val;

  if (inputctl->pub.eoi_reached) /* After hitting EOI, read no further */
    return JPEG_REACHED_EOI;

  val = (*cinfo->marker->read_markers) (cinfo);

  switch (val) {
  case JPEG_REACHED_SOS:        /* Found SOS */
    if (inputctl->inheaders) {  /* 1st SOS */
      initial_setup(cinfo);
      inputctl->inheaders = FALSE;
    } else {                    /* 2nd or later SOS marker */
      if (!inputctl->pub.has_multiple_scans)
      start_input_pass(cinfo);
    }
    break;
  case JPEG_REACHED_EOI:        /* Found EOI */
    inputctl->pub.eoi_reached = TRUE;
    if (inputctl->inheaders) {  /* Tables-only datastream, apparently */
      if (cinfo->marker->saw_SOF)
        ERREXIT(cinfo, JERR_SOF_NO_SOS);
    } else {
      if (cinfo->output_scan_number > cinfo->input_scan_number)
        cinfo->output_scan_number = cinfo->input_scan_number;
    }
    break;
  case JPEG_SUSPENDED:
    break;
  }

  return val;
}

METHODDEF(void)
reset_input_controller(j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr)cinfo->inputctl;

  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.has_multiple_scans = FALSE;
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
  /* Reset other modules */
  (*cinfo->err->reset_error_mgr) ((j_common_ptr)cinfo);
  (*cinfo->marker->reset_marker_reader) (cinfo);
  /* Reset progression state -- would be cleaner if entropy decoder did this */
  cinfo->coef_bits = NULL;
}

GLOBAL(void)
jinit_input_controller(j_decompress_ptr cinfo)
{
  my_inputctl_ptr inputctl;

  /* Create subobject in permanent pool */
  inputctl = (my_inputctl_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                sizeof(my_input_controller));
  cinfo->inputctl = (struct jpeg_input_controller *)inputctl;
  /* Initialize method pointers */
  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.reset_input_controller = reset_input_controller;
  inputctl->pub.start_input_pass = start_input_pass;
  inputctl->pub.finish_input_pass = finish_input_pass;
  inputctl->pub.has_multiple_scans = FALSE;
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
}
