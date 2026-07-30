/*
 * jclhuff.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains Huffman entropy encoding routines for lossless JPEG.
 *
 * Much of the complexity here has to do with supporting output suspension.
 * If the data destination module demands suspension, we want to be able to
 * back up to the start of the current MCU.  To do this, we copy state
 * variables into local working storage, and update them back to the
 * permanent JPEG objects only upon successful completion of an MCU.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jlossls.h"
#include "jchuff.h"

#ifdef C_LOSSLESS_SUPPORTED

#define MAX_DIFF_BITS  16

typedef struct {
  size_t put_buffer;            /* current bit-accumulation buffer */
  int put_bits;                 /* # of bits now in it */
} savable_state;

typedef struct {
  int ci, yoffset, MCU_width;
} lhe_input_ptr_info;

typedef struct {
  struct jpeg_entropy_encoder pub; /* public fields */

  savable_state saved;          /* Bit buffer at start of MCU */

  /* These fields are NOT loaded into local working state. */
  unsigned int restarts_to_go;  /* MCUs left in this restart interval */
  int next_restart_num;         /* next restart number to write (0-7) */

  /* Pointers to derived tables (these workspaces have image lifespan) */
  c_derived_tbl *derived_tbls[NUM_HUFF_TBLS];

  /* Pointers to derived tables to be used for each data unit within an MCU */
  c_derived_tbl *cur_tbls[C_MAX_BLOCKS_IN_MCU];

#ifdef ENTROPY_OPT_SUPPORTED    /* Statistics tables for optimization */
  long *count_ptrs[NUM_HUFF_TBLS];

  /* Pointers to stats tables to be used for each data unit within an MCU */
  long *cur_counts[C_MAX_BLOCKS_IN_MCU];
#endif

  /* Pointers to the proper input difference row for each group of data units
   * within an MCU.  For each component, there are Vi groups of Hi data units.
   */
  JDIFFROW input_ptr[C_MAX_BLOCKS_IN_MCU];

  /* Number of input pointers in use for the current MCU.  This is the sum
   * of all Vi in the MCU.
   */
  int num_input_ptrs;

  /* Information used for positioning the input pointers within the input
   * difference rows.
   */
  lhe_input_ptr_info input_ptr_info[C_MAX_BLOCKS_IN_MCU];

  /* Index of the proper input pointer for each data unit within an MCU */
  int input_ptr_index[C_MAX_BLOCKS_IN_MCU];

} lhuff_entropy_encoder;

typedef lhuff_entropy_encoder *lhuff_entropy_ptr;

typedef struct {
  JOCTET *next_output_byte;     /* => next byte to write in buffer */
  size_t free_in_buffer;        /* # of byte spaces remaining in buffer */
  savable_state cur;            /* Current bit buffer & DC state */
  j_compress_ptr cinfo;         /* dump_buffer needs access to this */
} working_state;

/* Forward declarations */
METHODDEF(JDIMENSION) encode_mcus_huff(j_compress_ptr cinfo,
                                       JDIFFIMAGE diff_buf,
                                       JDIMENSION MCU_row_num,
                                       JDIMENSION MCU_col_num,
                                       JDIMENSION nMCU);
METHODDEF(void) finish_pass_huff(j_compress_ptr cinfo);
#ifdef ENTROPY_OPT_SUPPORTED
METHODDEF(JDIMENSION) encode_mcus_gather(j_compress_ptr cinfo,
                                         JDIFFIMAGE diff_buf,
                                         JDIMENSION MCU_row_num,
                                         JDIMENSION MCU_col_num,
                                         JDIMENSION nMCU);
METHODDEF(void) finish_pass_gather(j_compress_ptr cinfo);
#endif

