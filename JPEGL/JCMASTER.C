/*
 * jcmaster.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 2003-2010 by Guido Vollbeding.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2010, 2016, 2018, 2022-2023, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains master control logic for the JPEG compressor.
 * These routines are concerned with parameter validation, initial setup,
 * and inter-pass control (determining the number of passes and the work
 * to be done in each pass).
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jpegapicomp.h"
#include "jcmaster.h"

#if JPEG_LIB_VERSION >= 70

GLOBAL(void)
jpeg_calc_jpeg_dimensions(j_compress_ptr cinfo)
/* Do computations that are needed before master selection phase */
{
  int data_unit = cinfo->master->lossless ? 1 : DCTSIZE;

  cinfo->jpeg_width = cinfo->image_width;
  cinfo->jpeg_height = cinfo->image_height;
  cinfo->min_DCT_h_scaled_size = data_unit;
  cinfo->min_DCT_v_scaled_size = data_unit;
}
#endif

LOCAL(void)
initial_setup(j_compress_ptr cinfo, boolean transcode_only)
/* Do computations that are needed before master selection phase */
{
  int ci;
  jpeg_component_info *compptr;
  long samplesperrow;
  JDIMENSION jd_samplesperrow;
  int data_unit = cinfo->master->lossless ? 1 : DCTSIZE;

#if JPEG_LIB_VERSION >= 70
#if JPEG_LIB_VERSION >= 80
  if (!transcode_only)
#endif
    jpeg_calc_jpeg_dimensions(cinfo);
#endif

  /* Sanity check on image dimensions */
  if (cinfo->_jpeg_height <= 0 || cinfo->_jpeg_width <= 0 ||
      cinfo->num_components <= 0 || cinfo->input_components <= 0)
    ERREXIT(cinfo, JERR_EMPTY_IMAGE);

  /* Make sure image isn't bigger than I can handle */
  if ((long)cinfo->_jpeg_height > (long)JPEG_MAX_DIMENSION ||
      (long)cinfo->_jpeg_width > (long)JPEG_MAX_DIMENSION)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int)JPEG_MAX_DIMENSION);

  /* Width of an input scanline must be representable as JDIMENSION. */
  samplesperrow = (long)cinfo->image_width * (long)cinfo->input_components;
  jd_samplesperrow = (JDIMENSION)samplesperrow;
  if ((long)jd_samplesperrow != samplesperrow)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);

#ifdef C_LOSSLESS_SUPPORTED
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

  /* Compute dimensions of components */
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    /* Fill in the correct component_index value; don't rely on application */
    compptr->component_index = ci;
    /* For compression, we never do DCT scaling. */
#if JPEG_LIB_VERSION >= 70
    compptr->DCT_h_scaled_size = compptr->DCT_v_scaled_size = data_unit;
#else
    compptr->DCT_scaled_size = data_unit;
#endif
    /* Size in data units */
    compptr->width_in_blocks = (JDIMENSION)
      jdiv_round_up((long)cinfo->_jpeg_width * (long)compptr->h_samp_factor,
                    (long)(cinfo->max_h_samp_factor * data_unit));
    compptr->height_in_blocks = (JDIMENSION)
      jdiv_round_up((long)cinfo->_jpeg_height * (long)compptr->v_samp_factor,
                    (long)(cinfo->max_v_samp_factor * data_unit));
    /* Size in samples */
    compptr->downsampled_width = (JDIMENSION)
      jdiv_round_up((long)cinfo->_jpeg_width * (long)compptr->h_samp_factor,
                    (long)cinfo->max_h_samp_factor);
    compptr->downsampled_height = (JDIMENSION)
      jdiv_round_up((long)cinfo->_jpeg_height * (long)compptr->v_samp_factor,
                    (long)cinfo->max_v_samp_factor);
    /* Mark component needed (this flag isn't actually used for compression) */
    compptr->component_needed = TRUE;
  }

  cinfo->total_iMCU_rows = (JDIMENSION)
    jdiv_round_up((long)cinfo->_jpeg_height,
                  (long)(cinfo->max_v_samp_factor * data_unit));
}

#if defined(C_MULTISCAN_FILES_SUPPORTED) || defined(C_LOSSLESS_SUPPORTED)
#define NEED_SCAN_SCRIPT
#endif

#ifdef NEED_SCAN_SCRIPT

