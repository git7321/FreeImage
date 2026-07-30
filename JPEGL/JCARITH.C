/*
 * jcarith.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Developed 1997-2009 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2015, 2018, 2021-2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains portable arithmetic entropy encoding routines for JPEG
 * (implementing Recommendation ITU-T T.81 | ISO/IEC 10918-1).
 *
 * Both sequential and progressive modes are supported in this single module.
 *
 * Suspension is not currently supported in this module.
 *
 * NOTE: All referenced figures are from
 * Recommendation ITU-T T.81 (1992) | ISO/IEC 10918-1:1994.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

typedef struct {
  struct jpeg_entropy_encoder pub; /* public fields */

  JLONG c; /* C register, base of coding interval, layout as in sec. D.1.3 */
  JLONG a;               /* A register, normalized size of coding interval */
  JLONG sc;        /* counter for stacked 0xFF values which might overflow */
  JLONG zc;          /* counter for pending 0x00 output values which might *
                          * be discarded at the end ("Pacman" termination) */
  int ct;  /* bit shift counter, determines when next byte will be written */
  int buffer;                /* buffer for most recent output byte != 0xFF */

  int last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each component */
  int dc_context[MAX_COMPS_IN_SCAN]; /* context index for DC conditioning */

  unsigned int restarts_to_go;  /* MCUs left in this restart interval */
  int next_restart_num;         /* next restart number to write (0-7) */

  /* Pointers to statistics areas (these workspaces have image lifespan) */
  unsigned char *dc_stats[NUM_ARITH_TBLS];
  unsigned char *ac_stats[NUM_ARITH_TBLS];

  /* Statistics bin for coding with fixed probability 0.5 */
  unsigned char fixed_bin[4];
} arith_entropy_encoder;

typedef arith_entropy_encoder *arith_entropy_ptr;