METHODDEF(void)
start_pass_lhuff(j_compress_ptr cinfo, boolean gather_statistics)
{
  lhuff_entropy_ptr entropy = (lhuff_entropy_ptr)cinfo->entropy;
  int ci, dctbl, sampn, ptrn, yoffset, xoffset;
  jpeg_component_info *compptr;

  if (gather_statistics) {
#ifdef ENTROPY_OPT_SUPPORTED
    entropy->pub.encode_mcus = encode_mcus_gather;
    entropy->pub.finish_pass = finish_pass_gather;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    entropy->pub.encode_mcus = encode_mcus_huff;
    entropy->pub.finish_pass = finish_pass_huff;
  }

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    if (gather_statistics) {
#ifdef ENTROPY_OPT_SUPPORTED
      if (dctbl < 0 || dctbl >= NUM_HUFF_TBLS)
        ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, dctbl);
      if (entropy->count_ptrs[dctbl] == NULL)
        entropy->count_ptrs[dctbl] = (long *)
          (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                      257 * sizeof(long));
      memset(entropy->count_ptrs[dctbl], 0, 257 * sizeof(long));
#endif
    } else {
      jpeg_make_c_derived_tbl(cinfo, TRUE, dctbl,
                              &entropy->derived_tbls[dctbl]);
    }
  }

  /* Precalculate encoding info for each sample in an MCU of this scan */
  for (sampn = 0, ptrn = 0; sampn < cinfo->blocks_in_MCU;) {
    compptr = cinfo->cur_comp_info[cinfo->MCU_membership[sampn]];
    ci = compptr->component_index;
    for (yoffset = 0; yoffset < compptr->MCU_height; yoffset++, ptrn++) {
      /* Precalculate the setup info for each input pointer */
      entropy->input_ptr_info[ptrn].ci = ci;
      entropy->input_ptr_info[ptrn].yoffset = yoffset;
      entropy->input_ptr_info[ptrn].MCU_width = compptr->MCU_width;
      for (xoffset = 0; xoffset < compptr->MCU_width; xoffset++, sampn++) {
        /* Precalculate the input pointer index for each sample */
        entropy->input_ptr_index[sampn] = ptrn;
        /* Precalculate which tables to use for each sample */
        entropy->cur_tbls[sampn] = entropy->derived_tbls[compptr->dc_tbl_no];
        entropy->cur_counts[sampn] = entropy->count_ptrs[compptr->dc_tbl_no];
      }
    }
  }
  entropy->num_input_ptrs = ptrn;

  /* Initialize bit buffer to empty */
  entropy->saved.put_buffer = 0;
  entropy->saved.put_bits = 0;

  /* Initialize restart stuff */
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}

/* Emit a byte, taking 'action' if must suspend. */
#define emit_byte(state, val, action) { \
  *(state)->next_output_byte++ = (JOCTET)(val); \
  if (--(state)->free_in_buffer == 0) \
    if (!dump_buffer(state)) \
      { action; } \
}

LOCAL(boolean)
dump_buffer(working_state *state)
/* Empty the output buffer; return TRUE if successful, FALSE if must suspend */
{
  struct jpeg_destination_mgr *dest = state->cinfo->dest;

  if (!(*dest->empty_output_buffer) (state->cinfo))
    return FALSE;
  /* After a successful buffer dump, must reset buffer pointers */
  state->next_output_byte = dest->next_output_byte;
  state->free_in_buffer = dest->free_in_buffer;
  return TRUE;
}

INLINE
LOCAL(boolean)
emit_bits(working_state *state, unsigned int code, int size)
/* Emit some bits; return TRUE if successful, FALSE if must suspend */
{
  /* This routine is heavily used, so it's worth coding tightly. */
  register size_t put_buffer = (size_t)code;
  register int put_bits = state->cur.put_bits;

  /* if size is 0, caller used an invalid Huffman table entry */
  if (size == 0)
    ERREXIT(state->cinfo, JERR_HUFF_MISSING_CODE);

  put_buffer &= (((size_t)1) << size) - 1; /* mask off any extra bits in code */

  put_bits += size;             /* new number of bits in buffer */

  put_buffer <<= 24 - put_bits; /* align incoming bits */

  put_buffer |= state->cur.put_buffer; /* and merge with old buffer contents */

  while (put_bits >= 8) {
    int c = (int)((put_buffer >> 16) & 0xFF);

    emit_byte(state, c, return FALSE);
    if (c == 0xFF) {            /* need to stuff a zero byte? */
      emit_byte(state, 0, return FALSE);
    }
    put_buffer <<= 8;
    put_bits -= 8;
  }

  state->cur.put_buffer = put_buffer; /* update state variables */
  state->cur.put_bits = put_bits;

  return TRUE;
}