LOCAL(void)
validate_script(j_compress_ptr cinfo)
{
  const jpeg_scan_info *scanptr;
  int scanno, ncomps, ci, coefi, thisi;
  int Ss, Se, Ah, Al;
  boolean component_sent[MAX_COMPONENTS];
#ifdef C_PROGRESSIVE_SUPPORTED
  int *last_bitpos_ptr;
  int last_bitpos[MAX_COMPONENTS][DCTSIZE2];
#endif

  if (cinfo->num_scans <= 0)
    ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, 0);

#ifndef C_MULTISCAN_FILES_SUPPORTED
  if (cinfo->num_scans > 1)
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif

  scanptr = cinfo->scan_info;
  if (scanptr->Ss != 0 && scanptr->Se == 0) {
#ifdef C_LOSSLESS_SUPPORTED
    cinfo->master->lossless = TRUE;
    cinfo->progressive_mode = FALSE;
    for (ci = 0; ci < cinfo->num_components; ci++)
      component_sent[ci] = FALSE;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  }
  else if (scanptr->Ss != 0 || scanptr->Se != DCTSIZE2 - 1) {
#ifdef C_PROGRESSIVE_SUPPORTED
    cinfo->progressive_mode = TRUE;
    cinfo->master->lossless = FALSE;
    last_bitpos_ptr = &last_bitpos[0][0];
    for (ci = 0; ci < cinfo->num_components; ci++)
      for (coefi = 0; coefi < DCTSIZE2; coefi++)
        *last_bitpos_ptr++ = -1;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    cinfo->progressive_mode = cinfo->master->lossless = FALSE;
    for (ci = 0; ci < cinfo->num_components; ci++)
      component_sent[ci] = FALSE;
  }

  for (scanno = 1; scanno <= cinfo->num_scans; scanptr++, scanno++) {
    /* Validate component indexes */
    ncomps = scanptr->comps_in_scan;
    if (ncomps <= 0 || ncomps > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, ncomps, MAX_COMPS_IN_SCAN);
    for (ci = 0; ci < ncomps; ci++) {
      thisi = scanptr->component_index[ci];
      if (thisi < 0 || thisi >= cinfo->num_components)
        ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, scanno);
      /* Components must appear in SOF order within each scan */
      if (ci > 0 && thisi <= scanptr->component_index[ci - 1])
        ERREXIT1(cinfo, JERR_BAD_SCAN_SCRIPT, scanno);
    }
    /* Validate progression parameters */
    Ss = scanptr->Ss;
    Se = scanptr->Se;
    Ah = scanptr->Ah;
    Al = scanptr->Al;
    if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
      int max_Ah_Al = cinfo->data_precision == 12 ? 13 : 10;

      if (Ss < 0 || Ss >= DCTSIZE2 || Se < Ss || Se >= DCTSIZE2 ||
          Ah < 0 || Ah > max_Ah_Al || Al < 0 || Al > max_Ah_Al)
        ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      if (Ss == 0) {
        if (Se != 0)            /* DC and AC together not OK */
          ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      } else {
        if (ncomps != 1)        /* AC scans must be for only one component */
          ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      }
      for (ci = 0; ci < ncomps; ci++) {
        last_bitpos_ptr = &last_bitpos[scanptr->component_index[ci]][0];
        if (Ss != 0 && last_bitpos_ptr[0] < 0) /* AC without prior DC scan */
          ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
        for (coefi = Ss; coefi <= Se; coefi++) {
          if (last_bitpos_ptr[coefi] < 0) {
            /* first scan of this coefficient */
            if (Ah != 0)
              ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
          } else {
            /* not first scan */
            if (Ah != last_bitpos_ptr[coefi] || Al != Ah - 1)
              ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
          }
          last_bitpos_ptr[coefi] = Al;
        }
      }
#endif
    } else {
#ifdef C_LOSSLESS_SUPPORTED
      if (cinfo->master->lossless) {
        if (Ss < 1 || Ss > 7 ||         /* predictor selection value */
            Se != 0 || Ah != 0 ||
            Al < 0 || Al >= cinfo->data_precision) /* point transform */
          ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      } else
#endif
      {
        /* For sequential JPEG, all progression parameters must be these: */
        if (Ss != 0 || Se != DCTSIZE2 - 1 || Ah != 0 || Al != 0)
          ERREXIT1(cinfo, JERR_BAD_PROG_SCRIPT, scanno);
      }
      /* Make sure components are not sent twice */
      for (ci = 0; ci < ncomps; ci++) {
        thisi = scanptr->component_index[ci];
        if (component_sent[thisi])
        component_sent[thisi] = TRUE;
      }
    }
  }

  /* Now verify that everything got sent. */
  if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
    for (ci = 0; ci < cinfo->num_components; ci++) {
      if (last_bitpos[ci][0] < 0)
        ERREXIT(cinfo, JERR_MISSING_DATA);
    }