#define DC_STAT_BINS  64
#define AC_STAT_BINS  256

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS    int ishift_temp;
#define IRIGHT_SHIFT(x, shft) \
  ((ishift_temp = (x)) < 0 ? \
   (ishift_temp >> (shft)) | ((~0) << (16 - (shft))) : \
   (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x, shft)   ((x) >> (shft))
#endif

LOCAL(void)
emit_byte(int val, j_compress_ptr cinfo)
{
  struct jpeg_destination_mgr *dest = cinfo->dest;

  *dest->next_output_byte++ = (JOCTET)val;
  if (--dest->free_in_buffer == 0)
    if (!(*dest->empty_output_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
}

METHODDEF(void)
finish_pass(j_compress_ptr cinfo)
{
  arith_entropy_ptr e = (arith_entropy_ptr)cinfo->entropy;
  JLONG temp;

  if ((temp = (e->a - 1 + e->c) & 0xFFFF0000UL) < e->c)
    e->c = temp + 0x8000L;
  else
    e->c = temp;
  /* Send remaining bytes to output */
  e->c <<= e->ct;
  if (e->c & 0xF8000000UL) {
    /* One final overflow has to be handled */
    if (e->buffer >= 0) {
      if (e->zc)
        do emit_byte(0x00, cinfo);
        while (--e->zc);
      emit_byte(e->buffer + 1, cinfo);
      if (e->buffer + 1 == 0xFF)
        emit_byte(0x00, cinfo);
    }
    e->zc += e->sc;
    e->sc = 0;
  } else {
    if (e->buffer == 0)
      ++e->zc;
    else if (e->buffer >= 0) {
      if (e->zc)
        do emit_byte(0x00, cinfo);
        while (--e->zc);
      emit_byte(e->buffer, cinfo);
    }
    if (e->sc) {
      if (e->zc)
        do emit_byte(0x00, cinfo);
        while (--e->zc);
      do {
        emit_byte(0xFF, cinfo);
        emit_byte(0x00, cinfo);
      } while (--e->sc);
    }
  }
  /* Output final bytes only if they are not 0x00 */
  if (e->c & 0x7FFF800L) {
    if (e->zc)
      do emit_byte(0x00, cinfo);
      while (--e->zc);
    emit_byte((e->c >> 19) & 0xFF, cinfo);
    if (((e->c >> 19) & 0xFF) == 0xFF)
      emit_byte(0x00, cinfo);
    if (e->c & 0x7F800L) {
      emit_byte((e->c >> 11) & 0xFF, cinfo);
      if (((e->c >> 11) & 0xFF) == 0xFF)
        emit_byte(0x00, cinfo);
    }
  }
}

LOCAL(void)
arith_encode(j_compress_ptr cinfo, unsigned char *st, int val)
{
  register arith_entropy_ptr e = (arith_entropy_ptr)cinfo->entropy;
  register unsigned char nl, nm;
  register JLONG qe, temp;
  register int sv;

  sv = *st;
  qe = jpeg_aritab[sv & 0x7F];
  nl = (unsigned char)(qe & 0xFF);  qe >>= 8;
  nm = (unsigned char)(qe & 0xFF);  qe >>= 8;

  /* Encode & estimation procedures per sections D.1.4 & D.1.5 */
  e->a -= qe;
  if (val != (sv >> 7)) {
    /* Encode the less probable symbol */
    if (e->a >= qe) {
      e->c += e->a;
      e->a = qe;
    }
    *st = (unsigned char)((sv & 0x80) ^ nl);
  } else {
    /* Encode the more probable symbol */
    if (e->a >= 0x8000L)
      return;
    if (e->a < qe) {
      e->c += e->a;
      e->a = qe;
    }
    *st = (unsigned char)((sv & 0x80) ^ nm);
  }

  /* Renormalization & data output per section D.1.6 */
  do {
    e->a <<= 1;
    e->c <<= 1;
    if (--e->ct == 0) {
      /* Another byte is ready for output */
      temp = e->c >> 19;
      if (temp > 0xFF) {
        /* Handle overflow over all stacked 0xFF bytes */
        if (e->buffer >= 0) {
          if (e->zc)
            do emit_byte(0x00, cinfo);
            while (--e->zc);
          emit_byte(e->buffer + 1, cinfo);
          if (e->buffer + 1 == 0xFF)
            emit_byte(0x00, cinfo);
        }
        e->zc += e->sc;
        e->sc = 0;
        e->buffer = temp & 0xFF;
      } else if (temp == 0xFF) {
        ++e->sc;
      } else {
        if (e->buffer == 0)
          ++e->zc;
        else if (e->buffer >= 0) {
          if (e->zc)
            do emit_byte(0x00, cinfo);
            while (--e->zc);
          emit_byte(e->buffer, cinfo);
        }
        if (e->sc) {
          if (e->zc)
            do emit_byte(0x00, cinfo);
            while (--e->zc);
          do {
            emit_byte(0xFF, cinfo);
            emit_byte(0x00, cinfo);
          } while (--e->sc);
        }
        e->buffer = temp & 0xFF;
      }
      e->c &= 0x7FFFFL;
      e->ct += 8;
    }
  } while (e->a < 0x8000L);
}

LOCAL(void)
emit_restart(j_compress_ptr cinfo, int restart_num)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  int ci;
  jpeg_component_info *compptr;

  finish_pass(cinfo);

  emit_byte(0xFF, cinfo);
  emit_byte(JPEG_RST0 + restart_num, cinfo);

  /* Re-initialize statistics areas */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* DC needs no table for refinement scan */
    if (cinfo->progressive_mode == 0 || (cinfo->Ss == 0 && cinfo->Ah == 0)) {
      memset(entropy->dc_stats[compptr->dc_tbl_no], 0, DC_STAT_BINS);
      /* Reset DC predictions to 0 */
      entropy->last_dc_val[ci] = 0;
      entropy->dc_context[ci] = 0;
    }
    /* AC needs no table when not present */
    if (cinfo->progressive_mode == 0 || cinfo->Se) {
      memset(entropy->ac_stats[compptr->ac_tbl_no], 0, AC_STAT_BINS);
    }
  }

  /* Reset arithmetic encoding variables */
  entropy->c = 0;
  entropy->a = 0x10000L;
  entropy->sc = 0;
  entropy->zc = 0;
  entropy->ct = 11;
  entropy->buffer = -1;
}

METHODDEF(boolean)
encode_mcu_DC_first(j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int blkn, ci, tbl;
  int v, v2, m;
  ISHIFT_TEMPS

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    tbl = cinfo->cur_comp_info[ci]->dc_tbl_no;

    m = IRIGHT_SHIFT((int)((*block)[0]), cinfo->Al);

    st = entropy->dc_stats[tbl] + entropy->dc_context[ci];

    /* Figure F.4: Encode_DC_DIFF */
    if ((v = m - entropy->last_dc_val[ci]) == 0) {
      arith_encode(cinfo, st, 0);
      entropy->dc_context[ci] = 0;
    } else {
      entropy->last_dc_val[ci] = m;
      arith_encode(cinfo, st, 1);
      if (v > 0) {
        arith_encode(cinfo, st + 1, 0);
        st += 2;
        entropy->dc_context[ci] = 4;
      } else {
        v = -v;
        arith_encode(cinfo, st + 1, 1);
        st += 3;
        entropy->dc_context[ci] = 8;
      }
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
        arith_encode(cinfo, st, 1);
        m = 1;
        v2 = v;
        st = entropy->dc_stats[tbl] + 20;
        while (v2 >>= 1) {
          arith_encode(cinfo, st, 1);
          m <<= 1;
          st += 1;
        }
      }
      arith_encode(cinfo, st, 0);
      /* Section F.1.4.4.1.2: Establish dc_context conditioning category */
      if (m < (int)((1L << cinfo->arith_dc_L[tbl]) >> 1))
        entropy->dc_context[ci] = 0;
      else if (m > (int)((1L << cinfo->arith_dc_U[tbl]) >> 1))
        entropy->dc_context[ci] += 8;
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
        arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }
  }

  return TRUE;
}