LOCAL(boolean)
flush_bits(working_state *state)
{
  if (!emit_bits(state, 0x7F, 7)) /* fill any partial byte with ones */
    return FALSE;
  state->cur.put_buffer = 0;    /* and reset bit-buffer to empty */
  state->cur.put_bits = 0;
  return TRUE;
}

LOCAL(boolean)
emit_restart(working_state *state, int restart_num)
{
  if (!flush_bits(state))
    return FALSE;

  emit_byte(state, 0xFF, return FALSE);
  emit_byte(state, JPEG_RST0 + restart_num, return FALSE);

  return TRUE;
}

METHODDEF(JDIMENSION)
encode_mcus_huff(j_compress_ptr cinfo, JDIFFIMAGE diff_buf,
                 JDIMENSION MCU_row_num, JDIMENSION MCU_col_num,
                 JDIMENSION nMCU)
{
  lhuff_entropy_ptr entropy = (lhuff_entropy_ptr)cinfo->entropy;
  working_state state;
  int sampn, ci, yoffset, MCU_width, ptrn;
  JDIMENSION mcu_num;

  /* Load up working state */
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  state.cur = entropy->saved;
  state.cinfo = cinfo;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (!emit_restart(&state, entropy->next_restart_num))
        return 0;
  }

  /* Set input pointer locations based on MCU_col_num */
  for (ptrn = 0; ptrn < entropy->num_input_ptrs; ptrn++) {
    ci = entropy->input_ptr_info[ptrn].ci;
    yoffset = entropy->input_ptr_info[ptrn].yoffset;
    MCU_width = entropy->input_ptr_info[ptrn].MCU_width;
    entropy->input_ptr[ptrn] =
      diff_buf[ci][MCU_row_num + yoffset] + (MCU_col_num * MCU_width);
  }

  for (mcu_num = 0; mcu_num < nMCU; mcu_num++) {

    /* Inner loop handles the samples in the MCU */
    for (sampn = 0; sampn < cinfo->blocks_in_MCU; sampn++) {
      register int temp, temp2;
      register int nbits;
      c_derived_tbl *dctbl = entropy->cur_tbls[sampn];

      /* Input the sample difference */
      temp = *entropy->input_ptr[entropy->input_ptr_index[sampn]]++;

      if (temp & 0x8000) {      /* instead of temp < 0 */
        temp = (-temp) & 0x7FFF; /* absolute value, mod 2^16 */
        if (temp == 0)          /* special case: magnitude = 32768 */
          temp2 = temp = 0x8000;
        temp2 = ~temp;          /* one's complement of magnitude */
      } else {
        temp &= 0x7FFF;         /* abs value mod 2^16 */
        temp2 = temp;           /* magnitude */
      }

      /* Find the number of bits needed for the magnitude of the difference */
      nbits = 0;
      while (temp) {
        nbits++;
        temp >>= 1;
      }
      if (nbits > MAX_DIFF_BITS)
        ERREXIT(cinfo, JERR_BAD_DCT_COEF);

      if (!emit_bits(&state, dctbl->ehufco[nbits], dctbl->ehufsi[nbits]))
        return mcu_num;

      if (nbits &&              /* emit_bits rejects calls with size 0 */
          nbits != 16)          /* special case: no bits should be emitted */
        if (!emit_bits(&state, (unsigned int)temp2, nbits))
          return mcu_num;
    }

    /* Completed MCU, so update state */
    cinfo->dest->next_output_byte = state.next_output_byte;
    cinfo->dest->free_in_buffer = state.free_in_buffer;
    entropy->saved = state.cur;

    /* Update restart-interval state too */
    if (cinfo->restart_interval) {
      if (entropy->restarts_to_go == 0) {
        entropy->restarts_to_go = cinfo->restart_interval;
        entropy->next_restart_num++;
        entropy->next_restart_num &= 7;
      }
      entropy->restarts_to_go--;
    }
  }

  return nMCU;
}

