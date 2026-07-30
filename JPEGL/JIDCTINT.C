#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"

#ifdef DCT_ISLOW_SUPPORTED

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCT blocks.
#endif

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1
#endif

#if CONST_BITS == 13
#define FIX_0_298631336  ((JLONG)2446)
#define FIX_0_390180644  ((JLONG)3196)
#define FIX_0_541196100  ((JLONG)4433)
#define FIX_0_765366865  ((JLONG)6270)
#define FIX_0_899976223  ((JLONG)7373)
#define FIX_1_175875602  ((JLONG)9633)
#define FIX_1_501321110  ((JLONG)12299)
#define FIX_1_847759065  ((JLONG)15137)
#define FIX_1_961570560  ((JLONG)16069)
#define FIX_2_053119869  ((JLONG)16819)
#define FIX_2_562915447  ((JLONG)20995)
#define FIX_3_072711026  ((JLONG)25172)
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var, const)  MULTIPLY16C16(var, const)
#else
#define MULTIPLY(var, const)  ((var) * (const))
#endif

#define DEQUANTIZE(coef, quantval)  (((ISLOW_MULT_TYPE)(coef)) * (quantval))

GLOBAL(void)
_jpeg_idct_islow(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp3;
  JLONG tmp10, tmp11, tmp12, tmp13;
  JLONG z1, z2, z3, z4, z5;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE2];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {

    if (inptr[DCTSIZE * 1] == 0 && inptr[DCTSIZE * 2] == 0 &&
        inptr[DCTSIZE * 3] == 0 && inptr[DCTSIZE * 4] == 0 &&
        inptr[DCTSIZE * 5] == 0 && inptr[DCTSIZE * 6] == 0 &&
        inptr[DCTSIZE * 7] == 0) {
      int dcval = LEFT_SHIFT(DEQUANTIZE(inptr[DCTSIZE * 0],
                             quantptr[DCTSIZE * 0]), PASS1_BITS);

      wsptr[DCTSIZE * 0] = dcval;
      wsptr[DCTSIZE * 1] = dcval;
      wsptr[DCTSIZE * 2] = dcval;
      wsptr[DCTSIZE * 3] = dcval;
      wsptr[DCTSIZE * 4] = dcval;
      wsptr[DCTSIZE * 5] = dcval;
      wsptr[DCTSIZE * 6] = dcval;
      wsptr[DCTSIZE * 7] = dcval;

      inptr++;
      quantptr++;
      wsptr++;
      continue;
    }

    z2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, -FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    z2 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);

    tmp0 = LEFT_SHIFT(z2 + z3, CONST_BITS);
    tmp1 = LEFT_SHIFT(z2 - z3, CONST_BITS);

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, -FIX_0_899976223); /* sqrt(2) * ( c7-c3) */
    z2 = MULTIPLY(z2, -FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, -FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, -FIX_0_390180644); /* sqrt(2) * ( c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    wsptr[DCTSIZE * 0] = (int)DESCALE(tmp10 + tmp3, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 7] = (int)DESCALE(tmp10 - tmp3, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 1] = (int)DESCALE(tmp11 + tmp2, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 6] = (int)DESCALE(tmp11 - tmp2, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 2] = (int)DESCALE(tmp12 + tmp1, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 5] = (int)DESCALE(tmp12 - tmp1, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 3] = (int)DESCALE(tmp13 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 4] = (int)DESCALE(tmp13 - tmp0, CONST_BITS - PASS1_BITS);

    inptr++;
    quantptr++;
    wsptr++;
  }

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;

#ifndef NO_ZERO_ROW_TEST
    if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 && wsptr[4] == 0 &&
        wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
      _JSAMPLE dcval = range_limit[(int)DESCALE((JLONG)wsptr[0],
                                                PASS1_BITS + 3) & RANGE_MASK];

      outptr[0] = dcval;
      outptr[1] = dcval;
      outptr[2] = dcval;
      outptr[3] = dcval;
      outptr[4] = dcval;
      outptr[5] = dcval;
      outptr[6] = dcval;
      outptr[7] = dcval;

      wsptr += DCTSIZE;
      continue;
    }
#endif

    z2 = (JLONG)wsptr[2];
    z3 = (JLONG)wsptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, -FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = LEFT_SHIFT((JLONG)wsptr[0] + (JLONG)wsptr[4], CONST_BITS);
    tmp1 = LEFT_SHIFT((JLONG)wsptr[0] - (JLONG)wsptr[4], CONST_BITS);

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (JLONG)wsptr[7];
    tmp1 = (JLONG)wsptr[5];
    tmp2 = (JLONG)wsptr[3];
    tmp3 = (JLONG)wsptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, -FIX_0_899976223); /* sqrt(2) * ( c7-c3) */
    z2 = MULTIPLY(z2, -FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, -FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, -FIX_0_390180644); /* sqrt(2) * ( c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    outptr[0] = range_limit[(int)DESCALE(tmp10 + tmp3,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[7] = range_limit[(int)DESCALE(tmp10 - tmp3,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)DESCALE(tmp11 + tmp2,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[6] = range_limit[(int)DESCALE(tmp11 - tmp2,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)DESCALE(tmp12 + tmp1,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)DESCALE(tmp12 - tmp1,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)DESCALE(tmp13 + tmp0,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)DESCALE(tmp13 - tmp0,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += DCTSIZE;
  }
}

#ifdef IDCT_SCALING_SUPPORTED

GLOBAL(void)
_jpeg_idct_7x7(j_decompress_ptr cinfo, jpeg_component_info *compptr,
               JCOEFPTR coef_block, _JSAMPARRAY output_buf,
               JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp10, tmp11, tmp12, tmp13;
  JLONG z1, z2, z3;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[7 * 7];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 7; ctr++, inptr++, quantptr++, wsptr++) {

    tmp13 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp13 = LEFT_SHIFT(tmp13, CONST_BITS);
    tmp13 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    tmp10 = MULTIPLY(z2 - z3, FIX(0.881747734));     /* c4 */
    tmp12 = MULTIPLY(z1 - z2, FIX(0.314692123));     /* c6 */
    tmp11 = tmp10 + tmp12 + tmp13 - MULTIPLY(z2, FIX(1.841218003)); /* c2+c4-c6 */
    tmp0 = z1 + z3;
    z2 -= tmp0;
    tmp0 = MULTIPLY(tmp0, FIX(1.274162392)) + tmp13; /* c2 */
    tmp10 += tmp0 - MULTIPLY(z3, FIX(0.077722536));  /* c2-c4-c6 */
    tmp12 += tmp0 - MULTIPLY(z1, FIX(2.470602249));  /* c2+c4+c6 */
    tmp13 += MULTIPLY(z2, FIX(1.414213562));         /* c0 */

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);

    tmp1 = MULTIPLY(z1 + z2, FIX(0.935414347));      /* (c3+c1-c5)/2 */
    tmp2 = MULTIPLY(z1 - z2, FIX(0.170262339));      /* (c3+c5-c1)/2 */
    tmp0 = tmp1 - tmp2;
    tmp1 += tmp2;
    tmp2 = MULTIPLY(z2 + z3, -FIX(1.378756276));     /* -c1 */
    tmp1 += tmp2;
    z2 = MULTIPLY(z1 + z3, FIX(0.613604268));        /* c5 */
    tmp0 += z2;
    tmp2 += z2 + MULTIPLY(z3, FIX(1.870828693));     /* c3+c1-c5 */

    wsptr[7 * 0] = (int)RIGHT_SHIFT(tmp10 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[7 * 6] = (int)RIGHT_SHIFT(tmp10 - tmp0, CONST_BITS - PASS1_BITS);
    wsptr[7 * 1] = (int)RIGHT_SHIFT(tmp11 + tmp1, CONST_BITS - PASS1_BITS);
    wsptr[7 * 5] = (int)RIGHT_SHIFT(tmp11 - tmp1, CONST_BITS - PASS1_BITS);
    wsptr[7 * 2] = (int)RIGHT_SHIFT(tmp12 + tmp2, CONST_BITS - PASS1_BITS);
    wsptr[7 * 4] = (int)RIGHT_SHIFT(tmp12 - tmp2, CONST_BITS - PASS1_BITS);
    wsptr[7 * 3] = (int)RIGHT_SHIFT(tmp13, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 7; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp13 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp13 = LEFT_SHIFT(tmp13, CONST_BITS);

    z1 = (JLONG)wsptr[2];
    z2 = (JLONG)wsptr[4];
    z3 = (JLONG)wsptr[6];

    tmp10 = MULTIPLY(z2 - z3, FIX(0.881747734));     /* c4 */
    tmp12 = MULTIPLY(z1 - z2, FIX(0.314692123));     /* c6 */
    tmp11 = tmp10 + tmp12 + tmp13 - MULTIPLY(z2, FIX(1.841218003)); /* c2+c4-c6 */
    tmp0 = z1 + z3;
    z2 -= tmp0;
    tmp0 = MULTIPLY(tmp0, FIX(1.274162392)) + tmp13; /* c2 */
    tmp10 += tmp0 - MULTIPLY(z3, FIX(0.077722536));  /* c2-c4-c6 */
    tmp12 += tmp0 - MULTIPLY(z1, FIX(2.470602249));  /* c2+c4+c6 */
    tmp13 += MULTIPLY(z2, FIX(1.414213562));         /* c0 */

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];

    tmp1 = MULTIPLY(z1 + z2, FIX(0.935414347));      /* (c3+c1-c5)/2 */
    tmp2 = MULTIPLY(z1 - z2, FIX(0.170262339));      /* (c3+c5-c1)/2 */
    tmp0 = tmp1 - tmp2;
    tmp1 += tmp2;
    tmp2 = MULTIPLY(z2 + z3, -FIX(1.378756276));     /* -c1 */
    tmp1 += tmp2;
    z2 = MULTIPLY(z1 + z3, FIX(0.613604268));        /* c5 */
    tmp0 += z2;
    tmp2 += z2 + MULTIPLY(z3, FIX(1.870828693));     /* c3+c1-c5 */

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp10 + tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[6] = range_limit[(int)RIGHT_SHIFT(tmp10 - tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp11 + tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)RIGHT_SHIFT(tmp11 - tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp12 + tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)RIGHT_SHIFT(tmp12 - tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)RIGHT_SHIFT(tmp13,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 7;
  }
}

GLOBAL(void)
_jpeg_idct_6x6(j_decompress_ptr cinfo, jpeg_component_info *compptr,
               JCOEFPTR coef_block, _JSAMPARRAY output_buf,
               JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp10, tmp11, tmp12;
  JLONG z1, z2, z3;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[6 * 6];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 6; ctr++, inptr++, quantptr++, wsptr++) {

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp0 += ONE << (CONST_BITS - PASS1_BITS - 1);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    tmp10 = MULTIPLY(tmp2, FIX(0.707106781));   /* c4 */
    tmp1 = tmp0 + tmp10;
    tmp11 = RIGHT_SHIFT(tmp0 - tmp10 - tmp10, CONST_BITS - PASS1_BITS);
    tmp10 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    tmp0 = MULTIPLY(tmp10, FIX(1.224744871));   /* c2 */
    tmp10 = tmp1 + tmp0;
    tmp12 = tmp1 - tmp0;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    tmp1 = MULTIPLY(z1 + z3, FIX(0.366025404)); /* c5 */
    tmp0 = tmp1 + LEFT_SHIFT(z1 + z2, CONST_BITS);
    tmp2 = tmp1 + LEFT_SHIFT(z3 - z2, CONST_BITS);
    tmp1 = LEFT_SHIFT(z1 - z2 - z3, PASS1_BITS);

    wsptr[6 * 0] = (int)RIGHT_SHIFT(tmp10 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[6 * 5] = (int)RIGHT_SHIFT(tmp10 - tmp0, CONST_BITS - PASS1_BITS);
    wsptr[6 * 1] = (int)(tmp11 + tmp1);
    wsptr[6 * 4] = (int)(tmp11 - tmp1);
    wsptr[6 * 2] = (int)RIGHT_SHIFT(tmp12 + tmp2, CONST_BITS - PASS1_BITS);
    wsptr[6 * 3] = (int)RIGHT_SHIFT(tmp12 - tmp2, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 6; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp0 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp2 = (JLONG)wsptr[4];
    tmp10 = MULTIPLY(tmp2, FIX(0.707106781));   /* c4 */
    tmp1 = tmp0 + tmp10;
    tmp11 = tmp0 - tmp10 - tmp10;
    tmp10 = (JLONG)wsptr[2];
    tmp0 = MULTIPLY(tmp10, FIX(1.224744871));   /* c2 */
    tmp10 = tmp1 + tmp0;
    tmp12 = tmp1 - tmp0;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    tmp1 = MULTIPLY(z1 + z3, FIX(0.366025404)); /* c5 */
    tmp0 = tmp1 + LEFT_SHIFT(z1 + z2, CONST_BITS);
    tmp2 = tmp1 + LEFT_SHIFT(z3 - z2, CONST_BITS);
    tmp1 = LEFT_SHIFT(z1 - z2 - z3, CONST_BITS);

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp10 + tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)RIGHT_SHIFT(tmp10 - tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp11 + tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)RIGHT_SHIFT(tmp11 - tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp12 + tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)RIGHT_SHIFT(tmp12 - tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 6;
  }
}

GLOBAL(void)
_jpeg_idct_5x5(j_decompress_ptr cinfo, jpeg_component_info *compptr,
               JCOEFPTR coef_block, _JSAMPARRAY output_buf,
               JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp10, tmp11, tmp12;
  JLONG z1, z2, z3;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[5 * 5];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 5; ctr++, inptr++, quantptr++, wsptr++) {

    tmp12 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp12 = LEFT_SHIFT(tmp12, CONST_BITS);
    tmp12 += ONE << (CONST_BITS - PASS1_BITS - 1);
    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z1 = MULTIPLY(tmp0 + tmp1, FIX(0.790569415)); /* (c2+c4)/2 */
    z2 = MULTIPLY(tmp0 - tmp1, FIX(0.353553391)); /* (c2-c4)/2 */
    z3 = tmp12 + z2;
    tmp10 = z3 + z1;
    tmp11 = z3 - z1;
    tmp12 -= LEFT_SHIFT(z2, 2);

    z2 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);

    z1 = MULTIPLY(z2 + z3, FIX(0.831253876));     /* c3 */
    tmp0 = z1 + MULTIPLY(z2, FIX(0.513743148));   /* c1-c3 */
    tmp1 = z1 - MULTIPLY(z3, FIX(2.176250899));   /* c1+c3 */

    wsptr[5 * 0] = (int)RIGHT_SHIFT(tmp10 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[5 * 4] = (int)RIGHT_SHIFT(tmp10 - tmp0, CONST_BITS - PASS1_BITS);
    wsptr[5 * 1] = (int)RIGHT_SHIFT(tmp11 + tmp1, CONST_BITS - PASS1_BITS);
    wsptr[5 * 3] = (int)RIGHT_SHIFT(tmp11 - tmp1, CONST_BITS - PASS1_BITS);
    wsptr[5 * 2] = (int)RIGHT_SHIFT(tmp12, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 5; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp12 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp12 = LEFT_SHIFT(tmp12, CONST_BITS);
    tmp0 = (JLONG)wsptr[2];
    tmp1 = (JLONG)wsptr[4];
    z1 = MULTIPLY(tmp0 + tmp1, FIX(0.790569415)); /* (c2+c4)/2 */
    z2 = MULTIPLY(tmp0 - tmp1, FIX(0.353553391)); /* (c2-c4)/2 */
    z3 = tmp12 + z2;
    tmp10 = z3 + z1;
    tmp11 = z3 - z1;
    tmp12 -= LEFT_SHIFT(z2, 2);

    z2 = (JLONG)wsptr[1];
    z3 = (JLONG)wsptr[3];

    z1 = MULTIPLY(z2 + z3, FIX(0.831253876));     /* c3 */
    tmp0 = z1 + MULTIPLY(z2, FIX(0.513743148));   /* c1-c3 */
    tmp1 = z1 - MULTIPLY(z3, FIX(2.176250899));   /* c1+c3 */

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp10 + tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)RIGHT_SHIFT(tmp10 - tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp11 + tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)RIGHT_SHIFT(tmp11 - tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp12,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 5;
  }
}

GLOBAL(void)
_jpeg_idct_3x3(j_decompress_ptr cinfo, jpeg_component_info *compptr,
               JCOEFPTR coef_block, _JSAMPARRAY output_buf,
               JDIMENSION output_col)
{
  JLONG tmp0, tmp2, tmp10, tmp12;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[3 * 3];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 3; ctr++, inptr++, quantptr++, wsptr++) {

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp0 += ONE << (CONST_BITS - PASS1_BITS - 1);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    tmp12 = MULTIPLY(tmp2, FIX(0.707106781)); /* c2 */
    tmp10 = tmp0 + tmp12;
    tmp2 = tmp0 - tmp12 - tmp12;

    tmp12 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    tmp0 = MULTIPLY(tmp12, FIX(1.224744871)); /* c1 */

    wsptr[3 * 0] = (int)RIGHT_SHIFT(tmp10 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[3 * 2] = (int)RIGHT_SHIFT(tmp10 - tmp0, CONST_BITS - PASS1_BITS);
    wsptr[3 * 1] = (int)RIGHT_SHIFT(tmp2, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 3; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp0 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp2 = (JLONG)wsptr[2];
    tmp12 = MULTIPLY(tmp2, FIX(0.707106781)); /* c2 */
    tmp10 = tmp0 + tmp12;
    tmp2 = tmp0 - tmp12 - tmp12;

    tmp12 = (JLONG)wsptr[1];
    tmp0 = MULTIPLY(tmp12, FIX(1.224744871)); /* c1 */

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp10 + tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp10 - tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 3;
  }
}

GLOBAL(void)
_jpeg_idct_9x9(j_decompress_ptr cinfo, jpeg_component_info *compptr,
               JCOEFPTR coef_block, _JSAMPARRAY output_buf,
               JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp3, tmp10, tmp11, tmp12, tmp13, tmp14;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 9];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp0 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    tmp3 = MULTIPLY(z3, FIX(0.707106781));      /* c6 */
    tmp1 = tmp0 + tmp3;
    tmp2 = tmp0 - tmp3 - tmp3;

    tmp0 = MULTIPLY(z1 - z2, FIX(0.707106781)); /* c6 */
    tmp11 = tmp2 + tmp0;
    tmp14 = tmp2 - tmp0 - tmp0;

    tmp0 = MULTIPLY(z1 + z2, FIX(1.328926049)); /* c2 */
    tmp2 = MULTIPLY(z1, FIX(1.083350441));      /* c4 */
    tmp3 = MULTIPLY(z2, FIX(0.245575608));      /* c8 */

    tmp10 = tmp1 + tmp0 - tmp3;
    tmp12 = tmp1 - tmp0 + tmp2;
    tmp13 = tmp1 - tmp2 + tmp3;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    z2 = MULTIPLY(z2, -FIX(1.224744871));            /* -c3 */

    tmp2 = MULTIPLY(z1 + z3, FIX(0.909038955));      /* c5 */
    tmp3 = MULTIPLY(z1 + z4, FIX(0.483689525));      /* c7 */
    tmp0 = tmp2 + tmp3 - z2;
    tmp1 = MULTIPLY(z3 - z4, FIX(1.392728481));      /* c1 */
    tmp2 += z2 - tmp1;
    tmp3 += z2 + tmp1;
    tmp1 = MULTIPLY(z1 - z3 - z4, FIX(1.224744871)); /* c3 */

    wsptr[8 * 0] = (int)RIGHT_SHIFT(tmp10 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8] = (int)RIGHT_SHIFT(tmp10 - tmp0, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1] = (int)RIGHT_SHIFT(tmp11 + tmp1, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7] = (int)RIGHT_SHIFT(tmp11 - tmp1, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2] = (int)RIGHT_SHIFT(tmp12 + tmp2, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6] = (int)RIGHT_SHIFT(tmp12 - tmp2, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3] = (int)RIGHT_SHIFT(tmp13 + tmp3, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5] = (int)RIGHT_SHIFT(tmp13 - tmp3, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4] = (int)RIGHT_SHIFT(tmp14, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 9; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp0 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);

    z1 = (JLONG)wsptr[2];
    z2 = (JLONG)wsptr[4];
    z3 = (JLONG)wsptr[6];

    tmp3 = MULTIPLY(z3, FIX(0.707106781));      /* c6 */
    tmp1 = tmp0 + tmp3;
    tmp2 = tmp0 - tmp3 - tmp3;

    tmp0 = MULTIPLY(z1 - z2, FIX(0.707106781)); /* c6 */
    tmp11 = tmp2 + tmp0;
    tmp14 = tmp2 - tmp0 - tmp0;

    tmp0 = MULTIPLY(z1 + z2, FIX(1.328926049)); /* c2 */
    tmp2 = MULTIPLY(z1, FIX(1.083350441));      /* c4 */
    tmp3 = MULTIPLY(z2, FIX(0.245575608));      /* c8 */

    tmp10 = tmp1 + tmp0 - tmp3;
    tmp12 = tmp1 - tmp0 + tmp2;
    tmp13 = tmp1 - tmp2 + tmp3;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];

    z2 = MULTIPLY(z2, -FIX(1.224744871));            /* -c3 */

    tmp2 = MULTIPLY(z1 + z3, FIX(0.909038955));      /* c5 */
    tmp3 = MULTIPLY(z1 + z4, FIX(0.483689525));      /* c7 */
    tmp0 = tmp2 + tmp3 - z2;
    tmp1 = MULTIPLY(z3 - z4, FIX(1.392728481));      /* c1 */
    tmp2 += z2 - tmp1;
    tmp3 += z2 + tmp1;
    tmp1 = MULTIPLY(z1 - z3 - z4, FIX(1.224744871)); /* c3 */

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp10 + tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[8] = range_limit[(int)RIGHT_SHIFT(tmp10 - tmp0,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp11 + tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[7] = range_limit[(int)RIGHT_SHIFT(tmp11 - tmp1,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp12 + tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[6] = range_limit[(int)RIGHT_SHIFT(tmp12 - tmp2,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)RIGHT_SHIFT(tmp13 + tmp3,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)RIGHT_SHIFT(tmp13 - tmp3,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)RIGHT_SHIFT(tmp14,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_10x10(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24;
  JLONG z1, z2, z3, z4, z5;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 10];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    z3 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z3 = LEFT_SHIFT(z3, CONST_BITS);
    z3 += ONE << (CONST_BITS - PASS1_BITS - 1);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z1 = MULTIPLY(z4, FIX(1.144122806));         /* c4 */
    z2 = MULTIPLY(z4, FIX(0.437016024));         /* c8 */
    tmp10 = z3 + z1;
    tmp11 = z3 - z2;

    tmp22 = RIGHT_SHIFT(z3 - LEFT_SHIFT(z1 - z2, 1),
                        CONST_BITS - PASS1_BITS); /* c0 = (c4-c8)*2 */

    z2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    z1 = MULTIPLY(z2 + z3, FIX(0.831253876));    /* c6 */
    tmp12 = z1 + MULTIPLY(z2, FIX(0.513743148)); /* c2-c6 */
    tmp13 = z1 - MULTIPLY(z3, FIX(2.176250899)); /* c2+c6 */

    tmp20 = tmp10 + tmp12;
    tmp24 = tmp10 - tmp12;
    tmp21 = tmp11 + tmp13;
    tmp23 = tmp11 - tmp13;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp11 = z2 + z4;
    tmp13 = z2 - z4;

    tmp12 = MULTIPLY(tmp13, FIX(0.309016994));        /* (c3-c7)/2 */
    z5 = LEFT_SHIFT(z3, CONST_BITS);

    z2 = MULTIPLY(tmp11, FIX(0.951056516));           /* (c3+c7)/2 */
    z4 = z5 + tmp12;

    tmp10 = MULTIPLY(z1, FIX(1.396802247)) + z2 + z4; /* c1 */
    tmp14 = MULTIPLY(z1, FIX(0.221231742)) - z2 + z4; /* c9 */

    z2 = MULTIPLY(tmp11, FIX(0.587785252));           /* (c1-c9)/2 */
    z4 = z5 - tmp12 - LEFT_SHIFT(tmp13, CONST_BITS - 1);

    tmp12 = LEFT_SHIFT(z1 - tmp13 - z3, PASS1_BITS);

    tmp11 = MULTIPLY(z1, FIX(1.260073511)) - z2 - z4; /* c3 */
    tmp13 = MULTIPLY(z1, FIX(0.642039522)) - z2 + z4; /* c7 */

    wsptr[8 * 0] = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1] = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8] = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2] = (int)(tmp22 + tmp12);
    wsptr[8 * 7] = (int)(tmp22 - tmp12);
    wsptr[8 * 3] = (int)RIGHT_SHIFT(tmp23 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6] = (int)RIGHT_SHIFT(tmp23 - tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4] = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5] = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 10; ctr++) {
    outptr = output_buf[ctr] + output_col;

    z3 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    z3 = LEFT_SHIFT(z3, CONST_BITS);
    z4 = (JLONG)wsptr[4];
    z1 = MULTIPLY(z4, FIX(1.144122806));         /* c4 */
    z2 = MULTIPLY(z4, FIX(0.437016024));         /* c8 */
    tmp10 = z3 + z1;
    tmp11 = z3 - z2;

    tmp22 = z3 - LEFT_SHIFT(z1 - z2, 1);         /* c0 = (c4-c8)*2 */

    z2 = (JLONG)wsptr[2];
    z3 = (JLONG)wsptr[6];

    z1 = MULTIPLY(z2 + z3, FIX(0.831253876));    /* c6 */
    tmp12 = z1 + MULTIPLY(z2, FIX(0.513743148)); /* c2-c6 */
    tmp13 = z1 - MULTIPLY(z3, FIX(2.176250899)); /* c2+c6 */

    tmp20 = tmp10 + tmp12;
    tmp24 = tmp10 - tmp12;
    tmp21 = tmp11 + tmp13;
    tmp23 = tmp11 - tmp13;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z3 = LEFT_SHIFT(z3, CONST_BITS);
    z4 = (JLONG)wsptr[7];

    tmp11 = z2 + z4;
    tmp13 = z2 - z4;

    tmp12 = MULTIPLY(tmp13, FIX(0.309016994));        /* (c3-c7)/2 */

    z2 = MULTIPLY(tmp11, FIX(0.951056516));           /* (c3+c7)/2 */
    z4 = z3 + tmp12;

    tmp10 = MULTIPLY(z1, FIX(1.396802247)) + z2 + z4; /* c1 */
    tmp14 = MULTIPLY(z1, FIX(0.221231742)) - z2 + z4; /* c9 */

    z2 = MULTIPLY(tmp11, FIX(0.587785252));           /* (c1-c9)/2 */
    z4 = z3 - tmp12 - LEFT_SHIFT(tmp13, CONST_BITS - 1);

    tmp12 = LEFT_SHIFT(z1 - tmp13, CONST_BITS) - z3;

    tmp11 = MULTIPLY(z1, FIX(1.260073511)) - z2 - z4; /* c3 */
    tmp13 = MULTIPLY(z1, FIX(0.642039522)) - z2 + z4; /* c7 */

    outptr[0] = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[9] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[8] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[7] = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[6] = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                             CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_11x11(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 11];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    tmp10 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp10 = LEFT_SHIFT(tmp10, CONST_BITS);
    tmp10 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    tmp20 = MULTIPLY(z2 - z3, FIX(2.546640132));     /* c2+c4 */
    tmp23 = MULTIPLY(z2 - z1, FIX(0.430815045));     /* c2-c6 */
    z4 = z1 + z3;
    tmp24 = MULTIPLY(z4, -FIX(1.155664402));         /* -(c2-c10) */
    z4 -= z2;
    tmp25 = tmp10 + MULTIPLY(z4, FIX(1.356927976));  /* c2 */
    tmp21 = tmp20 + tmp23 + tmp25 -
            MULTIPLY(z2, FIX(1.821790775));          /* c2+c4+c10-c6 */
    tmp20 += tmp25 + MULTIPLY(z3, FIX(2.115825087)); /* c4+c6 */
    tmp23 += tmp25 - MULTIPLY(z1, FIX(1.513598477)); /* c6+c8 */
    tmp24 += tmp25;
    tmp22 = tmp24 - MULTIPLY(z3, FIX(0.788749120));  /* c8+c10 */
    tmp24 += MULTIPLY(z2, FIX(1.944413522)) -        /* c2+c8 */
             MULTIPLY(z1, FIX(1.390975730));         /* c4+c10 */
    tmp25 = tmp10 - MULTIPLY(z4, FIX(1.414213562));  /* c0 */

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp11 = z1 + z2;
    tmp14 = MULTIPLY(tmp11 + z3 + z4, FIX(0.398430003)); /* c9 */
    tmp11 = MULTIPLY(tmp11, FIX(0.887983902));           /* c3-c9 */
    tmp12 = MULTIPLY(z1 + z3, FIX(0.670361295));         /* c5-c9 */
    tmp13 = tmp14 + MULTIPLY(z1 + z4, FIX(0.366151574)); /* c7-c9 */
    tmp10 = tmp11 + tmp12 + tmp13 -
            MULTIPLY(z1, FIX(0.923107866));              /* c7+c5+c3-c1-2*c9 */
    z1    = tmp14 - MULTIPLY(z2 + z3, FIX(1.163011579)); /* c7+c9 */
    tmp11 += z1 + MULTIPLY(z2, FIX(2.073276588));        /* c1+c7+3*c9-c3 */
    tmp12 += z1 - MULTIPLY(z3, FIX(1.192193623));        /* c3+c5-c7-c9 */
    z1    = MULTIPLY(z2 + z4, -FIX(1.798248910));        /* -(c1+c9) */
    tmp11 += z1;
    tmp13 += z1 + MULTIPLY(z4, FIX(2.102458632));        /* c1+c5+c9-c7 */
    tmp14 += MULTIPLY(z2, -FIX(1.467221301)) +           /* -(c5+c9) */
             MULTIPLY(z3, FIX(1.001388905)) -            /* c1-c9 */
             MULTIPLY(z4, FIX(1.684843907));             /* c3+c9 */

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 10] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp22 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)RIGHT_SHIFT(tmp23 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp23 - tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 11; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp10 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp10 = LEFT_SHIFT(tmp10, CONST_BITS);

    z1 = (JLONG)wsptr[2];
    z2 = (JLONG)wsptr[4];
    z3 = (JLONG)wsptr[6];

    tmp20 = MULTIPLY(z2 - z3, FIX(2.546640132));     /* c2+c4 */
    tmp23 = MULTIPLY(z2 - z1, FIX(0.430815045));     /* c2-c6 */
    z4 = z1 + z3;
    tmp24 = MULTIPLY(z4, -FIX(1.155664402));         /* -(c2-c10) */
    z4 -= z2;
    tmp25 = tmp10 + MULTIPLY(z4, FIX(1.356927976));  /* c2 */
    tmp21 = tmp20 + tmp23 + tmp25 -
            MULTIPLY(z2, FIX(1.821790775));          /* c2+c4+c10-c6 */
    tmp20 += tmp25 + MULTIPLY(z3, FIX(2.115825087)); /* c4+c6 */
    tmp23 += tmp25 - MULTIPLY(z1, FIX(1.513598477)); /* c6+c8 */
    tmp24 += tmp25;
    tmp22 = tmp24 - MULTIPLY(z3, FIX(0.788749120));  /* c8+c10 */
    tmp24 += MULTIPLY(z2, FIX(1.944413522)) -        /* c2+c8 */
             MULTIPLY(z1, FIX(1.390975730));         /* c4+c10 */
    tmp25 = tmp10 - MULTIPLY(z4, FIX(1.414213562));  /* c0 */

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];

    tmp11 = z1 + z2;
    tmp14 = MULTIPLY(tmp11 + z3 + z4, FIX(0.398430003)); /* c9 */
    tmp11 = MULTIPLY(tmp11, FIX(0.887983902));           /* c3-c9 */
    tmp12 = MULTIPLY(z1 + z3, FIX(0.670361295));         /* c5-c9 */
    tmp13 = tmp14 + MULTIPLY(z1 + z4, FIX(0.366151574)); /* c7-c9 */
    tmp10 = tmp11 + tmp12 + tmp13 -
            MULTIPLY(z1, FIX(0.923107866));              /* c7+c5+c3-c1-2*c9 */
    z1    = tmp14 - MULTIPLY(z2 + z3, FIX(1.163011579)); /* c7+c9 */
    tmp11 += z1 + MULTIPLY(z2, FIX(2.073276588));        /* c1+c7+3*c9-c3 */
    tmp12 += z1 - MULTIPLY(z3, FIX(1.192193623));        /* c3+c5-c7-c9 */
    z1    = MULTIPLY(z2 + z4, -FIX(1.798248910));        /* -(c1+c9) */
    tmp11 += z1;
    tmp13 += z1 + MULTIPLY(z4, FIX(2.102458632));        /* c1+c5+c9-c7 */
    tmp14 += MULTIPLY(z2, -FIX(1.467221301)) +           /* -(c5+c9) */
             MULTIPLY(z3, FIX(1.001388905)) -            /* c1-c9 */
             MULTIPLY(z4, FIX(1.684843907));             /* c3+c9 */

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_12x12(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14, tmp15;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 12];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    z3 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z3 = LEFT_SHIFT(z3, CONST_BITS);
    z3 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z4 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z4 = MULTIPLY(z4, FIX(1.224744871)); /* c4 */

    tmp10 = z3 + z4;
    tmp11 = z3 - z4;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z4 = MULTIPLY(z1, FIX(1.366025404)); /* c2 */
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);
    z2 = LEFT_SHIFT(z2, CONST_BITS);

    tmp12 = z1 - z2;

    tmp21 = z3 + tmp12;
    tmp24 = z3 - tmp12;

    tmp12 = z4 + z2;

    tmp20 = tmp10 + tmp12;
    tmp25 = tmp10 - tmp12;

    tmp12 = z4 - z1 - z2;

    tmp22 = tmp11 + tmp12;
    tmp23 = tmp11 - tmp12;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp11 = MULTIPLY(z2, FIX(1.306562965));                  /* c3 */
    tmp14 = MULTIPLY(z2, -FIX_0_541196100);                  /* -c9 */

    tmp10 = z1 + z3;
    tmp15 = MULTIPLY(tmp10 + z4, FIX(0.860918669));          /* c7 */
    tmp12 = tmp15 + MULTIPLY(tmp10, FIX(0.261052384));       /* c5-c7 */
    tmp10 = tmp12 + tmp11 + MULTIPLY(z1, FIX(0.280143716));  /* c1-c5 */
    tmp13 = MULTIPLY(z3 + z4, -FIX(1.045510580));            /* -(c7+c11) */
    tmp12 += tmp13 + tmp14 - MULTIPLY(z3, FIX(1.478575242)); /* c1+c5-c7-c11 */
    tmp13 += tmp15 - tmp11 + MULTIPLY(z4, FIX(1.586706681)); /* c1+c11 */
    tmp15 += tmp14 - MULTIPLY(z1, FIX(0.676326758)) -        /* c7-c11 */
             MULTIPLY(z4, FIX(1.982889723));                 /* c5+c7 */

    z1 -= z4;
    z2 -= z3;
    z3 = MULTIPLY(z1 + z2, FIX_0_541196100);                 /* c9 */
    tmp11 = z3 + MULTIPLY(z1, FIX_0_765366865);              /* c3-c9 */
    tmp14 = z3 - MULTIPLY(z2, FIX_1_847759065);              /* c3+c9 */

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 11] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 10] = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp22 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)RIGHT_SHIFT(tmp23 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp23 - tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25 + tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp25 - tmp15, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 12; ctr++) {
    outptr = output_buf[ctr] + output_col;

    z3 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    z3 = LEFT_SHIFT(z3, CONST_BITS);

    z4 = (JLONG)wsptr[4];
    z4 = MULTIPLY(z4, FIX(1.224744871)); /* c4 */

    tmp10 = z3 + z4;
    tmp11 = z3 - z4;

    z1 = (JLONG)wsptr[2];
    z4 = MULTIPLY(z1, FIX(1.366025404)); /* c2 */
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z2 = (JLONG)wsptr[6];
    z2 = LEFT_SHIFT(z2, CONST_BITS);

    tmp12 = z1 - z2;

    tmp21 = z3 + tmp12;
    tmp24 = z3 - tmp12;

    tmp12 = z4 + z2;

    tmp20 = tmp10 + tmp12;
    tmp25 = tmp10 - tmp12;

    tmp12 = z4 - z1 - z2;

    tmp22 = tmp11 + tmp12;
    tmp23 = tmp11 - tmp12;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];

    tmp11 = MULTIPLY(z2, FIX(1.306562965));                  /* c3 */
    tmp14 = MULTIPLY(z2, -FIX_0_541196100);                  /* -c9 */

    tmp10 = z1 + z3;
    tmp15 = MULTIPLY(tmp10 + z4, FIX(0.860918669));          /* c7 */
    tmp12 = tmp15 + MULTIPLY(tmp10, FIX(0.261052384));       /* c5-c7 */
    tmp10 = tmp12 + tmp11 + MULTIPLY(z1, FIX(0.280143716));  /* c1-c5 */
    tmp13 = MULTIPLY(z3 + z4, -FIX(1.045510580));            /* -(c7+c11) */
    tmp12 += tmp13 + tmp14 - MULTIPLY(z3, FIX(1.478575242)); /* c1+c5-c7-c11 */
    tmp13 += tmp15 - tmp11 + MULTIPLY(z4, FIX(1.586706681)); /* c1+c11 */
    tmp15 += tmp14 - MULTIPLY(z1, FIX(0.676326758)) -        /* c7-c11 */
             MULTIPLY(z4, FIX(1.982889723));                 /* c5+c7 */

    z1 -= z4;
    z2 -= z3;
    z3 = MULTIPLY(z1 + z2, FIX_0_541196100);                 /* c9 */
    tmp11 = z3 + MULTIPLY(z1, FIX_0_765366865);              /* c3-c9 */
    tmp14 = z3 - MULTIPLY(z2, FIX_1_847759065);              /* c3+c9 */

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[11] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25 + tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp25 - tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_13x13(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14, tmp15;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25, tmp26;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 13];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    z1 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z1 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    tmp10 = z3 + z4;
    tmp11 = z3 - z4;

    tmp12 = MULTIPLY(tmp10, FIX(1.155388986));                /* (c4+c6)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.096834934)) + z1;           /* (c4-c6)/2 */

    tmp20 = MULTIPLY(z2, FIX(1.373119086)) + tmp12 + tmp13;   /* c2 */
    tmp22 = MULTIPLY(z2, FIX(0.501487041)) - tmp12 + tmp13;   /* c10 */

    tmp12 = MULTIPLY(tmp10, FIX(0.316450131));                /* (c8-c12)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.486914739)) + z1;           /* (c8+c12)/2 */

    tmp21 = MULTIPLY(z2, FIX(1.058554052)) - tmp12 + tmp13;   /* c6 */
    tmp25 = MULTIPLY(z2, -FIX(1.252223920)) + tmp12 + tmp13;  /* c4 */

    tmp12 = MULTIPLY(tmp10, FIX(0.435816023));                /* (c2-c10)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.937303064)) - z1;           /* (c2+c10)/2 */

    tmp23 = MULTIPLY(z2, -FIX(0.170464608)) - tmp12 - tmp13;  /* c12 */
    tmp24 = MULTIPLY(z2, -FIX(0.803364869)) + tmp12 - tmp13;  /* c8 */

    tmp26 = MULTIPLY(tmp11 - z2, FIX(1.414213562)) + z1;      /* c0 */

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp11 = MULTIPLY(z1 + z2, FIX(1.322312651));     /* c3 */
    tmp12 = MULTIPLY(z1 + z3, FIX(1.163874945));     /* c5 */
    tmp15 = z1 + z4;
    tmp13 = MULTIPLY(tmp15, FIX(0.937797057));       /* c7 */
    tmp10 = tmp11 + tmp12 + tmp13 -
            MULTIPLY(z1, FIX(2.020082300));          /* c7+c5+c3-c1 */
    tmp14 = MULTIPLY(z2 + z3, -FIX(0.338443458));    /* -c11 */
    tmp11 += tmp14 + MULTIPLY(z2, FIX(0.837223564)); /* c5+c9+c11-c3 */
    tmp12 += tmp14 - MULTIPLY(z3, FIX(1.572116027)); /* c1+c5-c9-c11 */
    tmp14 = MULTIPLY(z2 + z4, -FIX(1.163874945));    /* -c5 */
    tmp11 += tmp14;
    tmp13 += tmp14 + MULTIPLY(z4, FIX(2.205608352)); /* c3+c5+c9-c7 */
    tmp14 = MULTIPLY(z3 + z4, -FIX(0.657217813));    /* -c9 */
    tmp12 += tmp14;
    tmp13 += tmp14;
    tmp15 = MULTIPLY(tmp15, FIX(0.338443458));       /* c11 */
    tmp14 = tmp15 + MULTIPLY(z1, FIX(0.318774355)) - /* c9-c11 */
            MULTIPLY(z2, FIX(0.466105296));          /* c1-c7 */
    z1    = MULTIPLY(z3 - z2, FIX(0.937797057));     /* c7 */
    tmp14 += z1;
    tmp15 += z1 + MULTIPLY(z3, FIX(0.384515595)) -   /* c3-c7 */
             MULTIPLY(z4, FIX(1.742345811));         /* c1+c11 */

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 12] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 11] = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 10] = (int)RIGHT_SHIFT(tmp22 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)RIGHT_SHIFT(tmp23 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp23 - tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25 + tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp25 - tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp26, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 13; ctr++) {
    outptr = output_buf[ctr] + output_col;

    z1 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    z1 = LEFT_SHIFT(z1, CONST_BITS);

    z2 = (JLONG)wsptr[2];
    z3 = (JLONG)wsptr[4];
    z4 = (JLONG)wsptr[6];

    tmp10 = z3 + z4;
    tmp11 = z3 - z4;

    tmp12 = MULTIPLY(tmp10, FIX(1.155388986));                /* (c4+c6)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.096834934)) + z1;           /* (c4-c6)/2 */

    tmp20 = MULTIPLY(z2, FIX(1.373119086)) + tmp12 + tmp13;   /* c2 */
    tmp22 = MULTIPLY(z2, FIX(0.501487041)) - tmp12 + tmp13;   /* c10 */

    tmp12 = MULTIPLY(tmp10, FIX(0.316450131));                /* (c8-c12)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.486914739)) + z1;           /* (c8+c12)/2 */

    tmp21 = MULTIPLY(z2, FIX(1.058554052)) - tmp12 + tmp13;   /* c6 */
    tmp25 = MULTIPLY(z2, -FIX(1.252223920)) + tmp12 + tmp13;  /* c4 */

    tmp12 = MULTIPLY(tmp10, FIX(0.435816023));                /* (c2-c10)/2 */
    tmp13 = MULTIPLY(tmp11, FIX(0.937303064)) - z1;           /* (c2+c10)/2 */

    tmp23 = MULTIPLY(z2, -FIX(0.170464608)) - tmp12 - tmp13;  /* c12 */
    tmp24 = MULTIPLY(z2, -FIX(0.803364869)) + tmp12 - tmp13;  /* c8 */

    tmp26 = MULTIPLY(tmp11 - z2, FIX(1.414213562)) + z1;      /* c0 */

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];

    tmp11 = MULTIPLY(z1 + z2, FIX(1.322312651));     /* c3 */
    tmp12 = MULTIPLY(z1 + z3, FIX(1.163874945));     /* c5 */
    tmp15 = z1 + z4;
    tmp13 = MULTIPLY(tmp15, FIX(0.937797057));       /* c7 */
    tmp10 = tmp11 + tmp12 + tmp13 -
            MULTIPLY(z1, FIX(2.020082300));          /* c7+c5+c3-c1 */
    tmp14 = MULTIPLY(z2 + z3, -FIX(0.338443458));    /* -c11 */
    tmp11 += tmp14 + MULTIPLY(z2, FIX(0.837223564)); /* c5+c9+c11-c3 */
    tmp12 += tmp14 - MULTIPLY(z3, FIX(1.572116027)); /* c1+c5-c9-c11 */
    tmp14 = MULTIPLY(z2 + z4, -FIX(1.163874945));    /* -c5 */
    tmp11 += tmp14;
    tmp13 += tmp14 + MULTIPLY(z4, FIX(2.205608352)); /* c3+c5+c9-c7 */
    tmp14 = MULTIPLY(z3 + z4, -FIX(0.657217813));    /* -c9 */
    tmp12 += tmp14;
    tmp13 += tmp14;
    tmp15 = MULTIPLY(tmp15, FIX(0.338443458));       /* c11 */
    tmp14 = tmp15 + MULTIPLY(z1, FIX(0.318774355)) - /* c9-c11 */
            MULTIPLY(z2, FIX(0.466105296));          /* c1-c7 */
    z1    = MULTIPLY(z3 - z2, FIX(0.937797057));     /* c7 */
    tmp14 += z1;
    tmp15 += z1 + MULTIPLY(z3, FIX(0.384515595)) -   /* c3-c7 */
             MULTIPLY(z4, FIX(1.742345811));         /* c1+c11 */

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[12] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[11] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25 + tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp25 - tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp26,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_14x14(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, tmp16;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25, tmp26;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 14];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    z1 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z1 += ONE << (CONST_BITS - PASS1_BITS - 1);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z2 = MULTIPLY(z4, FIX(1.274162392));         /* c4 */
    z3 = MULTIPLY(z4, FIX(0.314692123));         /* c12 */
    z4 = MULTIPLY(z4, FIX(0.881747734));         /* c8 */

    tmp10 = z1 + z2;
    tmp11 = z1 + z3;
    tmp12 = z1 - z4;

    tmp23 = RIGHT_SHIFT(z1 - LEFT_SHIFT(z2 + z3 - z4, 1),
                        CONST_BITS - PASS1_BITS); /* c0 = (c4+c12-c8)*2 */

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    z3 = MULTIPLY(z1 + z2, FIX(1.105676686));    /* c6 */

    tmp13 = z3 + MULTIPLY(z1, FIX(0.273079590)); /* c2-c6 */
    tmp14 = z3 - MULTIPLY(z2, FIX(1.719280954)); /* c6+c10 */
    tmp15 = MULTIPLY(z1, FIX(0.613604268)) -     /* c10 */
            MULTIPLY(z2, FIX(1.378756276));      /* c2 */

    tmp20 = tmp10 + tmp13;
    tmp26 = tmp10 - tmp13;
    tmp21 = tmp11 + tmp14;
    tmp25 = tmp11 - tmp14;
    tmp22 = tmp12 + tmp15;
    tmp24 = tmp12 - tmp15;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);
    tmp13 = LEFT_SHIFT(z4, CONST_BITS);

    tmp14 = z1 + z3;
    tmp11 = MULTIPLY(z1 + z2, FIX(1.334852607));           /* c3 */
    tmp12 = MULTIPLY(tmp14, FIX(1.197448846));             /* c5 */
    tmp10 = tmp11 + tmp12 + tmp13 - MULTIPLY(z1, FIX(1.126980169)); /* c3+c5-c1 */
    tmp14 = MULTIPLY(tmp14, FIX(0.752406978));             /* c9 */
    tmp16 = tmp14 - MULTIPLY(z1, FIX(1.061150426));        /* c9+c11-c13 */
    z1    -= z2;
    tmp15 = MULTIPLY(z1, FIX(0.467085129)) - tmp13;        /* c11 */
    tmp16 += tmp15;
    z1    += z4;
    z4    = MULTIPLY(z2 + z3, -FIX(0.158341681)) - tmp13;  /* -c13 */
    tmp11 += z4 - MULTIPLY(z2, FIX(0.424103948));          /* c3-c9-c13 */
    tmp12 += z4 - MULTIPLY(z3, FIX(2.373959773));          /* c3+c5-c13 */
    z4    = MULTIPLY(z3 - z2, FIX(1.405321284));           /* c1 */
    tmp14 += z4 + tmp13 - MULTIPLY(z3, FIX(1.6906431334)); /* c1+c9-c11 */
    tmp15 += z4 + MULTIPLY(z2, FIX(0.674957567));          /* c1+c11-c5 */

    tmp13 = LEFT_SHIFT(z1 - z3, PASS1_BITS);

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 13] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 12] = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 11] = (int)RIGHT_SHIFT(tmp22 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)(tmp23 + tmp13);
    wsptr[8 * 10] = (int)(tmp23 - tmp13);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25 + tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp25 - tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp26 + tmp16, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp26 - tmp16, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 14; ctr++) {
    outptr = output_buf[ctr] + output_col;

    z1 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z4 = (JLONG)wsptr[4];
    z2 = MULTIPLY(z4, FIX(1.274162392));         /* c4 */
    z3 = MULTIPLY(z4, FIX(0.314692123));         /* c12 */
    z4 = MULTIPLY(z4, FIX(0.881747734));         /* c8 */

    tmp10 = z1 + z2;
    tmp11 = z1 + z3;
    tmp12 = z1 - z4;

    tmp23 = z1 - LEFT_SHIFT(z2 + z3 - z4, 1);    /* c0 = (c4+c12-c8)*2 */

    z1 = (JLONG)wsptr[2];
    z2 = (JLONG)wsptr[6];

    z3 = MULTIPLY(z1 + z2, FIX(1.105676686));    /* c6 */

    tmp13 = z3 + MULTIPLY(z1, FIX(0.273079590)); /* c2-c6 */
    tmp14 = z3 - MULTIPLY(z2, FIX(1.719280954)); /* c6+c10 */
    tmp15 = MULTIPLY(z1, FIX(0.613604268)) -     /* c10 */
            MULTIPLY(z2, FIX(1.378756276));      /* c2 */

    tmp20 = tmp10 + tmp13;
    tmp26 = tmp10 - tmp13;
    tmp21 = tmp11 + tmp14;
    tmp25 = tmp11 - tmp14;
    tmp22 = tmp12 + tmp15;
    tmp24 = tmp12 - tmp15;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];
    z4 = LEFT_SHIFT(z4, CONST_BITS);

    tmp14 = z1 + z3;
    tmp11 = MULTIPLY(z1 + z2, FIX(1.334852607));           /* c3 */
    tmp12 = MULTIPLY(tmp14, FIX(1.197448846));             /* c5 */
    tmp10 = tmp11 + tmp12 + z4 - MULTIPLY(z1, FIX(1.126980169)); /* c3+c5-c1 */
    tmp14 = MULTIPLY(tmp14, FIX(0.752406978));             /* c9 */
    tmp16 = tmp14 - MULTIPLY(z1, FIX(1.061150426));        /* c9+c11-c13 */
    z1    -= z2;
    tmp15 = MULTIPLY(z1, FIX(0.467085129)) - z4;           /* c11 */
    tmp16 += tmp15;
    tmp13 = MULTIPLY(z2 + z3, -FIX(0.158341681)) - z4;     /* -c13 */
    tmp11 += tmp13 - MULTIPLY(z2, FIX(0.424103948));       /* c3-c9-c13 */
    tmp12 += tmp13 - MULTIPLY(z3, FIX(2.373959773));       /* c3+c5-c13 */
    tmp13 = MULTIPLY(z3 - z2, FIX(1.405321284));           /* c1 */
    tmp14 += tmp13 + z4 - MULTIPLY(z3, FIX(1.6906431334)); /* c1+c9-c11 */
    tmp15 += tmp13 + MULTIPLY(z2, FIX(0.674957567));       /* c1+c11-c5 */

    tmp13 = LEFT_SHIFT(z1 - z3, CONST_BITS) + z4;

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[13] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[12] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[11] = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25 + tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp25 - tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp26 + tmp16,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp26 - tmp16,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_15x15(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, tmp16;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25, tmp26, tmp27;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 15];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    z1 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z1 = LEFT_SHIFT(z1, CONST_BITS);
    z1 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    tmp10 = MULTIPLY(z4, FIX(0.437016024)); /* c12 */
    tmp11 = MULTIPLY(z4, FIX(1.144122806)); /* c6 */

    tmp12 = z1 - tmp10;
    tmp13 = z1 + tmp11;
    z1 -= LEFT_SHIFT(tmp11 - tmp10, 1);     /* c0 = (c6-c12)*2 */

    z4 = z2 - z3;
    z3 += z2;
    tmp10 = MULTIPLY(z3, FIX(1.337628990)); /* (c2+c4)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.045680613)); /* (c2-c4)/2 */
    z2 = MULTIPLY(z2, FIX(1.439773946));    /* c4+c14 */

    tmp20 = tmp13 + tmp10 + tmp11;
    tmp23 = tmp12 - tmp10 + tmp11 + z2;

    tmp10 = MULTIPLY(z3, FIX(0.547059574)); /* (c8+c14)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.399234004)); /* (c8-c14)/2 */

    tmp25 = tmp13 - tmp10 - tmp11;
    tmp26 = tmp12 + tmp10 - tmp11 - z2;

    tmp10 = MULTIPLY(z3, FIX(0.790569415)); /* (c6+c12)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.353553391)); /* (c6-c12)/2 */

    tmp21 = tmp12 + tmp10 + tmp11;
    tmp24 = tmp13 - tmp10 + tmp11;
    tmp11 += tmp11;
    tmp22 = z1 + tmp11;                     /* c10 = c6-c12 */
    tmp27 = z1 - tmp11 - tmp11;             /* c0 = (c6-c12)*2 */

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z3 = MULTIPLY(z4, FIX(1.224744871));                    /* c5 */
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp13 = z2 - z4;
    tmp15 = MULTIPLY(z1 + tmp13, FIX(0.831253876));         /* c9 */
    tmp11 = tmp15 + MULTIPLY(z1, FIX(0.513743148));         /* c3-c9 */
    tmp14 = tmp15 - MULTIPLY(tmp13, FIX(2.176250899));      /* c3+c9 */

    tmp13 = MULTIPLY(z2, -FIX(0.831253876));                /* -c9 */
    tmp15 = MULTIPLY(z2, -FIX(1.344997024));                /* -c3 */
    z2 = z1 - z4;
    tmp12 = z3 + MULTIPLY(z2, FIX(1.406466353));            /* c1 */

    tmp10 = tmp12 + MULTIPLY(z4, FIX(2.457431844)) - tmp15; /* c1+c7 */
    tmp16 = tmp12 - MULTIPLY(z1, FIX(1.112434820)) + tmp13; /* c1-c13 */
    tmp12 = MULTIPLY(z2, FIX(1.224744871)) - z3;            /* c5 */
    z2 = MULTIPLY(z1 + z4, FIX(0.575212477));               /* c11 */
    tmp13 += z2 + MULTIPLY(z1, FIX(0.475753014)) - z3;      /* c7-c11 */
    tmp15 += z2 - MULTIPLY(z4, FIX(0.869244010)) + z3;      /* c11+c13 */

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 14] = (int)RIGHT_SHIFT(tmp20 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 13] = (int)RIGHT_SHIFT(tmp21 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 12] = (int)RIGHT_SHIFT(tmp22 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)RIGHT_SHIFT(tmp23 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 11] = (int)RIGHT_SHIFT(tmp23 - tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 10] = (int)RIGHT_SHIFT(tmp24 - tmp14, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25 + tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp25 - tmp15, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp26 + tmp16, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp26 - tmp16, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp27, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 15; ctr++) {
    outptr = output_buf[ctr] + output_col;

    z1 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    z1 = LEFT_SHIFT(z1, CONST_BITS);

    z2 = (JLONG)wsptr[2];
    z3 = (JLONG)wsptr[4];
    z4 = (JLONG)wsptr[6];

    tmp10 = MULTIPLY(z4, FIX(0.437016024)); /* c12 */
    tmp11 = MULTIPLY(z4, FIX(1.144122806)); /* c6 */

    tmp12 = z1 - tmp10;
    tmp13 = z1 + tmp11;
    z1 -= LEFT_SHIFT(tmp11 - tmp10, 1);     /* c0 = (c6-c12)*2 */

    z4 = z2 - z3;
    z3 += z2;
    tmp10 = MULTIPLY(z3, FIX(1.337628990)); /* (c2+c4)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.045680613)); /* (c2-c4)/2 */
    z2 = MULTIPLY(z2, FIX(1.439773946));    /* c4+c14 */

    tmp20 = tmp13 + tmp10 + tmp11;
    tmp23 = tmp12 - tmp10 + tmp11 + z2;

    tmp10 = MULTIPLY(z3, FIX(0.547059574)); /* (c8+c14)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.399234004)); /* (c8-c14)/2 */

    tmp25 = tmp13 - tmp10 - tmp11;
    tmp26 = tmp12 + tmp10 - tmp11 - z2;

    tmp10 = MULTIPLY(z3, FIX(0.790569415)); /* (c6+c12)/2 */
    tmp11 = MULTIPLY(z4, FIX(0.353553391)); /* (c6-c12)/2 */

    tmp21 = tmp12 + tmp10 + tmp11;
    tmp24 = tmp13 - tmp10 + tmp11;
    tmp11 += tmp11;
    tmp22 = z1 + tmp11;                     /* c10 = c6-c12 */
    tmp27 = z1 - tmp11 - tmp11;             /* c0 = (c6-c12)*2 */

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z4 = (JLONG)wsptr[5];
    z3 = MULTIPLY(z4, FIX(1.224744871));                    /* c5 */
    z4 = (JLONG)wsptr[7];

    tmp13 = z2 - z4;
    tmp15 = MULTIPLY(z1 + tmp13, FIX(0.831253876));         /* c9 */
    tmp11 = tmp15 + MULTIPLY(z1, FIX(0.513743148));         /* c3-c9 */
    tmp14 = tmp15 - MULTIPLY(tmp13, FIX(2.176250899));      /* c3+c9 */

    tmp13 = MULTIPLY(z2, -FIX(0.831253876));                /* -c9 */
    tmp15 = MULTIPLY(z2, -FIX(1.344997024));                /* -c3 */
    z2 = z1 - z4;
    tmp12 = z3 + MULTIPLY(z2, FIX(1.406466353));            /* c1 */

    tmp10 = tmp12 + MULTIPLY(z4, FIX(2.457431844)) - tmp15; /* c1+c7 */
    tmp16 = tmp12 - MULTIPLY(z1, FIX(1.112434820)) + tmp13; /* c1-c13 */
    tmp12 = MULTIPLY(z2, FIX(1.224744871)) - z3;            /* c5 */
    z2 = MULTIPLY(z1 + z4, FIX(0.575212477));               /* c11 */
    tmp13 += z2 + MULTIPLY(z1, FIX(0.475753014)) - z3;      /* c7-c11 */
    tmp15 += z2 - MULTIPLY(z4, FIX(0.869244010)) + z3;      /* c11+c13 */

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[14] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[13] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[12] = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[11] = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp14,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25 + tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp25 - tmp15,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp26 + tmp16,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp26 - tmp16,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp27,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

GLOBAL(void)
_jpeg_idct_16x16(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp3, tmp10, tmp11, tmp12, tmp13;
  JLONG tmp20, tmp21, tmp22, tmp23, tmp24, tmp25, tmp26, tmp27;
  JLONG z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[8 * 16];
  SHIFT_TEMPS

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = 0; ctr < 8; ctr++, inptr++, quantptr++, wsptr++) {

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);
    tmp0 += ONE << (CONST_BITS - PASS1_BITS - 1);

    z1 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);
    tmp1 = MULTIPLY(z1, FIX(1.306562965));      /* c4[16] = c2[8] */
    tmp2 = MULTIPLY(z1, FIX_0_541196100);       /* c12[16] = c6[8] */

    tmp10 = tmp0 + tmp1;
    tmp11 = tmp0 - tmp1;
    tmp12 = tmp0 + tmp2;
    tmp13 = tmp0 - tmp2;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);
    z3 = z1 - z2;
    z4 = MULTIPLY(z3, FIX(0.275899379));        /* c14[16] = c7[8] */
    z3 = MULTIPLY(z3, FIX(1.387039845));        /* c2[16] = c1[8] */

    tmp0 = z3 + MULTIPLY(z2, FIX_2_562915447);  /* (c6+c2)[16] = (c3+c1)[8] */
    tmp1 = z4 + MULTIPLY(z1, FIX_0_899976223);  /* (c6-c14)[16] = (c3-c7)[8] */
    tmp2 = z3 - MULTIPLY(z1, FIX(0.601344887)); /* (c2-c10)[16] = (c1-c5)[8] */
    tmp3 = z4 - MULTIPLY(z2, FIX(0.509795579)); /* (c10-c14)[16] = (c5-c7)[8] */

    tmp20 = tmp10 + tmp0;
    tmp27 = tmp10 - tmp0;
    tmp21 = tmp12 + tmp1;
    tmp26 = tmp12 - tmp1;
    tmp22 = tmp13 + tmp2;
    tmp25 = tmp13 - tmp2;
    tmp23 = tmp11 + tmp3;
    tmp24 = tmp11 - tmp3;

    z1 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);
    z2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    z4 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);

    tmp11 = z1 + z3;

    tmp1  = MULTIPLY(z1 + z2, FIX(1.353318001));   /* c3 */
    tmp2  = MULTIPLY(tmp11,   FIX(1.247225013));   /* c5 */
    tmp3  = MULTIPLY(z1 + z4, FIX(1.093201867));   /* c7 */
    tmp10 = MULTIPLY(z1 - z4, FIX(0.897167586));   /* c9 */
    tmp11 = MULTIPLY(tmp11,   FIX(0.666655658));   /* c11 */
    tmp12 = MULTIPLY(z1 - z2, FIX(0.410524528));   /* c13 */
    tmp0  = tmp1 + tmp2 + tmp3 -
            MULTIPLY(z1, FIX(2.286341144));        /* c7+c5+c3-c1 */
    tmp13 = tmp10 + tmp11 + tmp12 -
            MULTIPLY(z1, FIX(1.835730603));        /* c9+c11+c13-c15 */
    z1    = MULTIPLY(z2 + z3, FIX(0.138617169));   /* c15 */
    tmp1  += z1 + MULTIPLY(z2, FIX(0.071888074));  /* c9+c11-c3-c15 */
    tmp2  += z1 - MULTIPLY(z3, FIX(1.125726048));  /* c5+c7+c15-c3 */
    z1    = MULTIPLY(z3 - z2, FIX(1.407403738));   /* c1 */
    tmp11 += z1 - MULTIPLY(z3, FIX(0.766367282));  /* c1+c11-c9-c13 */
    tmp12 += z1 + MULTIPLY(z2, FIX(1.971951411));  /* c1+c5+c13-c7 */
    z2    += z4;
    z1    = MULTIPLY(z2, -FIX(0.666655658));       /* -c11 */
    tmp1  += z1;
    tmp3  += z1 + MULTIPLY(z4, FIX(1.065388962));  /* c3+c11+c15-c7 */
    z2    = MULTIPLY(z2, -FIX(1.247225013));       /* -c5 */
    tmp10 += z2 + MULTIPLY(z4, FIX(3.141271809));  /* c1+c5+c9-c13 */
    tmp12 += z2;
    z2    = MULTIPLY(z3 + z4, -FIX(1.353318001));  /* -c3 */
    tmp2  += z2;
    tmp3  += z2;
    z2    = MULTIPLY(z4 - z3, FIX(0.410524528));   /* c13 */
    tmp10 += z2;
    tmp11 += z2;

    wsptr[8 * 0]  = (int)RIGHT_SHIFT(tmp20 + tmp0,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 15] = (int)RIGHT_SHIFT(tmp20 - tmp0,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 1]  = (int)RIGHT_SHIFT(tmp21 + tmp1,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 14] = (int)RIGHT_SHIFT(tmp21 - tmp1,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 2]  = (int)RIGHT_SHIFT(tmp22 + tmp2,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 13] = (int)RIGHT_SHIFT(tmp22 - tmp2,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 3]  = (int)RIGHT_SHIFT(tmp23 + tmp3,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 12] = (int)RIGHT_SHIFT(tmp23 - tmp3,  CONST_BITS - PASS1_BITS);
    wsptr[8 * 4]  = (int)RIGHT_SHIFT(tmp24 + tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 11] = (int)RIGHT_SHIFT(tmp24 - tmp10, CONST_BITS - PASS1_BITS);
    wsptr[8 * 5]  = (int)RIGHT_SHIFT(tmp25 + tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 10] = (int)RIGHT_SHIFT(tmp25 - tmp11, CONST_BITS - PASS1_BITS);
    wsptr[8 * 6]  = (int)RIGHT_SHIFT(tmp26 + tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 9]  = (int)RIGHT_SHIFT(tmp26 - tmp12, CONST_BITS - PASS1_BITS);
    wsptr[8 * 7]  = (int)RIGHT_SHIFT(tmp27 + tmp13, CONST_BITS - PASS1_BITS);
    wsptr[8 * 8]  = (int)RIGHT_SHIFT(tmp27 - tmp13, CONST_BITS - PASS1_BITS);
  }

  wsptr = workspace;
  for (ctr = 0; ctr < 16; ctr++) {
    outptr = output_buf[ctr] + output_col;

    tmp0 = (JLONG)wsptr[0] + (ONE << (PASS1_BITS + 2));
    tmp0 = LEFT_SHIFT(tmp0, CONST_BITS);

    z1 = (JLONG)wsptr[4];
    tmp1 = MULTIPLY(z1, FIX(1.306562965));      /* c4[16] = c2[8] */
    tmp2 = MULTIPLY(z1, FIX_0_541196100);       /* c12[16] = c6[8] */

    tmp10 = tmp0 + tmp1;
    tmp11 = tmp0 - tmp1;
    tmp12 = tmp0 + tmp2;
    tmp13 = tmp0 - tmp2;

    z1 = (JLONG)wsptr[2];
    z2 = (JLONG)wsptr[6];
    z3 = z1 - z2;
    z4 = MULTIPLY(z3, FIX(0.275899379));        /* c14[16] = c7[8] */
    z3 = MULTIPLY(z3, FIX(1.387039845));        /* c2[16] = c1[8] */

    tmp0 = z3 + MULTIPLY(z2, FIX_2_562915447);  /* (c6+c2)[16] = (c3+c1)[8] */
    tmp1 = z4 + MULTIPLY(z1, FIX_0_899976223);  /* (c6-c14)[16] = (c3-c7)[8] */
    tmp2 = z3 - MULTIPLY(z1, FIX(0.601344887)); /* (c2-c10)[16] = (c1-c5)[8] */
    tmp3 = z4 - MULTIPLY(z2, FIX(0.509795579)); /* (c10-c14)[16] = (c5-c7)[8] */

    tmp20 = tmp10 + tmp0;
    tmp27 = tmp10 - tmp0;
    tmp21 = tmp12 + tmp1;
    tmp26 = tmp12 - tmp1;
    tmp22 = tmp13 + tmp2;
    tmp25 = tmp13 - tmp2;
    tmp23 = tmp11 + tmp3;
    tmp24 = tmp11 - tmp3;

    z1 = (JLONG)wsptr[1];
    z2 = (JLONG)wsptr[3];
    z3 = (JLONG)wsptr[5];
    z4 = (JLONG)wsptr[7];

    tmp11 = z1 + z3;

    tmp1  = MULTIPLY(z1 + z2, FIX(1.353318001));   /* c3 */
    tmp2  = MULTIPLY(tmp11,   FIX(1.247225013));   /* c5 */
    tmp3  = MULTIPLY(z1 + z4, FIX(1.093201867));   /* c7 */
    tmp10 = MULTIPLY(z1 - z4, FIX(0.897167586));   /* c9 */
    tmp11 = MULTIPLY(tmp11,   FIX(0.666655658));   /* c11 */
    tmp12 = MULTIPLY(z1 - z2, FIX(0.410524528));   /* c13 */
    tmp0  = tmp1 + tmp2 + tmp3 -
            MULTIPLY(z1, FIX(2.286341144));        /* c7+c5+c3-c1 */
    tmp13 = tmp10 + tmp11 + tmp12 -
            MULTIPLY(z1, FIX(1.835730603));        /* c9+c11+c13-c15 */
    z1    = MULTIPLY(z2 + z3, FIX(0.138617169));   /* c15 */
    tmp1  += z1 + MULTIPLY(z2, FIX(0.071888074));  /* c9+c11-c3-c15 */
    tmp2  += z1 - MULTIPLY(z3, FIX(1.125726048));  /* c5+c7+c15-c3 */
    z1    = MULTIPLY(z3 - z2, FIX(1.407403738));   /* c1 */
    tmp11 += z1 - MULTIPLY(z3, FIX(0.766367282));  /* c1+c11-c9-c13 */
    tmp12 += z1 + MULTIPLY(z2, FIX(1.971951411));  /* c1+c5+c13-c7 */
    z2    += z4;
    z1    = MULTIPLY(z2, -FIX(0.666655658));       /* -c11 */
    tmp1  += z1;
    tmp3  += z1 + MULTIPLY(z4, FIX(1.065388962));  /* c3+c11+c15-c7 */
    z2    = MULTIPLY(z2, -FIX(1.247225013));       /* -c5 */
    tmp10 += z2 + MULTIPLY(z4, FIX(3.141271809));  /* c1+c5+c9-c13 */
    tmp12 += z2;
    z2    = MULTIPLY(z3 + z4, -FIX(1.353318001));  /* -c3 */
    tmp2  += z2;
    tmp3  += z2;
    z2    = MULTIPLY(z4 - z3, FIX(0.410524528));   /* c13 */
    tmp10 += z2;
    tmp11 += z2;

    outptr[0]  = range_limit[(int)RIGHT_SHIFT(tmp20 + tmp0,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[15] = range_limit[(int)RIGHT_SHIFT(tmp20 - tmp0,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[1]  = range_limit[(int)RIGHT_SHIFT(tmp21 + tmp1,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[14] = range_limit[(int)RIGHT_SHIFT(tmp21 - tmp1,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[2]  = range_limit[(int)RIGHT_SHIFT(tmp22 + tmp2,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[13] = range_limit[(int)RIGHT_SHIFT(tmp22 - tmp2,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[3]  = range_limit[(int)RIGHT_SHIFT(tmp23 + tmp3,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[12] = range_limit[(int)RIGHT_SHIFT(tmp23 - tmp3,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[4]  = range_limit[(int)RIGHT_SHIFT(tmp24 + tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[11] = range_limit[(int)RIGHT_SHIFT(tmp24 - tmp10,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[5]  = range_limit[(int)RIGHT_SHIFT(tmp25 + tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[10] = range_limit[(int)RIGHT_SHIFT(tmp25 - tmp11,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[6]  = range_limit[(int)RIGHT_SHIFT(tmp26 + tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[9]  = range_limit[(int)RIGHT_SHIFT(tmp26 - tmp12,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[7]  = range_limit[(int)RIGHT_SHIFT(tmp27 + tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];
    outptr[8]  = range_limit[(int)RIGHT_SHIFT(tmp27 - tmp13,
                                              CONST_BITS + PASS1_BITS + 3) &
                             RANGE_MASK];

    wsptr += 8;
  }
}

#endif /* IDCT_SCALING_SUPPORTED */
#endif /* DCT_ISLOW_SUPPORTED */