METHODDEF(boolean)
encode_mcu_AC_first(j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int tbl, k, ke;
  int v, v2, m;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  /* Encode the MCU data block */
  block = MCU_data[0];
  tbl = cinfo->cur_comp_info[0]->ac_tbl_no;

  for (ke = cinfo->Se; ke > 0; ke--)
    if ((v = (*block)[jpeg_natural_order[ke]]) >= 0) {
      if (v >>= cinfo->Al) break;
    } else {
      v = -v;
      if (v >>= cinfo->Al) break;
    }

  /* Figure F.5: Encode_AC_Coefficients */
  for (k = cinfo->Ss; k <= ke; k++) {
    st = entropy->ac_stats[tbl] + 3 * (k - 1);
    arith_encode(cinfo, st, 0);
    for (;;) {
      if ((v = (*block)[jpeg_natural_order[k]]) >= 0) {
        if (v >>= cinfo->Al) {
          arith_encode(cinfo, st + 1, 1);
          arith_encode(cinfo, entropy->fixed_bin, 0);
          break;
        }
      } else {
        v = -v;
        if (v >>= cinfo->Al) {
          arith_encode(cinfo, st + 1, 1);
          arith_encode(cinfo, entropy->fixed_bin, 1);
          break;
        }
      }
      arith_encode(cinfo, st + 1, 0);  st += 3;  k++;
    }
    st += 2;
    /* Figure F.8: Encoding the magnitude category of v */
    m = 0;
    if (v -= 1) {
      arith_encode(cinfo, st, 1);
      m = 1;
      v2 = v;
      if (v2 >>= 1) {
        arith_encode(cinfo, st, 1);
        m <<= 1;
        st = entropy->ac_stats[tbl] +
             (k <= cinfo->arith_ac_K[tbl] ? 189 : 217);
        while (v2 >>= 1) {
          arith_encode(cinfo, st, 1);
          m <<= 1;
          st += 1;
        }
      }
    }
    arith_encode(cinfo, st, 0);
    /* Figure F.9: Encoding the magnitude bit pattern of v */
    st += 14;
    while (m >>= 1)
      arith_encode(cinfo, st, (m & v) ? 1 : 0);
  }
  /* Encode EOB decision only if k <= cinfo->Se */
  if (k <= cinfo->Se) {
    st = entropy->ac_stats[tbl] + 3 * (k - 1);
    arith_encode(cinfo, st, 1);
  }

  return TRUE;
}

METHODDEF(boolean)
encode_mcu_DC_refine(j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  unsigned char *st;
  int Al, blkn;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  st = entropy->fixed_bin;
  Al = cinfo->Al;

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    arith_encode(cinfo, st, (MCU_data[blkn][0][0] >> Al) & 1);
  }

  return TRUE;
}