#endif
  } else {
    for (ci = 0; ci < cinfo->num_components; ci++) {
      if (!component_sent[ci])
        ERREXIT(cinfo, JERR_MISSING_DATA);
    }
  }
}

#endif /* NEED_SCAN_SCRIPT */

LOCAL(void)
select_scan_parameters(j_compress_ptr cinfo)
/* Set up the scan parameters for the current scan */
{
  int ci;

#ifdef NEED_SCAN_SCRIPT
  if (cinfo->scan_info != NULL) {
    /* Prepare for current scan --- the script is already validated */
    my_master_ptr master = (my_master_ptr)cinfo->master;
    const jpeg_scan_info *scanptr = cinfo->scan_info + master->scan_number;

    cinfo->comps_in_scan = scanptr->comps_in_scan;
    for (ci = 0; ci < scanptr->comps_in_scan; ci++) {
      cinfo->cur_comp_info[ci] =
        &cinfo->comp_info[scanptr->component_index[ci]];
    }
    cinfo->Ss = scanptr->Ss;
    cinfo->Se = scanptr->Se;
    cinfo->Ah = scanptr->Ah;
    cinfo->Al = scanptr->Al;
  } else
#endif
  {
    /* Prepare for single sequential-JPEG scan containing all components */
    if (cinfo->num_components > MAX_COMPS_IN_SCAN)
      ERREXIT2(cinfo, JERR_COMPONENT_COUNT, cinfo->num_components,
               MAX_COMPS_IN_SCAN);
    cinfo->comps_in_scan = cinfo->num_components;
    for (ci = 0; ci < cinfo->num_components; ci++) {
      cinfo->cur_comp_info[ci] = &cinfo->comp_info[ci];
    }
    if (!cinfo->master->lossless) {
      cinfo->Ss = 0;
      cinfo->Se = DCTSIZE2 - 1;
      cinfo->Ah = 0;
      cinfo->Al = 0;
    }
  }
}

LOCAL(void)
per_scan_setup(j_compress_ptr cinfo)
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

    /* For noninterleaved scan, always one block per MCU */
    compptr->MCU_width = 1;
    compptr->MCU_height = 1;
    compptr->MCU_blocks = 1;
    compptr->MCU_sample_width = data_unit;
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
      jdiv_round_up((long)cinfo->_jpeg_width,
                    (long)(cinfo->max_h_samp_factor * data_unit));
    cinfo->MCU_rows_in_scan = (JDIMENSION)
      jdiv_round_up((long)cinfo->_jpeg_height,
                    (long)(cinfo->max_v_samp_factor * data_unit));

    cinfo->blocks_in_MCU = 0;

    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
      /* Sampling factors give # of blocks of component in each MCU */
      compptr->MCU_width = compptr->h_samp_factor;
      compptr->MCU_height = compptr->v_samp_factor;
      compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
      compptr->MCU_sample_width = compptr->MCU_width * data_unit;
      /* Figure number of non-dummy blocks in last MCU column & row */
      tmp = (int)(compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int)(compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
      /* Prepare array describing MCU composition */
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > C_MAX_BLOCKS_IN_MCU)
        ERREXIT(cinfo, JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
        cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
  }

  if (cinfo->restart_in_rows > 0) {
    long nominal = (long)cinfo->restart_in_rows * (long)cinfo->MCUs_per_row;
    cinfo->restart_interval = (unsigned int)MIN(nominal, 65535L);
  }
}

