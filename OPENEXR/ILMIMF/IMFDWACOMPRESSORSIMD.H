///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2014 DreamWorks Animation LLC. 
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifndef IMF_DWACOMPRESSORSIMD_H_HAS_BEEN_INCLUDED
#define IMF_DWACOMPRESSORSIMD_H_HAS_BEEN_INCLUDED

//
// Various SSE accelerated functions, used by Imf::DwaCompressor. 
// These have been separated into a separate .h file, as the fast
// paths are done with template specialization.
//
// Unless otherwise noted, all pointers are assumed to be 32-byte 
// aligned. Unaligned pointers may risk seg-faulting.
//

#include "ImfNamespace.h"
#include "ImfSimd.h"
#include "ImfSystemSpecific.h"
#include "OpenEXRConfig.h"

#include <half.h>
#include <assert.h>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

#define _SSE_ALIGNMENT        32
#define _SSE_ALIGNMENT_MASK 0x0F
#define _AVX_ALIGNMENT_MASK 0x1F

//
// Test if we should enable GCC inline asm paths for AVX
//

#ifdef OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX 

    #define IMF_HAVE_GCC_INLINEASM

    #ifdef __LP64__
        #define IMF_HAVE_GCC_INLINEASM_64
    #endif /* __LP64__ */

#endif /* OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX */

//
// A simple 64-element array, aligned properly for SIMD access. 
//

template <class T>
class SimdAlignedBuffer64
{
    public:

        SimdAlignedBuffer64(): _buffer (0), _handle (0)           
        {
            alloc();
        }

        SimdAlignedBuffer64(const SimdAlignedBuffer64 &rhs): _handle(0)
        {
            alloc();
            memcpy (_buffer, rhs._buffer, 64 * sizeof (T));
        }

        ~SimdAlignedBuffer64 ()
        {
            EXRFreeAligned (_handle);
            _handle = 0;
            _buffer = 0;
        }

        void alloc()
        {
            //
            // Try EXRAllocAligned first - but it might fallback to
            // unaligned allocs. If so, overalloc.
            //

            _handle = (char *) EXRAllocAligned
                (64 * sizeof(T), _SSE_ALIGNMENT);

            if (((size_t)_handle & (_SSE_ALIGNMENT - 1)) == 0)
            {
                _buffer = (T *)_handle;
                return;
            }

            EXRFreeAligned(_handle);
            _handle = (char *) EXRAllocAligned
                (64 * sizeof(T) + _SSE_ALIGNMENT, _SSE_ALIGNMENT);

            char *aligned = _handle;

            while ((size_t)aligned & (_SSE_ALIGNMENT - 1))
                aligned++;

            _buffer = (T *)aligned;    
        }

        T     *_buffer;

    private:

        char  *_handle;
};

typedef SimdAlignedBuffer64<float>          SimdAlignedBuffer64f;
typedef SimdAlignedBuffer64<unsigned short> SimdAlignedBuffer64us;