METHODDEF(boolean)
encode_mcu_AC_refine(j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  JBLOCKROW block;
  unsigned char *st;
  int tbl, k, ke, kex;
  int v;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  /* Encode the MCU data block */
  block = MCU_data[0];
  tbl = cinfo->cur_comp_info[0]->ac_tbl_no;

  for (ke = cinfo->Se; ke > 0; ke--)
    if ((v = (*block)[jpeg_natural_order[ke]]) >= 0) {
      if (v >>= cinfo->Al) break;
    } else {
      v = -v;
      if (v >>= cinfo->Al) break;
    }

  /* Establish EOBx (previous stage end-of-block) index */
  for (kex = ke; kex > 0; kex--)
    if ((v = (*block)[jpeg_natural_order[kex]]) >= 0) {
      if (v >>= cinfo->Ah) break;
    } else {
      v = -v;
      if (v >>= cinfo->Ah) break;
    }

  /* Figure G.10: Encode_AC_Coefficients_SA */
  for (k = cinfo->Ss; k <= ke; k++) {
    st = entropy->ac_stats[tbl] + 3 * (k - 1);
    if (k > kex)
      arith_encode(cinfo, st, 0);
    for (;;) {
      if ((v = (*block)[jpeg_natural_order[k]]) >= 0) {
        if (v >>= cinfo->Al) {
          if (v >> 1)
            arith_encode(cinfo, st + 2, (v & 1));
          else {
            arith_encode(cinfo, st + 1, 1);
            arith_encode(cinfo, entropy->fixed_bin, 0);
          }
          break;
        }
      } else {
        v = -v;
        if (v >>= cinfo->Al) {
          if (v >> 1)
            arith_encode(cinfo, st + 2, (v & 1));
          else {
            arith_encode(cinfo, st + 1, 1);
            arith_encode(cinfo, entropy->fixed_bin, 1);
          }
          break;
        }
      }
      arith_encode(cinfo, st + 1, 0);  st += 3;  k++;
    }
  }
  /* Encode EOB decision only if k <= cinfo->Se */
  if (k <= cinfo->Se) {
    st = entropy->ac_stats[tbl] + 3 * (k - 1);
    arith_encode(cinfo, st, 1);
  }

  return TRUE;
}

METHODDEF(boolean)
encode_mcu(j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  jpeg_component_info *compptr;
  JBLOCKROW block;
  unsigned char *st;
  int blkn, ci, tbl, k, ke;
  int v, v2, m;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      emit_restart(cinfo, entropy->next_restart_num);
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];

    tbl = compptr->dc_tbl_no;

    /* Table F.4: Point to statistics bin S0 for DC coefficient coding */
    st = entropy->dc_stats[tbl] + entropy->dc_context[ci];

    /* Figure F.4: Encode_DC_DIFF */
    if ((v = (*block)[0] - entropy->last_dc_val[ci]) == 0) {
      arith_encode(cinfo, st, 0);
      entropy->dc_context[ci] = 0;
    } else {
      entropy->last_dc_val[ci] = (*block)[0];
      arith_encode(cinfo, st, 1);
      if (v > 0) {
        arith_encode(cinfo, st + 1, 0); /* Table F.4: SS = S0 + 1 */
        st += 2;                        /* Table F.4: SP = S0 + 2 */
        entropy->dc_context[ci] = 4;    /* small positive diff category */
      } else {
        v = -v;
        arith_encode(cinfo, st + 1, 1); /* Table F.4: SS = S0 + 1 */
        st += 3;                        /* Table F.4: SN = S0 + 3 */
        entropy->dc_context[ci] = 8;    /* small negative diff category */
      }
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
        arith_encode(cinfo, st, 1);
        m = 1;
        v2 = v;
        st = entropy->dc_stats[tbl] + 20;
        while (v2 >>= 1) {
          arith_encode(cinfo, st, 1);
          m <<= 1;
          st += 1;
        }
      }
      arith_encode(cinfo, st, 0);
      /* Section F.1.4.4.1.2: Establish dc_context conditioning category */
      if (m < (int)((1L << cinfo->arith_dc_L[tbl]) >> 1))
        entropy->dc_context[ci] = 0;
      else if (m > (int)((1L << cinfo->arith_dc_U[tbl]) >> 1))
        entropy->dc_context[ci] += 8;
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
        arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }

    tbl = compptr->ac_tbl_no;

    /* Establish EOB (end-of-block) index */
    for (ke = DCTSIZE2 - 1; ke > 0; ke--)
      if ((*block)[jpeg_natural_order[ke]]) break;

    /* Figure F.5: Encode_AC_Coefficients */
    for (k = 1; k <= ke; k++) {
      st = entropy->ac_stats[tbl] + 3 * (k - 1);
      arith_encode(cinfo, st, 0);
      while ((v = (*block)[jpeg_natural_order[k]]) == 0) {
        arith_encode(cinfo, st + 1, 0);  st += 3;  k++;
      }
      arith_encode(cinfo, st + 1, 1);
      if (v > 0) {
        arith_encode(cinfo, entropy->fixed_bin, 0);
      } else {
        v = -v;
        arith_encode(cinfo, entropy->fixed_bin, 1);
      }
      st += 2;
      /* Figure F.8: Encoding the magnitude category of v */
      m = 0;
      if (v -= 1) {
        arith_encode(cinfo, st, 1);
        m = 1;
        v2 = v;
        if (v2 >>= 1) {
          arith_encode(cinfo, st, 1);
          m <<= 1;
          st = entropy->ac_stats[tbl] +
               (k <= cinfo->arith_ac_K[tbl] ? 189 : 217);
          while (v2 >>= 1) {
            arith_encode(cinfo, st, 1);
            m <<= 1;
            st += 1;
          }
        }
      }
      arith_encode(cinfo, st, 0);
      /* Figure F.9: Encoding the magnitude bit pattern of v */
      st += 14;
      while (m >>= 1)
        arith_encode(cinfo, st, (m & v) ? 1 : 0);
    }
    /* Encode EOB decision only if k <= DCTSIZE2 - 1 */
    if (k <= DCTSIZE2 - 1) {
      st = entropy->ac_stats[tbl] + 3 * (k - 1);
      arith_encode(cinfo, st, 1);
    }
  }

  return TRUE;
}