METHODDEF(void)
finish_pass_huff(j_compress_ptr cinfo)
{
  lhuff_entropy_ptr entropy = (lhuff_entropy_ptr)cinfo->entropy;
  working_state state;

  /* Load up working state ... flush_bits needs it */
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  state.cur = entropy->saved;
  state.cinfo = cinfo;

  /* Flush out the last data */
  if (!flush_bits(&state))
    ERREXIT(cinfo, JERR_CANT_SUSPEND);

  /* Update state */
  cinfo->dest->next_output_byte = state.next_output_byte;
  cinfo->dest->free_in_buffer = state.free_in_buffer;
  entropy->saved = state.cur;
}

#ifdef ENTROPY_OPT_SUPPORTED

METHODDEF(JDIMENSION)
encode_mcus_gather(j_compress_ptr cinfo, JDIFFIMAGE diff_buf,
                   JDIMENSION MCU_row_num, JDIMENSION MCU_col_num,
                   JDIMENSION nMCU)
{
  lhuff_entropy_ptr entropy = (lhuff_entropy_ptr)cinfo->entropy;
  int sampn, ci, yoffset, MCU_width, ptrn;
  JDIMENSION mcu_num;

  /* Take care of restart intervals if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      entropy->restarts_to_go = cinfo->restart_interval;
    }
    entropy->restarts_to_go--;
  }

  /* Set input pointer locations based on MCU_col_num */
  for (ptrn = 0; ptrn < entropy->num_input_ptrs; ptrn++) {
    ci = entropy->input_ptr_info[ptrn].ci;
    yoffset = entropy->input_ptr_info[ptrn].yoffset;
    MCU_width = entropy->input_ptr_info[ptrn].MCU_width;
    entropy->input_ptr[ptrn] =
      diff_buf[ci][MCU_row_num + yoffset] + (MCU_col_num * MCU_width);
  }

  for (mcu_num = 0; mcu_num < nMCU; mcu_num++) {

    /* Inner loop handles the samples in the MCU */
    for (sampn = 0; sampn < cinfo->blocks_in_MCU; sampn++) {
      register int temp;
      register int nbits;
      long *counts = entropy->cur_counts[sampn];

      temp = *entropy->input_ptr[entropy->input_ptr_index[sampn]]++;

      if (temp & 0x8000) {      /* instead of temp < 0 */
        temp = (-temp) & 0x7FFF; /* absolute value, mod 2^16 */
        if (temp == 0)          /* special case: magnitude = 32768 */
          temp = 0x8000;
      } else
        temp &= 0x7FFF;         /* abs value mod 2^16 */

      /* Find the number of bits needed for the magnitude of the difference */
      nbits = 0;
      while (temp) {
        nbits++;
        temp >>= 1;
      }
      if (nbits > MAX_DIFF_BITS)
        ERREXIT(cinfo, JERR_BAD_DCT_COEF);

      /* Count the Huffman symbol for the number of bits */
      counts[nbits]++;
    }
  }

  return nMCU;
}

METHODDEF(void)
finish_pass_gather(j_compress_ptr cinfo)
{
  lhuff_entropy_ptr entropy = (lhuff_entropy_ptr)cinfo->entropy;
  int ci, dctbl;
  jpeg_component_info *compptr;
  JHUFF_TBL **htblptr;
  boolean did_dc[NUM_HUFF_TBLS];

  memset(did_dc, 0, sizeof(did_dc));

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    if (!did_dc[dctbl]) {
      htblptr = &cinfo->dc_huff_tbl_ptrs[dctbl];
      if (*htblptr == NULL)
        *htblptr = jpeg_alloc_huff_table((j_common_ptr)cinfo);
      jpeg_gen_optimal_table(cinfo, *htblptr, entropy->count_ptrs[dctbl]);
      did_dc[dctbl] = TRUE;
    }
  }
}

#endif /* ENTROPY_OPT_SUPPORTED */

GLOBAL(void)
jinit_lhuff_encoder(j_compress_ptr cinfo)
{
  lhuff_entropy_ptr entropy;
  int i;

  entropy = (lhuff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(lhuff_entropy_encoder));
  cinfo->entropy = (struct jpeg_entropy_encoder *)entropy;
  entropy->pub.start_pass = start_pass_lhuff;

  /* Mark tables unallocated */
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->derived_tbls[i] = NULL;
#ifdef ENTROPY_OPT_SUPPORTED
    entropy->count_ptrs[i] = NULL;
#endif
  }
}

#endif /* C_LOSSLESS_SUPPORTED */