namespace {

//
// Color space conversion, Inverse 709 CSC, Y'CbCr -> R'G'B'
//

void
csc709Inverse (float &comp0, float &comp1, float &comp2)
{
    float src[3];

    src[0] = comp0;
    src[1] = comp1;
    src[2] = comp2;

    comp0 = src[0]                    + 1.5747f * src[2];
    comp1 = src[0] - 0.1873f * src[1] - 0.4682f * src[2];
    comp2 = src[0] + 1.8556f * src[1];
}

void
csc709Inverse64 (float *comp0, float *comp1, float *comp2)
{
    __m128 c0 = { 1.5747f,  1.5747f,  1.5747f,  1.5747f};
    __m128 c1 = { 1.8556f,  1.8556f,  1.8556f,  1.8556f};
    __m128 c2 = {-0.1873f, -0.1873f, -0.1873f, -0.1873f};
    __m128 c3 = {-0.4682f, -0.4682f, -0.4682f, -0.4682f}; 

    __m128 *r = (__m128 *)comp0;
    __m128 *g = (__m128 *)comp1;
    __m128 *b = (__m128 *)comp2;
    __m128 src[3];

    #define CSC_INVERSE_709_SSE2_LOOP(i)                       \
            src[0] = r[i];                                     \
            src[1] = g[i];                                     \
            src[2] = b[i];                                     \
                                                               \
            r[i] = _mm_add_ps (r[i], _mm_mul_ps (src[2], c0)); \
                                                               \
            g[i]   = _mm_mul_ps (g[i], c2);                    \
            src[2] = _mm_mul_ps (src[2], c3);                  \
            g[i]   = _mm_add_ps (g[i], src[0]);                \
            g[i]   = _mm_add_ps (g[i], src[2]);                \
                                                               \
            b[i] = _mm_mul_ps (c1,   src[1]);                  \
            b[i] = _mm_add_ps (b[i], src[0]);

    CSC_INVERSE_709_SSE2_LOOP (0)
    CSC_INVERSE_709_SSE2_LOOP (1)
    CSC_INVERSE_709_SSE2_LOOP (2)
    CSC_INVERSE_709_SSE2_LOOP (3)

    CSC_INVERSE_709_SSE2_LOOP (4)
    CSC_INVERSE_709_SSE2_LOOP (5)
    CSC_INVERSE_709_SSE2_LOOP (6)
    CSC_INVERSE_709_SSE2_LOOP (7)

    CSC_INVERSE_709_SSE2_LOOP (8)
    CSC_INVERSE_709_SSE2_LOOP (9)
    CSC_INVERSE_709_SSE2_LOOP (10)
    CSC_INVERSE_709_SSE2_LOOP (11)

    CSC_INVERSE_709_SSE2_LOOP (12)
    CSC_INVERSE_709_SSE2_LOOP (13)
    CSC_INVERSE_709_SSE2_LOOP (14)
    CSC_INVERSE_709_SSE2_LOOP (15)
}

void
csc709Forward64 (float *comp0, float *comp1, float *comp2)
{
    float src[3];

    for (int i = 0; i<64; ++i)
    {
        src[0] = comp0[i];    
        src[1] = comp1[i]; 
        src[2] = comp2[i];     

        comp0[i] =  0.2126f * src[0] + 0.7152f * src[1] + 0.0722f * src[2];
        comp1[i] = -0.1146f * src[0] - 0.3854f * src[1] + 0.5000f * src[2];
        comp2[i] =  0.5000f * src[0] - 0.4542f * src[1] - 0.0458f * src[2];
    }
}

void
interleaveByte2 (char *dst, char *src0, char *src1, int numBytes)
{
    int dstAlignment  = (size_t)dst  % 16;
    int src0Alignment = (size_t)src0 % 16;
    int src1Alignment = (size_t)src1 % 16;

    __m128i *dst_epi8  = (__m128i*)dst;
    __m128i *src0_epi8 = (__m128i*)src0;
    __m128i *src1_epi8 = (__m128i*)src1;
    int sseWidth  =  numBytes / 16;

    if ((!dstAlignment) && (!src0Alignment) && (!src1Alignment))
    {
        __m128i tmp0, tmp1;

        for (int x = 0; x < sseWidth; ++x)
        {
            tmp0 = src0_epi8[x];
            tmp1 = src1_epi8[x];

            _mm_stream_si128 (&dst_epi8[2 * x],
                              _mm_unpacklo_epi8 (tmp0, tmp1));

            _mm_stream_si128 (&dst_epi8[2 * x + 1],
                              _mm_unpackhi_epi8 (tmp0, tmp1));
        }

        for (int x = 16 * sseWidth; x < numBytes; ++x)
        {
            dst[2 * x]     = src0[x];
            dst[2 * x + 1] = src1[x];
        }
    }
    else if ((!dstAlignment) && (src0Alignment == 8) && (src1Alignment == 8))
    {
        for (int x = 0; x < 8; ++x)
        {
            dst[2 * x]     = src0[x];
            dst[2 * x + 1] = src1[x];
        }

        dst_epi8  = (__m128i*)&dst[16];
        src0_epi8 = (__m128i*)&src0[8];
        src1_epi8 = (__m128i*)&src1[8];
        sseWidth  =  (numBytes - 8) / 16;

        for (int x=0; x<sseWidth; ++x)
        {
            _mm_stream_si128 (&dst_epi8[2 * x],
                              _mm_unpacklo_epi8 (src0_epi8[x], src1_epi8[x]));

            _mm_stream_si128 (&dst_epi8[2 * x + 1],
                              _mm_unpackhi_epi8 (src0_epi8[x], src1_epi8[x]));
        }

        for (int x = 16 * sseWidth + 8; x < numBytes; ++x)
        {
            dst[2 * x]     = src0[x];
            dst[2 * x + 1] = src1[x];
        }
    }
    else
    {
        for (int x = 0; x < sseWidth; ++x)
        {
            __m128i tmpSrc0_epi8 = _mm_loadu_si128 (&src0_epi8[x]);
            __m128i tmpSrc1_epi8 = _mm_loadu_si128 (&src1_epi8[x]);

            _mm_storeu_si128 (&dst_epi8[2 * x],
                              _mm_unpacklo_epi8 (tmpSrc0_epi8, tmpSrc1_epi8));

            _mm_storeu_si128 (&dst_epi8[2 * x + 1],
                              _mm_unpackhi_epi8 (tmpSrc0_epi8, tmpSrc1_epi8));
        }

        for (int x = 16 * sseWidth; x < numBytes; ++x)
        {
            dst[2 * x]     = src0[x];
            dst[2 * x + 1] = src1[x];
        }
    }
}

void 
convertFloatToHalf64_scalar (unsigned short *dst, float *src)
{
    for (int i=0; i<64; ++i)
        dst[i] = ((half)src[i]).bits();
}

void
convertFloatToHalf64_f16c (unsigned short *dst, float *src)
{
        convertFloatToHalf64_scalar (dst, src);
}

void
fromHalfZigZag_scalar (unsigned short *src, float *dst)
{
    half *srcHalf = (half *)src;

    dst[0] = (float)srcHalf[0];
    dst[1] = (float)srcHalf[1];
    dst[2] = (float)srcHalf[5];
    dst[3] = (float)srcHalf[6];
    dst[4] = (float)srcHalf[14];
    dst[5] = (float)srcHalf[15];
    dst[6] = (float)srcHalf[27];
    dst[7] = (float)srcHalf[28];
    dst[8] = (float)srcHalf[2];
    dst[9] = (float)srcHalf[4];

    dst[10] = (float)srcHalf[7];
    dst[11] = (float)srcHalf[13];
    dst[12] = (float)srcHalf[16];
    dst[13] = (float)srcHalf[26];
    dst[14] = (float)srcHalf[29];
    dst[15] = (float)srcHalf[42];
    dst[16] = (float)srcHalf[3];
    dst[17] = (float)srcHalf[8];
    dst[18] = (float)srcHalf[12];
    dst[19] = (float)srcHalf[17];

    dst[20] = (float)srcHalf[25];
    dst[21] = (float)srcHalf[30];
    dst[22] = (float)srcHalf[41];
    dst[23] = (float)srcHalf[43];
    dst[24] = (float)srcHalf[9];
    dst[25] = (float)srcHalf[11];
    dst[26] = (float)srcHalf[18];
    dst[27] = (float)srcHalf[24];
    dst[28] = (float)srcHalf[31];
    dst[29] = (float)srcHalf[40];

    dst[30] = (float)srcHalf[44];
    dst[31] = (float)srcHalf[53];
    dst[32] = (float)srcHalf[10];
    dst[33] = (float)srcHalf[19];
    dst[34] = (float)srcHalf[23];
    dst[35] = (float)srcHalf[32];
    dst[36] = (float)srcHalf[39];
    dst[37] = (float)srcHalf[45];
    dst[38] = (float)srcHalf[52];
    dst[39] = (float)srcHalf[54];

    dst[40] = (float)srcHalf[20];
    dst[41] = (float)srcHalf[22];
    dst[42] = (float)srcHalf[33];
    dst[43] = (float)srcHalf[38];
    dst[44] = (float)srcHalf[46];
    dst[45] = (float)srcHalf[51];
    dst[46] = (float)srcHalf[55];
    dst[47] = (float)srcHalf[60];
    dst[48] = (float)srcHalf[21];
    dst[49] = (float)srcHalf[34];

    dst[50] = (float)srcHalf[37];
    dst[51] = (float)srcHalf[47];
    dst[52] = (float)srcHalf[50];
    dst[53] = (float)srcHalf[56];
    dst[54] = (float)srcHalf[59];
    dst[55] = (float)srcHalf[61];
    dst[56] = (float)srcHalf[35];
    dst[57] = (float)srcHalf[36];
    dst[58] = (float)srcHalf[48];
    dst[59] = (float)srcHalf[49];

    dst[60] = (float)srcHalf[57];
    dst[61] = (float)srcHalf[58];
    dst[62] = (float)srcHalf[62];
    dst[63] = (float)srcHalf[63];
}

void 
fromHalfZigZag_f16c (unsigned short *src, float *dst)
{
        fromHalfZigZag_scalar(src, dst);
}

void
dctInverse8x8DcOnly (float *data)
{
    __m128 src = _mm_set1_ps (data[0] * 3.535536e-01f * 3.535536e-01f);
    __m128 *dst = (__m128 *)data;

    for (int i = 0; i < 16; ++i)
        dst[i] = src;
}

template <int zeroedRows>
void
dctInverse8x8_scalar (float *data)
{
    const float a = .5f * cosf (3.14159f / 4.0f);
    const float b = .5f * cosf (3.14159f / 16.0f);
    const float c = .5f * cosf (3.14159f / 8.0f);
    const float d = .5f * cosf (3.f*3.14159f / 16.0f);
    const float e = .5f * cosf (5.f*3.14159f / 16.0f);
    const float f = .5f * cosf (3.f*3.14159f / 8.0f);
    const float g = .5f * cosf (7.f*3.14159f / 16.0f);

    float alpha[4], beta[4], theta[4], gamma[4];

    float *rowPtr = NULL;

    for (int row = 0; row < 8 - zeroedRows; ++row)
    {
        rowPtr = data + row * 8;

        alpha[0] = c * rowPtr[2]; 
        alpha[1] = f * rowPtr[2]; 
        alpha[2] = c * rowPtr[6]; 
        alpha[3] = f * rowPtr[6]; 

        beta[0] = b * rowPtr[1] + d * rowPtr[3] + e * rowPtr[5] + g * rowPtr[7];
        beta[1] = d * rowPtr[1] - g * rowPtr[3] - b * rowPtr[5] - e * rowPtr[7];
        beta[2] = e * rowPtr[1] - b * rowPtr[3] + g * rowPtr[5] + d * rowPtr[7];
        beta[3] = g * rowPtr[1] - e * rowPtr[3] + d * rowPtr[5] - b * rowPtr[7];

        theta[0] = a * (rowPtr[0] + rowPtr[4]);
        theta[3] = a * (rowPtr[0] - rowPtr[4]);

        theta[1] = alpha[0] + alpha[3]; 
        theta[2] = alpha[1] - alpha[2]; 


        gamma[0] = theta[0] + theta[1];
        gamma[1] = theta[3] + theta[2];
        gamma[2] = theta[3] - theta[2];
        gamma[3] = theta[0] - theta[1];


        rowPtr[0] = gamma[0] + beta[0];
        rowPtr[1] = gamma[1] + beta[1];
        rowPtr[2] = gamma[2] + beta[2];
        rowPtr[3] = gamma[3] + beta[3];

        rowPtr[4] = gamma[3] - beta[3];
        rowPtr[5] = gamma[2] - beta[2];
        rowPtr[6] = gamma[1] - beta[1];
        rowPtr[7] = gamma[0] - beta[0];
    }

    for (int column = 0; column < 8; ++column)
    {
        alpha[0] = c * data[16+column]; 
        alpha[1] = f * data[16+column]; 
        alpha[2] = c * data[48+column]; 
        alpha[3] = f * data[48+column]; 

        beta[0] = b * data[8+column]  + d * data[24+column] +
                  e * data[40+column] + g * data[56+column];

        beta[1] = d * data[8+column]  - g * data[24+column] -
                  b * data[40+column] - e * data[56+column];

        beta[2] = e * data[8+column]  - b * data[24+column] + 
                  g * data[40+column] + d * data[56+column];

        beta[3] = g * data[8+column]  - e * data[24+column] + 
                  d * data[40+column] - b * data[56+column];

        theta[0] = a * (data[column] + data[32+column]);
        theta[3] = a * (data[column] - data[32+column]);

        theta[1] = alpha[0] + alpha[3]; 
        theta[2] = alpha[1] - alpha[2]; 

        gamma[0] = theta[0] + theta[1];
        gamma[1] = theta[3] + theta[2];
        gamma[2] = theta[3] - theta[2];
        gamma[3] = theta[0] - theta[1];

        data[     column] = gamma[0] + beta[0];
        data[ 8 + column] = gamma[1] + beta[1];
        data[16 + column] = gamma[2] + beta[2];
        data[24 + column] = gamma[3] + beta[3];

        data[32 + column] = gamma[3] - beta[3];
        data[40 + column] = gamma[2] - beta[2];
        data[48 + column] = gamma[1] - beta[1];
        data[56 + column] = gamma[0] - beta[0];
    }
}

template <int zeroedRows>
void
dctInverse8x8_sse2 (float *data)
{
        __m128 a  = {3.535536e-01f,3.535536e-01f,3.535536e-01f,3.535536e-01f};
        __m128 b  = {4.903927e-01f,4.903927e-01f,4.903927e-01f,4.903927e-01f};
        __m128 c  = {4.619398e-01f,4.619398e-01f,4.619398e-01f,4.619398e-01f};
        __m128 d  = {4.157349e-01f,4.157349e-01f,4.157349e-01f,4.157349e-01f};
        __m128 e  = {2.777855e-01f,2.777855e-01f,2.777855e-01f,2.777855e-01f};
        __m128 f  = {1.913422e-01f,1.913422e-01f,1.913422e-01f,1.913422e-01f};
        __m128 g  = {9.754573e-02f,9.754573e-02f,9.754573e-02f,9.754573e-02f};

        __m128 c0 = {3.535536e-01f, 3.535536e-01f, 3.535536e-01f, 3.535536e-01f};
        __m128 c1 = {4.619398e-01f, 1.913422e-01f,-1.913422e-01f,-4.619398e-01f};
        __m128 c2 = {3.535536e-01f,-3.535536e-01f,-3.535536e-01f, 3.535536e-01f};
        __m128 c3 = {1.913422e-01f,-4.619398e-01f, 4.619398e-01f,-1.913422e-01f};

        __m128 c4 = {4.903927e-01f, 4.157349e-01f, 2.777855e-01f, 9.754573e-02f};
        __m128 c5 = {4.157349e-01f,-9.754573e-02f,-4.903927e-01f,-2.777855e-01f};
        __m128 c6 = {2.777855e-01f,-4.903927e-01f, 9.754573e-02f, 4.157349e-01f};
        __m128 c7 = {9.754573e-02f,-2.777855e-01f, 4.157349e-01f,-4.903927e-01f};

        __m128 *srcVec = (__m128 *)data;
        __m128 x[8], evenSum, oddSum;
        __m128 in[8], alpha[4], beta[4], theta[4], gamma[4];
        
        #define DCT_INVERSE_8x8_SS2_ROW_LOOP(i)                             \
            x[0] = _mm_shuffle_ps (srcVec[2 * i],                           \
                                   srcVec[2 * i],                           \
                                   _MM_SHUFFLE (0, 0, 0, 0));               \
                                                                            \
            x[1] = _mm_shuffle_ps (srcVec[2 * i],                           \
                                   srcVec[2 * i],                           \
                                   _MM_SHUFFLE (1, 1, 1, 1));               \
                                                                            \
            x[2] = _mm_shuffle_ps (srcVec[2 * i],                           \
                                   srcVec[2 * i],                           \
                                   _MM_SHUFFLE (2, 2, 2, 2));               \
                                                                            \
            x[3] = _mm_shuffle_ps (srcVec[2 * i],                           \
                                   srcVec[2 * i],                           \
                                   _MM_SHUFFLE (3, 3, 3, 3));               \
                                                                            \
            x[4] = _mm_shuffle_ps (srcVec[2 * i + 1],                       \
                                   srcVec[2 * i + 1],                       \
                                   _MM_SHUFFLE (0, 0, 0, 0));               \
                                                                            \
            x[5] = _mm_shuffle_ps (srcVec[2 * i + 1],                       \
                                   srcVec[2 * i + 1],                       \
                                   _MM_SHUFFLE (1, 1, 1, 1));               \
                                                                            \
            x[6] = _mm_shuffle_ps (srcVec[2 * i + 1],                       \
                                   srcVec[2 * i + 1],                       \
                                   _MM_SHUFFLE (2, 2, 2, 2));               \
                                                                            \
            x[7] = _mm_shuffle_ps (srcVec[2 * i + 1],                       \
                                   srcVec[2 * i + 1],                       \
                                   _MM_SHUFFLE (3, 3, 3, 3));               \
                                                                            \
            x[0] = _mm_mul_ps (x[0], c0);                                   \
            x[2] = _mm_mul_ps (x[2], c1);                                   \
            x[4] = _mm_mul_ps (x[4], c2);                                   \
            x[6] = _mm_mul_ps (x[6], c3);                                   \
                                                                            \
            x[1] = _mm_mul_ps (x[1], c4);                                   \
            x[3] = _mm_mul_ps (x[3], c5);                                   \
            x[5] = _mm_mul_ps (x[5], c6);                                   \
            x[7] = _mm_mul_ps (x[7], c7);                                   \
                                                                            \
                                                                            \
            evenSum = _mm_setzero_ps();                                     \
            evenSum = _mm_add_ps (evenSum, x[0]);                           \
            evenSum = _mm_add_ps (evenSum, x[2]);                           \
            evenSum = _mm_add_ps (evenSum, x[4]);                           \
            evenSum = _mm_add_ps (evenSum, x[6]);                           \
                                                                            \
            oddSum = _mm_setzero_ps();                                      \
            oddSum = _mm_add_ps (oddSum, x[1]);                             \
            oddSum = _mm_add_ps (oddSum, x[3]);                             \
            oddSum = _mm_add_ps (oddSum, x[5]);                             \
            oddSum = _mm_add_ps (oddSum, x[7]);                             \
                                                                            \
                                                                            \
            srcVec[2 * i]     = _mm_add_ps (evenSum, oddSum);               \
            srcVec[2 * i + 1] = _mm_sub_ps (evenSum, oddSum);               \
            srcVec[2 * i + 1] = _mm_shuffle_ps (srcVec[2 * i + 1],          \
                                                srcVec[2 * i + 1],          \
                                                _MM_SHUFFLE (0, 1, 2, 3));

        switch (zeroedRows)
        {
          case 0:
          default:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (3)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (4)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (5)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (6)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (7)
            break;

          case 1:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (3)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (4)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (5)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (6)
            break;

          case 2:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (3)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (4)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (5)
            break;

          case 3:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (3)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (4)
            break;

          case 4:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (3)
            break;

          case 5:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (2)
            break;

          case 6:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            DCT_INVERSE_8x8_SS2_ROW_LOOP (1)
            break;

          case 7:
            DCT_INVERSE_8x8_SS2_ROW_LOOP (0)
            break;
        }

        for (int col = 0; col < 2; ++col)
        {

            for (int i = 0; i < 8; ++i)
                in[i] = srcVec[2 * i + col];

            alpha[0] = _mm_mul_ps (c, in[2]);
            alpha[1] = _mm_mul_ps (f, in[2]);
            alpha[2] = _mm_mul_ps (c, in[6]);
            alpha[3] = _mm_mul_ps (f, in[6]);

            beta[0] = _mm_add_ps (_mm_add_ps (_mm_mul_ps (in[1], b),
                                                          _mm_mul_ps (in[3], d)),
                                              _mm_add_ps (_mm_mul_ps (in[5], e),
                                                          _mm_mul_ps (in[7], g)));

            beta[1] = _mm_sub_ps (_mm_sub_ps (_mm_mul_ps (in[1], d),
                                                          _mm_mul_ps (in[3], g)),
                                              _mm_add_ps (_mm_mul_ps (in[5], b),
                                                          _mm_mul_ps (in[7], e)));

            beta[2] = _mm_add_ps (_mm_sub_ps (_mm_mul_ps (in[1], e),
                                                          _mm_mul_ps (in[3], b)),
                                              _mm_add_ps (_mm_mul_ps (in[5], g),
                                                          _mm_mul_ps (in[7], d)));

            beta[3] = _mm_add_ps (_mm_sub_ps (_mm_mul_ps (in[1], g),
                                                          _mm_mul_ps (in[3], e)),
                                              _mm_sub_ps (_mm_mul_ps (in[5], d),
                                                          _mm_mul_ps (in[7], b)));

            theta[0] = _mm_mul_ps (a, _mm_add_ps (in[0], in[4]));
            theta[3] = _mm_mul_ps (a, _mm_sub_ps (in[0], in[4]));

            theta[1] = _mm_add_ps (alpha[0], alpha[3]);
            theta[2] = _mm_sub_ps (alpha[1], alpha[2]);

            gamma[0] = _mm_add_ps (theta[0], theta[1]);
            gamma[1] = _mm_add_ps (theta[3], theta[2]);
            gamma[2] = _mm_sub_ps (theta[3], theta[2]);
            gamma[3] = _mm_sub_ps (theta[0], theta[1]);

            srcVec[  col] = _mm_add_ps (gamma[0], beta[0]);
            srcVec[2+col] = _mm_add_ps (gamma[1], beta[1]);
            srcVec[4+col] = _mm_add_ps (gamma[2], beta[2]);
            srcVec[6+col] = _mm_add_ps (gamma[3], beta[3]);

            srcVec[ 8+col] = _mm_sub_ps (gamma[3], beta[3]);
            srcVec[10+col] = _mm_sub_ps (gamma[2], beta[2]);
            srcVec[12+col] = _mm_sub_ps (gamma[1], beta[1]);
            srcVec[14+col] = _mm_sub_ps (gamma[0], beta[0]);
        }

}

#define STR(A) #A

#define IDCT_AVX_SETUP_2_ROWS(_DST0,  _DST1,  _TMP0,  _TMP1, \
                              _OFF00, _OFF01, _OFF10, _OFF11) \
    "vmovaps                 " STR(_OFF00) "(%0),  %%xmm" STR(_TMP0) "  \n" \
    "vmovaps                 " STR(_OFF01) "(%0),  %%xmm" STR(_TMP1) "  \n" \
    "                                                                                \n" \
    "vinsertf128  $1, " STR(_OFF10) "(%0), %%ymm" STR(_TMP0) ", %%ymm" STR(_TMP0) "  \n" \
    "vinsertf128  $1, " STR(_OFF11) "(%0), %%ymm" STR(_TMP1) ", %%ymm" STR(_TMP1) "  \n" \
    "                                                                                \n" \
    "vunpcklpd      %%ymm" STR(_TMP1) ",  %%ymm" STR(_TMP0) ",  %%ymm" STR(_DST0) "  \n" \
    "vunpckhpd      %%ymm" STR(_TMP1) ",  %%ymm" STR(_TMP0) ",  %%ymm" STR(_DST1) "  \n" \
    "                                                                                \n" \
    "vunpcklps      %%ymm" STR(_DST1) ",  %%ymm" STR(_DST0) ",  %%ymm" STR(_TMP0) "  \n" \
    "vunpckhps      %%ymm" STR(_DST1) ",  %%ymm" STR(_DST0) ",  %%ymm" STR(_TMP1) "  \n" \
    "                                                                                \n" \
    "vunpcklpd      %%ymm" STR(_TMP1) ",  %%ymm" STR(_TMP0) ",  %%ymm" STR(_DST0) "  \n" \
    "vunpckhpd      %%ymm" STR(_TMP1) ",  %%ymm" STR(_TMP0) ",  %%ymm" STR(_DST1) "  \n" 