METHODDEF(void)
start_pass(j_compress_ptr cinfo, boolean gather_statistics)
{
  arith_entropy_ptr entropy = (arith_entropy_ptr)cinfo->entropy;
  int ci, tbl;
  jpeg_component_info *compptr;

  if (gather_statistics)
    ERREXIT(cinfo, JERR_NOTIMPL);

  if (cinfo->progressive_mode) {
    if (cinfo->Ah == 0) {
      if (cinfo->Ss == 0)
        entropy->pub.encode_mcu = encode_mcu_DC_first;
      else
        entropy->pub.encode_mcu = encode_mcu_AC_first;
    } else {
      if (cinfo->Ss == 0)
        entropy->pub.encode_mcu = encode_mcu_DC_refine;
      else
        entropy->pub.encode_mcu = encode_mcu_AC_refine;
    }
  } else
    entropy->pub.encode_mcu = encode_mcu;

  /* Allocate & initialize requested statistics areas */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* DC needs no table for refinement scan */
    if (cinfo->progressive_mode == 0 || (cinfo->Ss == 0 && cinfo->Ah == 0)) {
      tbl = compptr->dc_tbl_no;
      if (tbl < 0 || tbl >= NUM_ARITH_TBLS)
        ERREXIT1(cinfo, JERR_NO_ARITH_TABLE, tbl);
      if (entropy->dc_stats[tbl] == NULL)
        entropy->dc_stats[tbl] = (unsigned char *)(*cinfo->mem->alloc_small)
          ((j_common_ptr)cinfo, JPOOL_IMAGE, DC_STAT_BINS);
      memset(entropy->dc_stats[tbl], 0, DC_STAT_BINS);
      /* Initialize DC predictions to 0 */
      entropy->last_dc_val[ci] = 0;
      entropy->dc_context[ci] = 0;
    }
    /* AC needs no table when not present */
    if (cinfo->progressive_mode == 0 || cinfo->Se) {
      tbl = compptr->ac_tbl_no;
      if (tbl < 0 || tbl >= NUM_ARITH_TBLS)
        ERREXIT1(cinfo, JERR_NO_ARITH_TABLE, tbl);
      if (entropy->ac_stats[tbl] == NULL)
        entropy->ac_stats[tbl] = (unsigned char *)(*cinfo->mem->alloc_small)
          ((j_common_ptr)cinfo, JPOOL_IMAGE, AC_STAT_BINS);
      memset(entropy->ac_stats[tbl], 0, AC_STAT_BINS);
#ifdef CALCULATE_SPECTRAL_CONDITIONING
      if (cinfo->progressive_mode)
        cinfo->arith_ac_K[tbl] = cinfo->Ss +
                                 ((8 + cinfo->Se - cinfo->Ss) >> 4);
#endif
    }
  }

  /* Initialize arithmetic encoding variables */
  entropy->c = 0;
  entropy->a = 0x10000L;
  entropy->sc = 0;
  entropy->zc = 0;
  entropy->ct = 11;
  entropy->buffer = -1;

  /* Initialize restart stuff */
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}

GLOBAL(void)
jinit_arith_encoder(j_compress_ptr cinfo)
{
  arith_entropy_ptr entropy;
  int i;

  entropy = (arith_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(arith_entropy_encoder));
  cinfo->entropy = (struct jpeg_entropy_encoder *)entropy;
  entropy->pub.start_pass = start_pass;
  entropy->pub.finish_pass = finish_pass;

  /* Mark tables unallocated */
  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    entropy->dc_stats[i] = NULL;
    entropy->ac_stats[i] = NULL;
  }

  /* Initialize index for fixed probability estimation */
  entropy->fixed_bin[0] = 113;
}
