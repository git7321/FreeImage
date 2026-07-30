/*
 * jdtrans.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1995-1997, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2020, 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains library routines for transcoding decompression,
 * that is, reading raw DCT coefficient arrays from an input JPEG file.
 * The routines in jdapimin.c will also be needed by a transcoder.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jpegapicomp.h"

LOCAL(void) transdecode_master_selection(j_decompress_ptr cinfo);

GLOBAL(jvirt_barray_ptr *)
jpeg_read_coefficients(j_decompress_ptr cinfo)
{
  if (cinfo->master->lossless)
    ERREXIT(cinfo, JERR_NOTIMPL);

  if (cinfo->global_state == DSTATE_READY) {
    /* First call: initialize active modules */
    transdecode_master_selection(cinfo);
    cinfo->global_state = DSTATE_RDCOEFS;
  }
  if (cinfo->global_state == DSTATE_RDCOEFS) {
    /* Absorb whole file into the coef buffer */
    for (;;) {
      int retcode;
      /* Call progress monitor hook if present */
      if (cinfo->progress != NULL)
        (*cinfo->progress->progress_monitor) ((j_common_ptr)cinfo);
      /* Absorb some more input */
      retcode = (*cinfo->inputctl->consume_input) (cinfo);
      if (retcode == JPEG_SUSPENDED)
        return NULL;
      if (retcode == JPEG_REACHED_EOI)
        break;
      /* Advance progress counter if appropriate */
      if (cinfo->progress != NULL &&
          (retcode == JPEG_ROW_COMPLETED || retcode == JPEG_REACHED_SOS)) {
        if (++cinfo->progress->pass_counter >= cinfo->progress->pass_limit) {
          /* startup underestimated number of scans; ratchet up one scan */
          cinfo->progress->pass_limit += (long)cinfo->total_iMCU_rows;
        }
      }
    }
    /* Set state so that jpeg_finish_decompress does the right thing */
    cinfo->global_state = DSTATE_STOPPING;
  }
  if ((cinfo->global_state == DSTATE_STOPPING ||
       cinfo->global_state == DSTATE_BUFIMAGE) && cinfo->buffered_image) {
    return cinfo->coef->coef_arrays;
  }
  return NULL;                  /* keep compiler happy */
}

LOCAL(void)
transdecode_master_selection(j_decompress_ptr cinfo)
{
  /* This is effectively a buffered-image operation. */
  cinfo->buffered_image = TRUE;

#if JPEG_LIB_VERSION >= 80
  /* Compute output image dimensions and related values. */
  jpeg_core_output_dimensions(cinfo);
#endif

  /* Entropy decoding: either Huffman or arithmetic coding. */
  if (cinfo->arith_code) {
#ifdef D_ARITH_CODING_SUPPORTED
    jinit_arith_decoder(cinfo);
#else
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
#endif
  } else {
    if (cinfo->progressive_mode) {
#ifdef D_PROGRESSIVE_SUPPORTED
      jinit_phuff_decoder(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else
      jinit_huff_decoder(cinfo);
  }

  /* Always get a full-image coefficient buffer. */
  if (cinfo->data_precision == 12)
    j12init_d_coef_controller(cinfo, TRUE);
  else
    jinit_d_coef_controller(cinfo, TRUE);

  /* We can now tell the memory manager to allocate virtual arrays. */
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr)cinfo);

  /* Initialize input side of decompressor to consume first scan. */
  (*cinfo->inputctl->start_input_pass) (cinfo);

  /* Initialize progress monitoring. */
  if (cinfo->progress != NULL) {
    int nscans;
    /* Estimate number of scans to set pass_limit. */
    if (cinfo->progressive_mode) {
      /* Arbitrarily estimate 2 interleaved DC scans + 3 AC scans/component. */
      nscans = 2 + 3 * cinfo->num_components;
    } else if (cinfo->inputctl->has_multiple_scans) {
      /* For a nonprogressive multiscan file, estimate 1 scan per component. */
      nscans = cinfo->num_components;
    } else {
      nscans = 1;
    }
    cinfo->progress->pass_counter = 0L;
    cinfo->progress->pass_limit = (long)cinfo->total_iMCU_rows * nscans;
    cinfo->progress->completed_passes = 0;
    cinfo->progress->total_passes = 1;
  }
}