#define IDCT_AVX_MMULT_ROWS(_SRC)                       \
    /* Broadcast the source values into y12-y15 */      \
    "vpermilps $0x00, " STR(_SRC) ", %%ymm12       \n"  \
    "vpermilps $0x55, " STR(_SRC) ", %%ymm13       \n"  \
    "vpermilps $0xaa, " STR(_SRC) ", %%ymm14       \n"  \
    "vpermilps $0xff, " STR(_SRC) ", %%ymm15       \n"  \
                                                        \
    /* Multiple coefs and the broadcasted values */     \
    "vmulps    %%ymm12,  %%ymm8, %%ymm12     \n"        \
    "vmulps    %%ymm13,  %%ymm9, %%ymm13     \n"        \
    "vmulps    %%ymm14, %%ymm10, %%ymm14     \n"        \
    "vmulps    %%ymm15, %%ymm11, %%ymm15     \n"        \
                                                        \
    /* Accumulate the result back into the source */    \
    "vaddps    %%ymm13, %%ymm12, %%ymm12      \n"       \
    "vaddps    %%ymm15, %%ymm14, %%ymm14      \n"       \
    "vaddps    %%ymm14, %%ymm12, " STR(_SRC) "\n"     

#define IDCT_AVX_EO_TO_ROW_HALVES(_EVEN, _ODD, _FRONT, _BACK)      \
    "vsubps   " STR(_ODD) "," STR(_EVEN) "," STR(_BACK)  "\n"  \
    "vaddps   " STR(_ODD) "," STR(_EVEN) "," STR(_FRONT) "\n"  \
    /* Reverse the back half                                */ \
    "vpermilps $0x1b," STR(_BACK) "," STR(_BACK) "\n"  