METHODDEF(void)
prepare_for_pass(j_compress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  switch (master->pass_type) {
  case main_pass:
    select_scan_parameters(cinfo);
    per_scan_setup(cinfo);
    if (!cinfo->raw_data_in) {
      (*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->downsample->start_pass) (cinfo);
      (*cinfo->prep->start_pass) (cinfo, JBUF_PASS_THRU);
    }
    (*cinfo->fdct->start_pass) (cinfo);
    (*cinfo->entropy->start_pass) (cinfo, cinfo->optimize_coding);
    (*cinfo->coef->start_pass) (cinfo,
                                (master->total_passes > 1 ?
                                 JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
    (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    if (cinfo->optimize_coding) {
      /* No immediate data output; postpone writing frame/scan headers */
      master->pub.call_pass_startup = FALSE;
    } else {
      /* Will write frame/scan headers at first jpeg_write_scanlines call */
      master->pub.call_pass_startup = TRUE;
    }
    break;
#ifdef ENTROPY_OPT_SUPPORTED
  case huff_opt_pass:
    /* Do Huffman optimization for a scan after the first one. */
    select_scan_parameters(cinfo);
    per_scan_setup(cinfo);
    if (cinfo->Ss != 0 || cinfo->Ah == 0 || cinfo->arith_code ||
        cinfo->master->lossless) {
      (*cinfo->entropy->start_pass) (cinfo, TRUE);
      (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
      master->pub.call_pass_startup = FALSE;
      break;
    }
    master->pass_type = output_pass;
    master->pass_number++;
#endif
    FALLTHROUGH                 /*FALLTHROUGH*/
  case output_pass:
    if (!cinfo->optimize_coding) {
      select_scan_parameters(cinfo);
      per_scan_setup(cinfo);
    }
    (*cinfo->entropy->start_pass) (cinfo, FALSE);
    (*cinfo->coef->start_pass) (cinfo, JBUF_CRANK_DEST);
    /* We emit frame/scan headers now */
    if (master->scan_number == 0)
      (*cinfo->marker->write_frame_header) (cinfo);
    (*cinfo->marker->write_scan_header) (cinfo);
    master->pub.call_pass_startup = FALSE;
    break;
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  }

  master->pub.is_last_pass = (master->pass_number == master->total_passes - 1);

  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = master->total_passes;
  }
}

METHODDEF(void)
pass_startup(j_compress_ptr cinfo)
{
  cinfo->master->call_pass_startup = FALSE;

  (*cinfo->marker->write_frame_header) (cinfo);
  (*cinfo->marker->write_scan_header) (cinfo);
}

METHODDEF(void)
finish_pass_master(j_compress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  (*cinfo->entropy->finish_pass) (cinfo);

  /* Update state for next pass */
  switch (master->pass_type) {
  case main_pass:
    master->pass_type = output_pass;
    if (!cinfo->optimize_coding)
      master->scan_number++;
    break;
  case huff_opt_pass:
    /* next pass is always output of current scan */
    master->pass_type = output_pass;
    break;
  case output_pass:
    /* next pass is either optimization or output of next scan */
    if (cinfo->optimize_coding)
      master->pass_type = huff_opt_pass;
    master->scan_number++;
    break;
  }

  master->pass_number++;
}

GLOBAL(void)
jinit_c_master_control(j_compress_ptr cinfo, boolean transcode_only)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  master->pub.prepare_for_pass = prepare_for_pass;
  master->pub.pass_startup = pass_startup;
  master->pub.finish_pass = finish_pass_master;
  master->pub.is_last_pass = FALSE;

  if (cinfo->scan_info != NULL) {
#ifdef NEED_SCAN_SCRIPT
    validate_script(cinfo);
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    cinfo->progressive_mode = FALSE;
    cinfo->num_scans = 1;
  }

  if (cinfo->master->lossless) {
    int ci;
    jpeg_component_info *compptr;

    cinfo->raw_data_in = FALSE;
    cinfo->smoothing_factor = 0;
    jpeg_default_colorspace(cinfo);
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
         ci++, compptr++)
      compptr->h_samp_factor = compptr->v_samp_factor = 1;
  }

  /* Validate parameters, determine derived values */
  initial_setup(cinfo, transcode_only);

  if (cinfo->master->lossless ||
      (cinfo->progressive_mode && !cinfo->arith_code))
    cinfo->optimize_coding = TRUE;
  if (cinfo->data_precision == 12 && !cinfo->arith_code)
    cinfo->optimize_coding = TRUE;

  /* Initialize my private state */
  if (transcode_only) {
    /* no main pass in transcoding */
    if (cinfo->optimize_coding)
      master->pass_type = huff_opt_pass;
    else
      master->pass_type = output_pass;
  } else {
    /* for normal compression, first pass is always this type: */
    master->pass_type = main_pass;
  }
  master->scan_number = 0;
  master->pass_number = 0;
  if (cinfo->optimize_coding)
    master->total_passes = cinfo->num_scans * 2;
  else
    master->total_passes = cinfo->num_scans;

  master->jpeg_version = PACKAGE_NAME " version " VERSION " (build " BUILD ")";
}