#define IDCT_AVX_BODY \
    ROW0( IDCT_AVX_SETUP_2_ROWS(0, 4, 14, 15,    0,  16,  32,  48) ) \
    ROW2( IDCT_AVX_SETUP_2_ROWS(1, 5, 12, 13,   64,  80,  96, 112) ) \
    ROW4( IDCT_AVX_SETUP_2_ROWS(2, 6, 10, 11,  128, 144, 160, 176) ) \
    ROW6( IDCT_AVX_SETUP_2_ROWS(3, 7,  8,  9,  192, 208, 224, 240) ) \
                                                                     \
    "vbroadcastf128   (%1),  %%ymm8         \n"                      \
    "vbroadcastf128 16(%1),  %%ymm9         \n"                      \
    "vbroadcastf128 32(%1), %%ymm10         \n"                      \
    "vbroadcastf128 48(%1), %%ymm11         \n"                      \
                                                                     \
    ROW0( IDCT_AVX_MMULT_ROWS(%%ymm0) )                              \
    ROW2( IDCT_AVX_MMULT_ROWS(%%ymm1) )                              \
    ROW4( IDCT_AVX_MMULT_ROWS(%%ymm2) )                              \
    ROW6( IDCT_AVX_MMULT_ROWS(%%ymm3) )                              \
                                                                     \
    "vbroadcastf128  64(%1),  %%ymm8         \n"                     \
    "vbroadcastf128  80(%1),  %%ymm9         \n"                     \
    "vbroadcastf128  96(%1), %%ymm10         \n"                     \
    "vbroadcastf128 112(%1), %%ymm11         \n"                     \
                                                                     \
    ROW0( IDCT_AVX_MMULT_ROWS(%%ymm4) )                              \
    ROW2( IDCT_AVX_MMULT_ROWS(%%ymm5) )                              \
    ROW4( IDCT_AVX_MMULT_ROWS(%%ymm6) )                              \
    ROW6( IDCT_AVX_MMULT_ROWS(%%ymm7) )                              \
                                                                     \
    ROW0( IDCT_AVX_EO_TO_ROW_HALVES(%%ymm0, %%ymm4, %%ymm0, %%ymm12) ) \
    ROW2( IDCT_AVX_EO_TO_ROW_HALVES(%%ymm1, %%ymm5, %%ymm1, %%ymm13) ) \
    ROW4( IDCT_AVX_EO_TO_ROW_HALVES(%%ymm2, %%ymm6, %%ymm2, %%ymm14) ) \
    ROW6( IDCT_AVX_EO_TO_ROW_HALVES(%%ymm3, %%ymm7, %%ymm3, %%ymm15) ) \
                                                                       \
    ROW7( "vperm2f128 $0x13, %%ymm3, %%ymm15, %%ymm7   \n" )           \
    ROW6( "vperm2f128 $0x02, %%ymm3, %%ymm15, %%ymm6   \n" )           \
    ROW5( "vperm2f128 $0x13, %%ymm2, %%ymm14, %%ymm5   \n" )           \
    ROW4( "vperm2f128 $0x02, %%ymm2, %%ymm14, %%ymm4   \n" )           \
    ROW3( "vperm2f128 $0x13, %%ymm1, %%ymm13, %%ymm3   \n" )           \
    ROW2( "vperm2f128 $0x02, %%ymm1, %%ymm13, %%ymm2   \n" )           \
    ROW1( "vperm2f128 $0x13, %%ymm0, %%ymm12, %%ymm1   \n" )           \
    ROW0( "vperm2f128 $0x02, %%ymm0, %%ymm12, %%ymm0   \n" )           \
                                                                       \
                                                                       \
    "vpermilps        $0xff,  %%ymm8, %%ymm11  \n"                     \
    "vpermilps        $0xaa,  %%ymm8, %%ymm10  \n"                     \
    "vpermilps        $0x55,  %%ymm8, %%ymm9   \n"                     \
    "vpermilps        $0x00,  %%ymm8, %%ymm8   \n"                     \
                                                                       \
    ROW1( "vmulps    %%ymm1,  %%ymm8, %%ymm12    \n" )                 \
    ROW3( "vmulps    %%ymm3,  %%ymm9, %%ymm13    \n" )                 \
    ROW5( "vmulps    %%ymm5, %%ymm10, %%ymm14    \n" )                 \
    ROW7( "vmulps    %%ymm7, %%ymm11, %%ymm15    \n" )                 \
                                                                       \
    ROW3( "vaddps   %%ymm12, %%ymm13, %%ymm12    \n" )                 \
    ROW7( "vaddps   %%ymm14, %%ymm15, %%ymm14    \n" )                 \
    ROW5( "vaddps   %%ymm12, %%ymm14, %%ymm12    \n" )                 \
                                                                       \
    ROW1( "vmulps    %%ymm1,   %%ymm9, %%ymm13   \n" )                 \
    ROW3( "vmulps    %%ymm3,  %%ymm11, %%ymm14   \n" )                 \
    ROW5( "vmulps    %%ymm5,   %%ymm8, %%ymm15   \n" )                 \
                                                                       \
    ROW5( "vaddps    %%ymm14, %%ymm15, %%ymm14   \n" )                 \
    ROW3( "vsubps    %%ymm14, %%ymm13, %%ymm13   \n" )                 \
                                                                       \
    ROW7( "vmulps    %%ymm7,  %%ymm10, %%ymm15   \n" )                 \
    ROW7( "vsubps    %%ymm15, %%ymm13, %%ymm13   \n" )                 \
                                                                       \
    ROW1( "vmulps     %%ymm1, %%ymm10,  %%ymm14  \n" )                 \
    ROW3( "vmulps     %%ymm3,  %%ymm8,  %%ymm15  \n" )                 \
                                                                       \
    ROW3( "vsubps    %%ymm15, %%ymm14, %%ymm14   \n" )                 \
                                                                       \
    ROW5( "vmulps     %%ymm5, %%ymm11, %%ymm15   \n" )                 \
    ROW5( "vaddps    %%ymm15, %%ymm14, %%ymm14   \n" )                 \
                                                                       \
    ROW7( "vmulps    %%ymm7,   %%ymm9, %%ymm15   \n" )                 \
    ROW7( "vaddps    %%ymm15, %%ymm14, %%ymm14   \n" )                 \
                                                                       \
                                                                       \
    ROW1( "vmulps    %%ymm1, %%ymm11, %%ymm15    \n" )                 \
    ROW3( "vmulps    %%ymm3, %%ymm10,  %%ymm3    \n" )                 \
    ROW5( "vmulps    %%ymm5,  %%ymm9,  %%ymm5    \n" )                 \
    ROW7( "vmulps    %%ymm7,  %%ymm8,  %%ymm7    \n" )                 \
                                                                       \
    ROW5( "vaddps   %%ymm15,  %%ymm5, %%ymm15    \n" )                 \
    ROW7( "vaddps    %%ymm3,  %%ymm7,  %%ymm3    \n" )                 \
    ROW3( "vsubps    %%ymm3, %%ymm15, %%ymm15    \n" )                 \
                                                                       \
                                                                       \
    "vbroadcastf128   8(%1),  %%ymm1          \n"                      \
    "vpermilps        $0xff,  %%ymm1, %%ymm5  \n"                      \
    "vpermilps        $0xaa,  %%ymm1, %%ymm3  \n"                      \
    "vpermilps        $0x00,  %%ymm1, %%ymm1  \n"                      \
                                                                       \
    ROW0( "vmulps    %%ymm1,  %%ymm0, %%ymm11   \n" )                  \
    ROW4( "vmulps    %%ymm1,  %%ymm4,  %%ymm4   \n" )                  \
    ROW0( "vmovaps   %%ymm11, %%ymm1            \n" )                  \
    ROW4( "vaddps    %%ymm4, %%ymm11, %%ymm11   \n" )                  \
    ROW4( "vsubps    %%ymm4,  %%ymm1,  %%ymm1   \n" )                  \
                                                                       \
    ROW2( "vmulps    %%ymm3, %%ymm2,  %%ymm7    \n" )                  \
    ROW6( "vmulps    %%ymm5, %%ymm6,  %%ymm9    \n" )                  \
    ROW6( "vaddps    %%ymm9, %%ymm7,  %%ymm7    \n" )                  \
                                                                       \
    ROW0( "vmovaps   %%ymm11, %%ymm8            \n" )                  \
    ROW2( "vaddps     %%ymm7, %%ymm8,  %%ymm8   \n" )                  \
    ROW2( "vsubps     %%ymm7, %%ymm11, %%ymm11  \n" )                  \
                                                                       \
    ROW2( "vmulps     %%ymm5,  %%ymm2, %%ymm7   \n" )                  \
    ROW6( "vmulps     %%ymm3,  %%ymm6, %%ymm9   \n" )                  \
    ROW6( "vsubps     %%ymm9,  %%ymm7, %%ymm7   \n" )                  \
                                                                       \
    ROW0( "vmovaps   %%ymm1,  %%ymm9            \n" )                  \
    ROW0( "vmovaps   %%ymm1, %%ymm10            \n" )                  \
    ROW2( "vaddps    %%ymm7,  %%ymm1,  %%ymm9   \n" )                  \
    ROW2( "vsubps    %%ymm7,  %%ymm1,  %%ymm10  \n" )                  \
                                                                       \
    "vaddps   %%ymm12,  %%ymm8, %%ymm0       \n"                       \
    "vaddps   %%ymm13,  %%ymm9, %%ymm1       \n"                       \
    "vaddps   %%ymm14, %%ymm10, %%ymm2       \n"                       \
    "vaddps   %%ymm15, %%ymm11, %%ymm3       \n"                       \
                                                                       \
    "vsubps   %%ymm12,  %%ymm8, %%ymm7       \n"                       \
    "vsubps   %%ymm13,  %%ymm9, %%ymm6       \n"                       \
    "vsubps   %%ymm14, %%ymm10, %%ymm5       \n"                       \
    "vsubps   %%ymm15, %%ymm11, %%ymm4       \n"                       \
                                                                       \
    "vmovaps   %%ymm0,    (%0)                   \n"                   \
    "vmovaps   %%ymm1,  32(%0)                   \n"                   \
    "vmovaps   %%ymm2,  64(%0)                   \n"                   \
    "vmovaps   %%ymm3,  96(%0)                   \n"                   \
    "vmovaps   %%ymm4, 128(%0)                   \n"                   \
    "vmovaps   %%ymm5, 160(%0)                   \n"                   \
    "vmovaps   %%ymm6, 192(%0)                   \n"                   \
    "vmovaps   %%ymm7, 224(%0)                   \n"            

#define IDCT_AVX_OIC(_IN0)                         \
        : /* Output  */                            \
        : /* Input   */ "r"(_IN0), "r"(sAvxCoef)   \
        : /* Clobber */ "memory",                  \
                        "%xmm0",  "%xmm1",  "%xmm2",  "%xmm3", \
                        "%xmm4",  "%xmm5",  "%xmm6",  "%xmm7", \
                        "%xmm8",  "%xmm9",  "%xmm10", "%xmm11",\
                        "%xmm12", "%xmm13", "%xmm14", "%xmm15" 

#ifndef __AVX__ 
    #define IDCT_AVX_ASM(_IN0)   \
        __asm__(                 \
            IDCT_AVX_BODY        \
            "vzeroupper      \n" \
            IDCT_AVX_OIC(_IN0)   \
        );                       
#else /* __AVX__ */
    #define IDCT_AVX_ASM(_IN0)   \
        __asm__(                 \
            IDCT_AVX_BODY        \
            IDCT_AVX_OIC(_IN0)   \
        );                       
#endif /* __AVX__ */

template <int zeroedRows>
void
dctInverse8x8_avx (float *data)
{

        dctInverse8x8_scalar<zeroedRows>(data);
}

void 
dctForward8x8 (float *data)
{
    __m128 *srcVec = (__m128 *)data;
    __m128  a0Vec, a1Vec, a2Vec, a3Vec, a4Vec, a5Vec, a6Vec, a7Vec;
    __m128  k0Vec, k1Vec, rotXVec, rotYVec;
    __m128  transTmp[4], transTmp2[4];

    __m128  c4Vec     = { .70710678f,  .70710678f,  .70710678f,  .70710678f};
    __m128  c4NegVec  = {-.70710678f, -.70710678f, -.70710678f, -.70710678f};

    __m128  c1HalfVec = {.490392640f, .490392640f, .490392640f, .490392640f}; 
    __m128  c2HalfVec = {.461939770f, .461939770f, .461939770f, .461939770f};
    __m128  c3HalfVec = {.415734810f, .415734810f, .415734810f, .415734810f}; 
    __m128  c5HalfVec = {.277785120f, .277785120f, .277785120f, .277785120f}; 
    __m128  c6HalfVec = {.191341720f, .191341720f, .191341720f, .191341720f};
    __m128  c7HalfVec = {.097545161f, .097545161f, .097545161f, .097545161f}; 

    __m128  halfVec   = {.5f, .5f, .5f, .5f};

    for (int iter = 0; iter < 2; ++iter)
    {
        for (int pass=0; pass<2; ++pass)
        {
            a0Vec = _mm_add_ps (srcVec[ 0 + pass], srcVec[14 + pass]);
            a1Vec = _mm_add_ps (srcVec[ 2 + pass], srcVec[ 4 + pass]);
            a3Vec = _mm_add_ps (srcVec[ 6 + pass], srcVec[ 8 + pass]);
            a5Vec = _mm_add_ps (srcVec[10 + pass], srcVec[12 + pass]);
 
            a7Vec = _mm_sub_ps (srcVec[ 0 + pass], srcVec[14 + pass]);
            a2Vec = _mm_sub_ps (srcVec[ 2 + pass], srcVec[ 4 + pass]);
            a4Vec = _mm_sub_ps (srcVec[ 6 + pass], srcVec[ 8 + pass]);
            a6Vec = _mm_sub_ps (srcVec[10 + pass], srcVec[12 + pass]);

            k0Vec = _mm_add_ps (a0Vec, a3Vec);
            k1Vec = _mm_add_ps (a1Vec, a5Vec);

            k0Vec = _mm_mul_ps (c4Vec, k0Vec);
            k1Vec = _mm_mul_ps (c4Vec, k1Vec);

            srcVec[0 + pass] = _mm_add_ps (k0Vec, k1Vec);
            srcVec[8 + pass] = _mm_sub_ps (k0Vec, k1Vec);

            srcVec[0 + pass] = _mm_mul_ps (srcVec[0 + pass], halfVec );
            srcVec[8 + pass] = _mm_mul_ps (srcVec[8 + pass], halfVec );

            k0Vec = _mm_sub_ps (a2Vec, a6Vec);
            k1Vec = _mm_sub_ps (a0Vec, a3Vec);

            srcVec[ 4 + pass] = _mm_add_ps (_mm_mul_ps (c6HalfVec, k0Vec),
                                            _mm_mul_ps (c2HalfVec, k1Vec));

            srcVec[12 + pass] = _mm_sub_ps (_mm_mul_ps (c6HalfVec, k1Vec), 
                                            _mm_mul_ps (c2HalfVec, k0Vec));

            k0Vec = _mm_mul_ps (_mm_sub_ps (a1Vec, a5Vec), c4Vec);
            k1Vec = _mm_mul_ps (_mm_add_ps (a2Vec, a6Vec), c4NegVec); 

            rotXVec = _mm_sub_ps (a7Vec, k0Vec);
            rotYVec = _mm_add_ps (a4Vec, k1Vec);

            srcVec[ 6 + pass] = _mm_sub_ps (_mm_mul_ps (c3HalfVec, rotXVec),
                                            _mm_mul_ps (c5HalfVec, rotYVec));

            srcVec[10 + pass] = _mm_add_ps (_mm_mul_ps (c5HalfVec, rotXVec),
                                            _mm_mul_ps (c3HalfVec, rotYVec));

            rotXVec = _mm_add_ps (a7Vec, k0Vec);
            rotYVec = _mm_sub_ps (k1Vec, a4Vec);

            srcVec[ 2 + pass] = _mm_sub_ps (_mm_mul_ps (c1HalfVec, rotXVec),
                                            _mm_mul_ps (c7HalfVec, rotYVec));

            srcVec[14 + pass] = _mm_add_ps (_mm_mul_ps (c7HalfVec, rotXVec), 
                                            _mm_mul_ps (c1HalfVec, rotYVec));
        }

        transTmp[0] = _mm_shuffle_ps (srcVec[0], srcVec[2], 0x44);
        transTmp[1] = _mm_shuffle_ps (srcVec[4], srcVec[6], 0x44);
        transTmp[3] = _mm_shuffle_ps (srcVec[4], srcVec[6], 0xEE);
        transTmp[2] = _mm_shuffle_ps (srcVec[0], srcVec[2], 0xEE);

        transTmp2[0] = _mm_shuffle_ps (srcVec[ 9], srcVec[11], 0x44);
        transTmp2[1] = _mm_shuffle_ps (srcVec[13], srcVec[15], 0x44);
        transTmp2[2] = _mm_shuffle_ps (srcVec[ 9], srcVec[11], 0xEE);
        transTmp2[3] = _mm_shuffle_ps (srcVec[13], srcVec[15], 0xEE);

        srcVec[0] = _mm_shuffle_ps (transTmp[0], transTmp[1], 0x88);
        srcVec[4] = _mm_shuffle_ps (transTmp[2], transTmp[3], 0x88);
        srcVec[2] = _mm_shuffle_ps (transTmp[0], transTmp[1], 0xDD);
        srcVec[6] = _mm_shuffle_ps (transTmp[2], transTmp[3], 0xDD);

        srcVec[ 9] = _mm_shuffle_ps (transTmp2[0], transTmp2[1], 0x88);
        srcVec[13] = _mm_shuffle_ps (transTmp2[2], transTmp2[3], 0x88);
        srcVec[11] = _mm_shuffle_ps (transTmp2[0], transTmp2[1], 0xDD);
        srcVec[15] = _mm_shuffle_ps (transTmp2[2], transTmp2[3], 0xDD);

        transTmp[0] = _mm_shuffle_ps (srcVec[1], srcVec[3], 0x44);
        transTmp[1] = _mm_shuffle_ps (srcVec[5], srcVec[7], 0x44);
        transTmp[2] = _mm_shuffle_ps (srcVec[1], srcVec[3], 0xEE);
        transTmp[3] = _mm_shuffle_ps (srcVec[5], srcVec[7], 0xEE);

        transTmp2[0] = _mm_shuffle_ps (srcVec[ 8], srcVec[10], 0x44);
        transTmp2[1] = _mm_shuffle_ps (srcVec[12], srcVec[14], 0x44);
        transTmp2[2] = _mm_shuffle_ps (srcVec[ 8], srcVec[10], 0xEE);
        transTmp2[3] = _mm_shuffle_ps (srcVec[12], srcVec[14], 0xEE);

        srcVec[ 8] = _mm_shuffle_ps (transTmp[0], transTmp[1], 0x88);
        srcVec[12] = _mm_shuffle_ps (transTmp[2], transTmp[3], 0x88);
        srcVec[10] = _mm_shuffle_ps (transTmp[0], transTmp[1], 0xDD);
        srcVec[14] = _mm_shuffle_ps (transTmp[2], transTmp[3], 0xDD);

        srcVec[1] = _mm_shuffle_ps (transTmp2[0], transTmp2[1], 0x88);
        srcVec[5] = _mm_shuffle_ps (transTmp2[2], transTmp2[3], 0x88);
        srcVec[3] = _mm_shuffle_ps (transTmp2[0], transTmp2[1], 0xDD);
        srcVec[7] = _mm_shuffle_ps (transTmp2[2], transTmp2[3], 0xDD);
    }
}

}

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
