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

#include "ImfDwaCompressor.h"
#include "ImfDwaCompressorSimd.h"

#include "ImfChannelList.h"
#include "ImfStandardAttributes.h"
#include "ImfHeader.h"
#include "ImfHuf.h"
#include "ImfInt64.h"
#include "ImfIntAttribute.h"
#include "ImfIO.h"
#include "ImfMisc.h"
#include "ImfNamespace.h"
#include "ImfRle.h"
#include "ImfSimd.h"
#include "ImfSystemSpecific.h"
#include "ImfXdr.h"
#include "ImfZip.h"

#include "ImathFun.h"
#include "ImathBox.h"
#include "ImathVec.h"
#include "half.h"

#include "dwaLookups.h"

#include <vector>
#include <string>
#include <cctype>
#include <cassert>
#include <algorithm>

#if defined _WIN32 || defined _WIN64
	#ifdef NOMINMAX
		#undef NOMINMAX
	#endif
	#define NOMINMAX
#endif
#include <zlib.h>

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER

namespace {
    void (*convertFloatToHalf64)(unsigned short*, float*) =
        convertFloatToHalf64_scalar;
    
    void (*fromHalfZigZag)(unsigned short*, float*) =
        fromHalfZigZag_scalar;

    void (*dctInverse8x8_0)(float*) = dctInverse8x8_scalar<0>;
    void (*dctInverse8x8_1)(float*) = dctInverse8x8_scalar<1>;
    void (*dctInverse8x8_2)(float*) = dctInverse8x8_scalar<2>;
    void (*dctInverse8x8_3)(float*) = dctInverse8x8_scalar<3>;
    void (*dctInverse8x8_4)(float*) = dctInverse8x8_scalar<4>;
    void (*dctInverse8x8_5)(float*) = dctInverse8x8_scalar<5>;
    void (*dctInverse8x8_6)(float*) = dctInverse8x8_scalar<6>;
    void (*dctInverse8x8_7)(float*) = dctInverse8x8_scalar<7>;
}

struct DwaCompressor::ChannelData
{
    std::string         name;
    CompressorScheme    compression;  
    int                 xSampling;
    int                 ySampling;
    PixelType           type;
    bool                pLinear;

    int                 width;
    int                 height;

    char               *planarUncBuffer;
    char               *planarUncBufferEnd;

    char               *planarUncRle[4];
    char               *planarUncRleEnd[4];

    PixelType           planarUncType;
    int                 planarUncSize;
};

struct DwaCompressor::CscChannelSet
{
    int idx[3];
};

struct DwaCompressor::Classifier
{
    Classifier (std::string suffix,
                CompressorScheme scheme,
                PixelType type,
                int cscIdx,
                bool caseInsensitive):
        _suffix(suffix),
        _scheme(scheme),
        _type(type),
        _cscIdx(cscIdx),
        _caseInsensitive(caseInsensitive)
    {
        if (caseInsensitive) 
            transform(_suffix.begin(), _suffix.end(), _suffix.begin(), tolower);
    }

    Classifier (const char *&ptr, int size)
    {
        if (size <= 0) 
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (truncated rule).");
            
        {
            char suffix[Name::SIZE+1];
            memset (suffix, 0, Name::SIZE+1);
            Xdr::read<CharPtrIO> (ptr, std::min(size, Name::SIZE-1), suffix);
            _suffix = std::string(suffix);
        }

        if (size < _suffix.length() + 1 + 2*Xdr::size<char>()) 
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (truncated rule).");

        char value;
        Xdr::read<CharPtrIO> (ptr, value);

        _cscIdx = (int)(value >> 4) - 1;
        if (_cscIdx < -1 || _cscIdx >= 3) 
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (corrupt cscIdx rule).");

        _scheme = (CompressorScheme)((value >> 2) & 3);
        if (_scheme < 0 || _scheme >= NUM_COMPRESSOR_SCHEMES) 
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (corrupt scheme rule).");

        _caseInsensitive = (value & 1 ? true : false);

        Xdr::read<CharPtrIO> (ptr, value);
        if (value < 0 || value >= NUM_PIXELTYPES) 
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (corrupt rule).");
        _type = (PixelType)value;
    }

    bool match (const std::string &suffix, const PixelType type) const
    {
        if (_type != type) return false;

        if (_caseInsensitive) 
        {
            std::string tmp(suffix);
            transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
            return tmp == _suffix;
        }

        return suffix == _suffix;
    }

    size_t size () const 
    {
        size_t sizeBytes = _suffix.length() + 1;

        sizeBytes += 2 * Xdr::size<char>();

        return sizeBytes;
    }

    void write (char *&ptr) const
    {
        Xdr::write<CharPtrIO> (ptr, _suffix.c_str());

        unsigned char value = 0;
        value |= ((unsigned char)(_cscIdx+1)      & 15) << 4;
        value |= ((unsigned char)_scheme          &  3) << 2;
        value |=  (unsigned char)_caseInsensitive &  1;

        Xdr::write<CharPtrIO> (ptr, value);
        Xdr::write<CharPtrIO> (ptr, (unsigned char)_type);
    }

    std::string      _suffix;
    CompressorScheme _scheme;
    PixelType        _type;
    int              _cscIdx;
    bool             _caseInsensitive;
};

class DwaCompressor::LossyDctDecoderBase
{
  public:

    LossyDctDecoderBase
        (char *packedAc,
         char *packedDc,
         const unsigned short *toLinear,
         int width,
         int height);

    virtual ~LossyDctDecoderBase ();

    void execute();

    int numAcValuesEncoded() const { return _packedAcCount; }
    int numDcValuesEncoded() const { return _packedDcCount; }

  protected:

    int unRleAc (unsigned short *&currAcComp,
                 unsigned short  *halfZigBlock); 

    bool                  _isNativeXdr;

    int                   _packedAcCount;
    int                   _packedDcCount;

    char                 *_packedAc;
    char                 *_packedDc;

    const unsigned short *_toLinear;

    int                   _width;
    int                   _height;

    std::vector< std::vector<char *> > _rowPtrs;

    std::vector<PixelType>             _type;
    std::vector<SimdAlignedBuffer64f>  _dctData;
};

class DwaCompressor::LossyDctDecoder: public LossyDctDecoderBase
{
  public:

    LossyDctDecoder
        (std::vector<char *> &rowPtrs,
         char *packedAc,
         char *packedDc,
         const unsigned short *toLinear,
         int width,
         int height,
         PixelType type)
    :
        LossyDctDecoderBase(packedAc, packedDc, toLinear, width, height)
    {
        _rowPtrs.push_back(rowPtrs);
        _type.push_back(type);
    }

    virtual ~LossyDctDecoder () {}
};

class DwaCompressor::LossyDctDecoderCsc: public LossyDctDecoderBase
{
  public:

    LossyDctDecoderCsc
        (std::vector<char *> &rowPtrsR,
         std::vector<char *> &rowPtrsG,
         std::vector<char *> &rowPtrsB,
         char *packedAc,
         char *packedDc,
         const unsigned short *toLinear,
         int width,
         int height,
         PixelType typeR,
         PixelType typeG,
         PixelType typeB)
    :
        LossyDctDecoderBase(packedAc, packedDc, toLinear, width, height)
    {
        _rowPtrs.push_back(rowPtrsR);
        _rowPtrs.push_back(rowPtrsG);
        _rowPtrs.push_back(rowPtrsB);
        _type.push_back(typeR);
        _type.push_back(typeG);
        _type.push_back(typeB);
    }

    virtual ~LossyDctDecoderCsc () {}
};

class DwaCompressor::LossyDctEncoderBase
{
  public:

    LossyDctEncoderBase
        (float quantBaseError,
         char *packedAc,
         char *packedDc,
         const unsigned short *toNonlinear,
         int width,
         int height);

    virtual ~LossyDctEncoderBase ();

    void execute ();

    int     numAcValuesEncoded () const {return _numAcComp;}
    int     numDcValuesEncoded () const {return _numDcComp;}

  protected:

    void    toZigZag (half *dst, half *src);
    int     countSetBits (unsigned short src);
    half    quantize (half src, float errorTolerance);
    void    rleAc (half *block, unsigned short *&acPtr);

    float                      _quantBaseError;

    int                        _width,
                               _height;
    const unsigned short      *_toNonlinear;

    int                        _numAcComp,
                               _numDcComp;

    std::vector< std::vector<const char *> > _rowPtrs;
    std::vector<PixelType>                   _type;
    std::vector<SimdAlignedBuffer64f>        _dctData;

    char                      *_packedAc;
    char                      *_packedDc;

    float                      _quantTableY[64];
    float                      _quantTableCbCr[64];
};

class DwaCompressor::LossyDctEncoder: public LossyDctEncoderBase
{
  public:

    LossyDctEncoder
        (float quantBaseError,
         std::vector<const char *> &rowPtrs,
         char *packedAc,
         char *packedDc,
         const unsigned short *toNonlinear,
         int width,
         int height,
         PixelType type)
    :
        LossyDctEncoderBase
            (quantBaseError, packedAc, packedDc, toNonlinear, width, height)
    {
        _rowPtrs.push_back(rowPtrs);
        _type.push_back(type);
    }

    virtual ~LossyDctEncoder () {}
};

class DwaCompressor::LossyDctEncoderCsc: public LossyDctEncoderBase
{
  public:

    LossyDctEncoderCsc
        (float quantBaseError,
         std::vector<const char *> &rowPtrsR,
         std::vector<const char *> &rowPtrsG,
         std::vector<const char *> &rowPtrsB,
         char *packedAc,
         char *packedDc,
         const unsigned short *toNonlinear,
         int width,
         int height,
         PixelType typeR,
         PixelType typeG,
         PixelType typeB)
    :
        LossyDctEncoderBase
            (quantBaseError, packedAc, packedDc, toNonlinear, width, height)
    {
        _type.push_back(typeR);
        _type.push_back(typeG);
        _type.push_back(typeB);

        _rowPtrs.push_back(rowPtrsR);
        _rowPtrs.push_back(rowPtrsG);
        _rowPtrs.push_back(rowPtrsB);
    }

    virtual ~LossyDctEncoderCsc () {}
};

DwaCompressor::LossyDctDecoderBase::LossyDctDecoderBase
    (char *packedAc,
     char *packedDc,
     const unsigned short *toLinear,
     int width,
     int height)
:
    _isNativeXdr(false),
    _packedAcCount(0),
    _packedDcCount(0),
    _packedAc(packedAc),
    _packedDc(packedDc),
    _toLinear(toLinear),
    _width(width),
    _height(height)
{
    if (_toLinear == 0)
        _toLinear = dwaCompressorNoOp;

    _isNativeXdr = GLOBAL_SYSTEM_LITTLE_ENDIAN;
}

DwaCompressor::LossyDctDecoderBase::~LossyDctDecoderBase () {}

void
DwaCompressor::LossyDctDecoderBase::execute ()
{
    int numComp        = _rowPtrs.size();
    int lastNonZero    = 0;
    int numBlocksX     = (int) ceil ((float)_width  / 8.0f);
    int numBlocksY     = (int) ceil ((float)_height / 8.0f);
    int leftoverX      = _width  - (numBlocksX-1) * 8;
    int leftoverY      = _height - (numBlocksY-1) * 8;

    int numFullBlocksX = (int)floor ((float)_width / 8.0f);

    unsigned short tmpShortNative = 0;
    unsigned short tmpShortXdr    = 0;
    const char *tmpConstCharPtr   = 0;

    unsigned short                    *currAcComp = (unsigned short *)_packedAc;
    std::vector<unsigned short *>      currDcComp (_rowPtrs.size());
    std::vector<SimdAlignedBuffer64us> halfZigBlock (_rowPtrs.size());

    if (_type.size() != _rowPtrs.size())
        throw Iex::BaseExc ("Row pointers and types mismatch in count");

    if ((_rowPtrs.size() != 3) && (_rowPtrs.size() != 1))
        throw Iex::NoImplExc ("Only 1 and 3 channel encoding is supported");

    _dctData.resize(numComp);

    unsigned char *rowBlockHandle = new unsigned char
        [numComp * numBlocksX * 64 * sizeof(unsigned short) + _SSE_ALIGNMENT];

    unsigned short *rowBlock[3];

    rowBlock[0] = (unsigned short*)rowBlockHandle;

    for (int i = 0; i < _SSE_ALIGNMENT; ++i)
    {
        if (((size_t)(rowBlockHandle + i) & _SSE_ALIGNMENT_MASK) == 0)
            rowBlock[0] = (unsigned short *)(rowBlockHandle + i);
    }

    for (int comp = 1; comp < numComp; ++comp)
        rowBlock[comp] = rowBlock[comp - 1] + numBlocksX * 64;
 
    currDcComp[0] = (unsigned short *)_packedDc;

    for (unsigned int comp = 1; comp < numComp; ++comp)
        currDcComp[comp] = currDcComp[comp - 1] + numBlocksX * numBlocksY;

    for (int blocky = 0; blocky < numBlocksY; ++blocky)
    {
        int maxY = 8;

        if (blocky == numBlocksY-1)
            maxY = leftoverY;

        int maxX = 8;

        for (int blockx = 0; blockx < numBlocksX; ++blockx)
        {
            if (blockx == numBlocksX-1)
                maxX = leftoverX;

            bool blockIsConstant = true;

            for (unsigned int comp = 0; comp < numComp; ++comp)
            {
                    {
                        __m128i *dst = (__m128i*)halfZigBlock[comp]._buffer;

                        dst[7] = _mm_setzero_si128();
                        dst[6] = _mm_setzero_si128();
                        dst[5] = _mm_setzero_si128();
                        dst[4] = _mm_setzero_si128();
                        dst[3] = _mm_setzero_si128();
                        dst[2] = _mm_setzero_si128();
                        dst[1] = _mm_setzero_si128();
                        dst[0] = _mm_insert_epi16
                            (_mm_setzero_si128(), *currDcComp[comp]++, 0);
                    }

                _packedDcCount++;
                
                //
                // UnRLE the AC. This will modify currAcComp
                //

                lastNonZero = unRleAc (currAcComp, halfZigBlock[comp]._buffer);

                //
                // Convert from XDR to NATIVE
                //

                if (!_isNativeXdr)
                {
                    for (int i = 0; i < 64; ++i)
                    {
                        tmpShortXdr      = halfZigBlock[comp]._buffer[i];
                        tmpConstCharPtr  = (const char *)&tmpShortXdr;

                        Xdr::read<CharPtrIO> (tmpConstCharPtr, tmpShortNative);

                        halfZigBlock[comp]._buffer[i] = tmpShortNative;
                    }
                }

                if (lastNonZero == 0)
                {
                    //
                    // DC only case - AC components are all 0   
                    //

                    half h;

                    h.setBits (halfZigBlock[comp]._buffer[0]);
                    _dctData[comp]._buffer[0] = (float)h;

                    dctInverse8x8DcOnly (_dctData[comp]._buffer);
                }
                else
                {
                    //
                    // We have some AC components that are non-zero. 
                    // Can't use the 'constant block' optimization
                    //

                    blockIsConstant = false;

                    //
                    // Un-Zig zag 
                    //

                    (*fromHalfZigZag)
                        (halfZigBlock[comp]._buffer, _dctData[comp]._buffer);

                    //
                    // Zig-Zag indices in normal layout are as follows:
                    //
                    // 0   1   3   6   10  15  21  28
                    // 2   4   7   11  16  22  29  36
                    // 5   8   12  17  23  30  37  43
                    // 9   13  18  24  31  38  44  49
                    // 14  19  25  32  39  45  50  54
                    // 20  26  33  40  46  51  55  58
                    // 27  34  41  47  52  56  59  61
                    // 35  42  48  53  57  60  62  63
                    //
                    // If lastNonZero is less than the first item on
                    // each row, we know that the whole row is zero and 
                    // can be skipped in the row-oriented part of the
                    // iDCT. 
                    //
                    // The unrolled logic here is:
                    //
                    //    if lastNonZero < rowStartIdx[i],
                    //    zeroedRows = rowsEmpty[i]
                    //
                    // where:
                    //
                    //    const int rowStartIdx[] = {2, 5, 9, 14, 20, 27, 35};
                    //    const int rowsEmpty[]   = {7, 6, 5,  4,  3,  2,  1};
                    //

                    if (lastNonZero < 2)
                        dctInverse8x8_7(_dctData[comp]._buffer);
                    else if (lastNonZero < 5)
                        dctInverse8x8_6(_dctData[comp]._buffer);
                    else if (lastNonZero < 9)
                        dctInverse8x8_5(_dctData[comp]._buffer);
                    else if (lastNonZero < 14)
                        dctInverse8x8_4(_dctData[comp]._buffer);
                    else if (lastNonZero < 20)
                        dctInverse8x8_3(_dctData[comp]._buffer);
                    else if (lastNonZero < 27)
                        dctInverse8x8_2(_dctData[comp]._buffer);
                    else if (lastNonZero < 35)
                        dctInverse8x8_1(_dctData[comp]._buffer);
                    else
                        dctInverse8x8_0(_dctData[comp]._buffer);
                }
            }

            //
            // Perform the CSC
            //

            if (numComp == 3)
            {
                if (!blockIsConstant)
                {
                    csc709Inverse64 (_dctData[0]._buffer, 
                                     _dctData[1]._buffer, 
                                     _dctData[2]._buffer);

                }
                else
                {
                    csc709Inverse (_dctData[0]._buffer[0], 
                                   _dctData[1]._buffer[0], 
                                   _dctData[2]._buffer[0]);
                }
            }

            //
            // Float -> Half conversion. 
            //
            // If the block has a constant value, just convert the first pixel.
            //

            for (unsigned int comp = 0; comp < numComp; ++comp)
            {
                if (!blockIsConstant)
                {
                    (*convertFloatToHalf64)
                        (&rowBlock[comp][blockx*64], _dctData[comp]._buffer);
                }
                else
                {
                        __m128i *dst = (__m128i*)&rowBlock[comp][blockx*64];

                        dst[0] = _mm_set1_epi16
                            (((half)_dctData[comp]._buffer[0]).bits());

                        dst[1] = dst[0];
                        dst[2] = dst[0];
                        dst[3] = dst[0];
                        dst[4] = dst[0];
                        dst[5] = dst[0];
                        dst[6] = dst[0];
                        dst[7] = dst[0];
                }
            }
        }

        for (int comp = 0; comp < numComp; ++comp)
        {
            bool fastPath = true;

            for (int y = 8 * blocky; y < 8 * blocky + maxY; ++y)
            {
                if ((size_t)_rowPtrs[comp][y] & _SSE_ALIGNMENT_MASK)
                    fastPath = false;
            }

            if (fastPath)
            {
                for (int y=8*blocky; y<8*blocky+maxY; ++y)
                {
                    __m128i *dst = (__m128i *)_rowPtrs[comp][y];
                    __m128i *src = (__m128i *)&rowBlock[comp][(y & 0x7) * 8];


                    for (int blockx = 0; blockx < numFullBlocksX; ++blockx)
                    {
                        _mm_prefetch ((char *)(src + 16), _MM_HINT_NTA); 

                        unsigned short i0  = _mm_extract_epi16 (*src, 0);
                        unsigned short i1  = _mm_extract_epi16 (*src, 1);
                        unsigned short i2  = _mm_extract_epi16 (*src, 2);
                        unsigned short i3  = _mm_extract_epi16 (*src, 3);

                        unsigned short i4  = _mm_extract_epi16 (*src, 4);
                        unsigned short i5  = _mm_extract_epi16 (*src, 5);
                        unsigned short i6  = _mm_extract_epi16 (*src, 6);
                        unsigned short i7  = _mm_extract_epi16 (*src, 7);

                        i0 = _toLinear[i0];
                        i1 = _toLinear[i1];
                        i2 = _toLinear[i2];
                        i3 = _toLinear[i3];

                        i4 = _toLinear[i4];
                        i5 = _toLinear[i5];
                        i6 = _toLinear[i6];
                        i7 = _toLinear[i7];

                        *dst = _mm_insert_epi16 (_mm_setzero_si128(), i0, 0);
                        *dst = _mm_insert_epi16 (*dst, i1, 1);
                        *dst = _mm_insert_epi16 (*dst, i2, 2);
                        *dst = _mm_insert_epi16 (*dst, i3, 3);

                        *dst = _mm_insert_epi16 (*dst, i4, 4);
                        *dst = _mm_insert_epi16 (*dst, i5, 5);
                        *dst = _mm_insert_epi16 (*dst, i6, 6);
                        *dst = _mm_insert_epi16 (*dst, i7, 7);

                        src += 8;
                        dst++;
                    }
                }
            }
            else
            {
                for (int y = 8 * blocky; y < 8 * blocky + maxY; ++y)
                {
                    unsigned short *dst = (unsigned short *)_rowPtrs[comp][y];

                    for (int blockx = 0; blockx < numFullBlocksX; ++blockx)
                    {
                        unsigned short *src =
                            &rowBlock[comp][blockx * 64 + ((y & 0x7) * 8)];

                        dst[0] = _toLinear[src[0]];
                        dst[1] = _toLinear[src[1]];
                        dst[2] = _toLinear[src[2]];
                        dst[3] = _toLinear[src[3]];

                        dst[4] = _toLinear[src[4]];
                        dst[5] = _toLinear[src[5]];
                        dst[6] = _toLinear[src[6]];
                        dst[7] = _toLinear[src[7]];

                        dst += 8;
                    }
                }
            }

            if (numFullBlocksX != numBlocksX)
            {
                for (int y = 8 * blocky; y < 8 * blocky + maxY; ++y)
                {
                    unsigned short *src = (unsigned short *)
                        &rowBlock[comp][numFullBlocksX * 64 + ((y & 0x7) * 8)];

                    unsigned short *dst = (unsigned short *)_rowPtrs[comp][y];

                    dst += 8 * numFullBlocksX;

                    for (int x = 0; x < maxX; ++x)
                    {
                        *dst++ = _toLinear[*src++];
                    }
                }
            }
        }
    }

    for (unsigned int chan = 0; chan < numComp; ++chan)
    {

        if (_type[chan] != FLOAT)
            continue;

        std::vector<unsigned short> halfXdr (_width);

        for (int y=0; y<_height; ++y)
        {
            char *floatXdrPtr = _rowPtrs[chan][y];

            memcpy(&halfXdr[0], floatXdrPtr, _width*sizeof(unsigned short));

            const char *halfXdrPtr = (const char *)(&halfXdr[0]);

            for (int x=0; x<_width; ++x)
            {
                half tmpHalf;

                Xdr::read<CharPtrIO> (halfXdrPtr, tmpHalf);
                Xdr::write<CharPtrIO> (floatXdrPtr, (float)tmpHalf);
            }
        }
    }

    delete[] rowBlockHandle;
}

int 
DwaCompressor::LossyDctDecoderBase::unRleAc
    (unsigned short *&currAcComp,
     unsigned short  *halfZigBlock) 
{
    int lastNonZero          = 0;
    int dctComp              = 1; 

    while (dctComp < 64)
    {
        if (*currAcComp == 0xff00)
        {
            dctComp = 64;

        }
        else if ((*currAcComp) >> 8 == 0xff)
        {
            dctComp += (*currAcComp) & 0xff; 
        }
        else
        {
            lastNonZero = dctComp;
            halfZigBlock[dctComp] = *currAcComp;

            dctComp++;
        }

        _packedAcCount++;
        currAcComp++;
    }

    return lastNonZero;
}

DwaCompressor::LossyDctEncoderBase::LossyDctEncoderBase
    (float quantBaseError,
     char *packedAc,
     char *packedDc,
     const unsigned short *toNonlinear,
     int width,
     int height)
:
    _quantBaseError(quantBaseError),
    _width(width),
    _height(height),
    _toNonlinear(toNonlinear),
    _numAcComp(0),
    _numDcComp(0),
    _packedAc(packedAc),
    _packedDc(packedDc)
{
    int jpegQuantTableY[] =
    {
        16,  11,  10,  16,   24,   40,   51,   61,
        12,  12,  14,  19,   26,   58,   60,   55,
        14,  13,  16,  24,   40,   57,   69,   56,
        14,  17,  22,  29,   51,   87,   80,   62,
        18,  22,  37,  56,   68,  109,  103,   77,
        24,  35,  55,  64,   81,  104,  113,   92,
        49,  64,  78,  87,  103,  121,  120,  101,
        72,  92,  95,  98,  112,  100,  103,   99
    };

    int jpegQuantTableYMin = 10;

    int jpegQuantTableCbCr[] =
    {
        17,  18,  24,  47,  99,  99,  99,  99,
        18,  21,  26,  66,  99,  99,  99,  99,
        24,  26,  56,  99,  99,  99,  99,  99,
        47,  66,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99
    };

    int jpegQuantTableCbCrMin = 17;

    for (int idx = 0; idx < 64; ++idx)
    {
        _quantTableY[idx] = static_cast<float> (jpegQuantTableY[idx]) /
                            static_cast<float> (jpegQuantTableYMin);

        _quantTableCbCr[idx] = static_cast<float> (jpegQuantTableCbCr[idx]) /
                               static_cast<float> (jpegQuantTableCbCrMin);
    }
    
    if (_quantBaseError < 0)
        quantBaseError = 0;
}

DwaCompressor::LossyDctEncoderBase::~LossyDctEncoderBase () 
{
}

void
DwaCompressor::LossyDctEncoderBase::execute ()
{
    int  numBlocksX   = (int)ceil ((float)_width / 8.0f);
    int  numBlocksY   = (int)ceil ((float)_height/ 8.0f);

    half halfZigCoef[64]; 
    half halfCoef[64];

    std::vector<unsigned short *> currDcComp (_rowPtrs.size());
    unsigned short               *currAcComp = (unsigned short *)_packedAc;

    _dctData.resize (_rowPtrs.size());
    _numAcComp = 0;
    _numDcComp = 0;
 
    assert (_type.size() == _rowPtrs.size());
    assert ((_rowPtrs.size() == 3) || (_rowPtrs.size() == 1));

    int tmpHalfBufferElements = 0;

    for (unsigned int chan = 0; chan < _rowPtrs.size(); ++chan)
        if (_type[chan] == FLOAT)
            tmpHalfBufferElements += _width * _height;

    std::vector<unsigned short> tmpHalfBuffer (tmpHalfBufferElements);

    char *tmpHalfBufferPtr = 0;

    if (tmpHalfBufferElements)
        tmpHalfBufferPtr = (char *)&tmpHalfBuffer[0];

    for (unsigned int chan = 0; chan < _rowPtrs.size(); ++chan)
    {
        if (_type[chan] != FLOAT)
            continue;
    
        for (int y = 0; y < _height; ++y)
        {
            float       src = 0;
            const char *srcXdr = _rowPtrs[chan][y];
            char       *dstXdr = tmpHalfBufferPtr;
           
            for (int x = 0; x < _width; ++x)
            {

                Xdr::read<CharPtrIO> (srcXdr, src);
                Xdr::write<CharPtrIO> (dstXdr, ((half)src).bits());
            }

            _rowPtrs[chan][y] = (const char *)tmpHalfBufferPtr;
            tmpHalfBufferPtr += _width * sizeof (unsigned short);
        }
    }

    currDcComp[0] = (unsigned short *)_packedDc;

    for (unsigned int chan = 1; chan < _rowPtrs.size(); ++chan)
        currDcComp[chan] = currDcComp[chan-1] + numBlocksX * numBlocksY;

    for (int blocky = 0; blocky < numBlocksY; ++blocky)
    {
        for (int blockx = 0; blockx < numBlocksX; ++blockx)
        {
            half           h;
            unsigned short tmpShortXdr, tmpShortNative;
            char          *tmpCharPtr;

            for (unsigned int chan = 0; chan < _rowPtrs.size(); ++chan)
            {
                for (int y = 0; y < 8; ++y)
                {
                    for (int x = 0; x < 8; ++x)
                    {
                        int vx = 8 * blockx + x;
                        int vy = 8 * blocky + y;

                        if (vx >= _width)
                            vx = _width - (vx - (_width - 1));
                        
                        if (vx < 0) vx = _width-1;

                        if (vy >=_height)
                            vy = _height - (vy - (_height - 1));

                        if (vy < 0) vy = _height-1;
                    
                        tmpShortXdr =
                            ((const unsigned short *)(_rowPtrs[chan])[vy])[vx];

                        if (_toNonlinear)
                        {
                            h.setBits (_toNonlinear[tmpShortXdr]);
                        }
                        else
                        {
                            const char *tmpConstCharPtr =
                                (const char *)(&tmpShortXdr);

                            Xdr::read<CharPtrIO>
                                (tmpConstCharPtr, tmpShortNative);

                            h.setBits(tmpShortNative);
                        }

                        _dctData[chan]._buffer[y * 8 + x] = (float)h;
                    }
                }
            }

            if (_rowPtrs.size() == 3)
            {
                csc709Forward64 (_dctData[0]._buffer, 
                                 _dctData[1]._buffer, 
                                 _dctData[2]._buffer);
            }

            for (unsigned int chan = 0; chan < _rowPtrs.size(); ++chan)
            {
                dctForward8x8(_dctData[chan]._buffer);

                if (chan == 0)
                {
                    for (int i = 0; i < 64; ++i)
                    {
                        halfCoef[i] =
                            quantize ((half)_dctData[chan]._buffer[i],
                                      _quantBaseError*_quantTableY[i]);
                    }
                }
                else
                {
                    for (int i = 0; i < 64; ++i)
                    {
                        halfCoef[i] =
                            quantize ((half)_dctData[chan]._buffer[i],
                                      _quantBaseError*_quantTableCbCr[i]);
                    }
                }

                toZigZag (halfZigCoef, halfCoef);

                for (int i = 0; i < 64; ++i)
                {
                    tmpCharPtr = (char *)&tmpShortXdr;
                    Xdr::write<CharPtrIO>(tmpCharPtr, halfZigCoef[i].bits());
                    halfZigCoef[i].setBits(tmpShortXdr);
                }

                *currDcComp[chan]++ = halfZigCoef[0].bits();
                _numDcComp++;
                
                rleAc (halfZigCoef, currAcComp);
            }
        }
    }
}

void 
DwaCompressor::LossyDctEncoderBase::toZigZag (half *dst, half *src) 
{
    const int remap[] =
    {
         0, 
         1,  8,
        16,  9,  2,
         3, 10, 17, 24,
        32, 25, 18, 11, 4,
         5, 12, 19, 26, 33, 40,
        48, 41, 34, 27, 20, 13, 6,
         7, 14, 21, 28, 35, 42, 49, 56,
            57, 50, 43, 36, 29, 22, 15,
                23, 30, 37, 44, 51, 58,
                    59, 52, 45, 38, 31,
                        39, 46, 53, 60,
                            61, 54, 47,
                                55, 62,
                                    63
    };

    for (int i=0; i<64; ++i)
        dst[i] = src[remap[i]];
}

int
DwaCompressor::LossyDctEncoderBase::countSetBits (unsigned short src)
{
    static const unsigned short numBitsSet[256] =
    {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };

    return numBitsSet[src & 0xff] + numBitsSet[src >> 8];
}

half
DwaCompressor::LossyDctEncoderBase::quantize (half src, float errorTolerance)
{
    half            tmp;
    float           srcFloat      = (float)src;
    int             numSetBits    = countSetBits(src.bits());
    const unsigned short *closest = closestData + closestDataOffset[src.bits()];

    for (int targetNumSetBits = numSetBits - 1;
         targetNumSetBits >= 0;
         --targetNumSetBits)
    {
        tmp.setBits (*closest);

        if (fabs ((float)tmp - srcFloat) < errorTolerance)
            return tmp;

        closest++;
    }

    return src;
}

void
DwaCompressor::LossyDctEncoderBase::rleAc
    (half *block,
     unsigned short *&acPtr)
{
    int dctComp              = 1; 
    unsigned short rleSymbol = 0x0;

    while (dctComp < 64)
    {
        int runLen = 1;
    
        if (block[dctComp].bits() != rleSymbol)
        {
            *acPtr++ =  block[dctComp].bits();
            _numAcComp++;

            dctComp += runLen;
            continue;
        }

        //
        // We're sitting on a 0, so see how big the run is.
        //

        while ((dctComp+runLen < 64) && 
               (block[dctComp+runLen].bits() == rleSymbol))
        {
            runLen++;
        }

        //
        // If the run len is too small, just output verbatim
        // otherwise output our run token
        //
        // Originally, we wouldn't have a separate symbol for
        // "end of block". But in some experimentation, it looks
        // like using 0xff00 for "end of block" can save a bit
        // of space. 
        //

        if (runLen == 1)
        {
            runLen           = 1;
            *acPtr++ = block[dctComp].bits();
            _numAcComp++;

            //
            // Using 0xff00 for "end of block"
            //
        }
        else if (runLen + dctComp == 64)
        {
            //
            // Signal EOB
            //

            *acPtr++ = 0xff00;
            _numAcComp++;
        }
        else
        {
            // 
            // Signal normal run
            //

            *acPtr++   = 0xff00 | runLen;
            _numAcComp++;
        }

        //
        // Advance by runLen
        //

        dctComp += runLen;
    }
}


// ==============================================================
//
//                     DwaCompressor
//
// --------------------------------------------------------------

// 
// DwaCompressor()
//

DwaCompressor::DwaCompressor
    (const Header &hdr,
     int maxScanLineSize,
     int numScanLines,
     AcCompression acCompression)
:
    Compressor(hdr),
    _acCompression(acCompression),
    _maxScanLineSize(maxScanLineSize),
    _numScanLines(numScanLines),
    _channels(hdr.channels()),
    _packedAcBuffer(0),
    _packedAcBufferSize(0),
    _packedDcBuffer(0),
    _packedDcBufferSize(0),
    _rleBuffer(0),
    _rleBufferSize(0),
    _outBuffer(0),
    _outBufferSize(0),
    _zip(0),
    _dwaCompressionLevel(45.0)
{
    _min[0] = hdr.dataWindow().min.x;
    _min[1] = hdr.dataWindow().min.y;
    _max[0] = hdr.dataWindow().max.x;
    _max[1] = hdr.dataWindow().max.y;

    for (int i=0; i < NUM_COMPRESSOR_SCHEMES; ++i) 
    {
        _planarUncBuffer[i] = 0;
        _planarUncBufferSize[i] = 0;
    }
    
    //
    // Check the header for a quality attribute
    //

    if (hasDwaCompressionLevel (hdr))
        _dwaCompressionLevel = dwaCompressionLevel (hdr);
}


DwaCompressor::~DwaCompressor()
{
    delete[] _packedAcBuffer;
    delete[] _packedDcBuffer;
    delete[] _rleBuffer;
    delete[] _outBuffer;
    delete _zip;

    for (int i=0; i<NUM_COMPRESSOR_SCHEMES; ++i)
        delete[] _planarUncBuffer[i];
}


int
DwaCompressor::numScanLines() const
{
    return _numScanLines;
}


Imf::Compressor::Format 
DwaCompressor::format() const
{
    if (GLOBAL_SYSTEM_LITTLE_ENDIAN)
        return NATIVE;
    else
        return XDR;
}


int
DwaCompressor::compress
    (const char *inPtr,
     int inSize,
     int minY,
     const char *&outPtr)
{
    return compress
        (inPtr,
         inSize, 
         Imath::Box2i (Imath::V2i (_min[0], minY),
                       Imath::V2i (_max[0], minY + numScanLines() - 1)),
         outPtr);
}


int
DwaCompressor::compressTile
    (const char *inPtr,
     int inSize,
     Imath::Box2i range,
     const char *&outPtr)
{
    return compress (inPtr, inSize, range, outPtr);
}


int 
DwaCompressor::compress
    (const char *inPtr,
     int inSize,
     Imath::Box2i range,
     const char  *&outPtr)
{
    const char *inDataPtr   = inPtr;
    char       *packedAcEnd = 0;
    char       *packedDcEnd = 0; 
    int         fileVersion = 2;   // Starting with 2, we write the channel
                                   // classification rules into the file

    if (fileVersion < 2) 
        initializeLegacyChannelRules();
    else 
        initializeDefaultChannelRules();

    size_t outBufferSize = 0;
    initializeBuffers(outBufferSize);

    unsigned short          channelRuleSize = 0;
    std::vector<Classifier> channelRules;
    if (fileVersion >= 2) 
    {
        relevantChannelRules(channelRules);

        channelRuleSize = Xdr::size<unsigned short>();
        for (size_t i = 0; i < channelRules.size(); ++i) 
            channelRuleSize += channelRules[i].size();
    }

    //
    // Remember to allocate _outBuffer, if we haven't done so already.
    //

    outBufferSize += channelRuleSize;
    if (outBufferSize > _outBufferSize) 
    {
        _outBufferSize = outBufferSize;
        if (_outBuffer == 0)
            delete[] _outBuffer;       
        _outBuffer = new char[outBufferSize];
    }

    char *outDataPtr = &_outBuffer[NUM_SIZES_SINGLE * sizeof(Imf::Int64) +
                                   channelRuleSize];

    //
    // We might not be dealing with any color data, in which
    // case the AC buffer size will be 0, and deferencing
    // a vector will not be a good thing to do.
    //

    if (_packedAcBuffer)
        packedAcEnd = _packedAcBuffer;

    if (_packedDcBuffer)
        packedDcEnd = _packedDcBuffer;

    #define OBIDX(x) (Int64 *)&_outBuffer[x * sizeof (Int64)]

    Int64 *version                 = OBIDX (VERSION);
    Int64 *unknownUncompressedSize = OBIDX (UNKNOWN_UNCOMPRESSED_SIZE);
    Int64 *unknownCompressedSize   = OBIDX (UNKNOWN_COMPRESSED_SIZE);
    Int64 *acCompressedSize        = OBIDX (AC_COMPRESSED_SIZE);
    Int64 *dcCompressedSize        = OBIDX (DC_COMPRESSED_SIZE);
    Int64 *rleCompressedSize       = OBIDX (RLE_COMPRESSED_SIZE);
    Int64 *rleUncompressedSize     = OBIDX (RLE_UNCOMPRESSED_SIZE);
    Int64 *rleRawSize              = OBIDX (RLE_RAW_SIZE);

    Int64 *totalAcUncompressedCount = OBIDX (AC_UNCOMPRESSED_COUNT);
    Int64 *totalDcUncompressedCount = OBIDX (DC_UNCOMPRESSED_COUNT);

    Int64 *acCompression            = OBIDX (AC_COMPRESSION);

    int minX   = range.min.x;
    int maxX   = std::min(range.max.x, _max[0]);
    int minY   = range.min.y;
    int maxY   = std::min(range.max.y, _max[1]);

    //
    // Zero all the numbers in the chunk header
    //

    memset (_outBuffer, 0, NUM_SIZES_SINGLE * sizeof (Int64));

    //
    // Setup the AC compression strategy and the version in the data block,
    // then write the relevant channel classification rules if needed
    //
    *version       = fileVersion;  
    *acCompression = _acCompression;

    setupChannelData (minX, minY, maxX, maxY);

    if (fileVersion >= 2) 
    {
        char *writePtr = &_outBuffer[NUM_SIZES_SINGLE * sizeof(Imf::Int64)];
        Xdr::write<CharPtrIO> (writePtr, channelRuleSize);
        
        for (size_t i = 0; i < channelRules.size(); ++i) 
            channelRules[i].write(writePtr);
    }

    //
    // Determine the start of each row in the input buffer
    // Channels are interleaved by scanline
    //

    std::vector<bool> encodedChannels (_channelData.size());
    std::vector< std::vector<const char *> > rowPtrs (_channelData.size());

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
        encodedChannels[chan] = false;

    inDataPtr =  inPtr;

    for (int y = minY; y <= maxY; ++y)
    {
        for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
        {

            ChannelData *cd = &_channelData[chan];

            if (Imath::modp(y, cd->ySampling) != 0)
                continue;

            rowPtrs[chan].push_back(inDataPtr);
            inDataPtr += cd->width * Imf::pixelTypeSize(cd->type);
        }
    }

    inDataPtr = inPtr;

    // 
    // Make a pass over all our CSC sets and try to encode them first
    // 

    for (unsigned int csc = 0; csc < _cscSets.size(); ++csc)
    {

        LossyDctEncoderCsc encoder
            (_dwaCompressionLevel / 100000.f,
             rowPtrs[_cscSets[csc].idx[0]],
             rowPtrs[_cscSets[csc].idx[1]],
             rowPtrs[_cscSets[csc].idx[2]],
             packedAcEnd,
             packedDcEnd,
             dwaCompressorToNonlinear,
             _channelData[_cscSets[csc].idx[0]].width,
             _channelData[_cscSets[csc].idx[0]].height,
             _channelData[_cscSets[csc].idx[0]].type,
             _channelData[_cscSets[csc].idx[1]].type,
             _channelData[_cscSets[csc].idx[2]].type);

        encoder.execute();

        *totalAcUncompressedCount  += encoder.numAcValuesEncoded();
        *totalDcUncompressedCount  += encoder.numDcValuesEncoded();

        packedAcEnd += encoder.numAcValuesEncoded() * sizeof(unsigned short);
        packedDcEnd += encoder.numDcValuesEncoded() * sizeof(unsigned short);

        encodedChannels[_cscSets[csc].idx[0]] = true;
        encodedChannels[_cscSets[csc].idx[1]] = true;
        encodedChannels[_cscSets[csc].idx[2]] = true;
    }

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
    {
        ChannelData *cd = &_channelData[chan];

        if (encodedChannels[chan])
            continue;

        switch (cd->compression)
        {
          case LOSSY_DCT:

            //
            // For LOSSY_DCT, treat this just like the CSC'd case,
            // but only operate on one channel
            //

            {
                const unsigned short *nonlinearLut = 0;

                if (!cd->pLinear)
                    nonlinearLut = dwaCompressorToNonlinear; 

                LossyDctEncoder encoder
                    (_dwaCompressionLevel / 100000.f,
                     rowPtrs[chan],
                     packedAcEnd,
                     packedDcEnd,
                     nonlinearLut,
                     cd->width,
                     cd->height,
                     cd->type);

                encoder.execute();

                *totalAcUncompressedCount  += encoder.numAcValuesEncoded();
                *totalDcUncompressedCount  += encoder.numDcValuesEncoded();

                packedAcEnd +=
                    encoder.numAcValuesEncoded() * sizeof (unsigned short);

                packedDcEnd +=
                    encoder.numDcValuesEncoded() * sizeof (unsigned short);
            }

            break;

          case RLE:

            //
            // For RLE, bash the bytes up so that the first bytes of each
            // pixel are contingous, as are the second bytes, and so on.
            //

            for (unsigned int y = 0; y < rowPtrs[chan].size(); ++y)
            {
                const char *row = rowPtrs[chan][y];

                for (int x = 0; x < cd->width; ++x)
                {
                    for (int byte = 0;
                         byte < Imf::pixelTypeSize (cd->type);
                         ++byte)
                    {
                            
                        *cd->planarUncRleEnd[byte]++ = *row++;
                    }
                }

                *rleRawSize += cd->width * Imf::pixelTypeSize(cd->type);
            }

            break;

          case UNKNOWN:
           
            //
            // Otherwise, just copy data over verbatim
            //

            {
                int scanlineSize = cd->width * Imf::pixelTypeSize(cd->type);

                for (unsigned int y = 0; y < rowPtrs[chan].size(); ++y)
                {
                    memcpy (cd->planarUncBufferEnd,
                            rowPtrs[chan][y],
                            scanlineSize);
    
                    cd->planarUncBufferEnd += scanlineSize;
                }

                *unknownUncompressedSize += cd->planarUncSize;
            }

            break;

          default:

            assert (false);
        }

        encodedChannels[chan] = true;
    }

    //
    // Pack the Unknown data into the output buffer first. Instead of
    // just copying it uncompressed, try zlib compression at least.
    //

    if (*unknownUncompressedSize > 0)
    {
        uLongf inSize  = (uLongf)(*unknownUncompressedSize);
        uLongf outSize = (uLongf)(ceil ((float)inSize * 1.01f) + 100);

        if (Z_OK != ::compress2 ((Bytef *)outDataPtr,
                                 &outSize,
                                 (const Bytef *)_planarUncBuffer[UNKNOWN],
                                 inSize,
                                 9))
        {
        }

        outDataPtr += outSize;
        *unknownCompressedSize = outSize;
    }

    //
    // Now, pack all the Lossy DCT coefficients into our output
    // buffer, with Huffman encoding.
    //
    // Also, record the compressed size and the number of 
    // uncompressed componentns we have.
    //

    if (*totalAcUncompressedCount > 0)
    { 
        switch (_acCompression)
        {
          case STATIC_HUFFMAN:

            *acCompressedSize = (int)
                hufCompress((unsigned short *)_packedAcBuffer,
                            (int)*totalAcUncompressedCount,
                            outDataPtr);                
            break;

          case DEFLATE:

            {
                uLongf destLen = (uLongf)
                    (2 * (*totalAcUncompressedCount) * sizeof (unsigned short));

                if (Z_OK != ::compress2
                                ((Bytef *)outDataPtr,
                                 &destLen,
                                 (Bytef *)_packedAcBuffer, 
                                 (uLong)(*totalAcUncompressedCount
                                                * sizeof (unsigned short)),
                                 9))
                {
                }

                *acCompressedSize = destLen;        
            }

            break;

          default:
            
            assert (false);
        }

        outDataPtr += *acCompressedSize;
    }

    // 
    // Handle the DC components separately
    //

    if (*totalDcUncompressedCount > 0)
    {
        *dcCompressedSize = _zip->compress
            (_packedDcBuffer,
             (int)(*totalDcUncompressedCount) * sizeof (unsigned short),
             outDataPtr);

        outDataPtr += *dcCompressedSize;
    }

    // 
    // If we have RLE data, first RLE encode it and set the uncompressed
    // size. Then, deflate the results and set the compressed size.
    //    

    if (*rleRawSize > 0)
    {
        *rleUncompressedSize = rleCompress
            ((int)(*rleRawSize),
             _planarUncBuffer[RLE],
             (signed char *)_rleBuffer);

        uLongf dstLen =
            (uLongf)ceil (1.01f * (float) * rleUncompressedSize) + 24;

        if (Z_OK != ::compress2
                        ((Bytef *)outDataPtr, 
                         &dstLen, 
                         (Bytef *)_rleBuffer, 
                         (uLong)(*rleUncompressedSize),
                         9))
        {
        }
        
       *rleCompressedSize = dstLen;
        outDataPtr       += *rleCompressedSize;
    }

    // 
    // Flip the counters to XDR format
    //         

    for (int i = 0; i < NUM_SIZES_SINGLE; ++i)
    {
        Int64  src = *(((Int64 *)_outBuffer) + i);
        char  *dst = (char *)(((Int64 *)_outBuffer) + i);

        Xdr::write<CharPtrIO> (dst, src);
    }

    //
    // We're done - compute the number of bytes we packed
    //

    outPtr = _outBuffer;

    return static_cast<int>(outDataPtr - _outBuffer + 1);
}


int
DwaCompressor::uncompress
    (const char *inPtr,
     int inSize,
     int minY,
     const char *&outPtr)
{
    return uncompress (inPtr,
                       inSize,
                       Imath::Box2i (Imath::V2i (_min[0], minY),
                       Imath::V2i (_max[0], minY + numScanLines() - 1)),
                       outPtr);
}


int 
DwaCompressor::uncompressTile
    (const char *inPtr,
     int inSize,
     Imath::Box2i range,
     const char *&outPtr)
{
    return uncompress (inPtr, inSize, range, outPtr);
}


int 
DwaCompressor::uncompress
    (const char *inPtr,
     int inSize,
     Imath::Box2i range,
     const char *&outPtr)
{
    int minX = range.min.x;
    int maxX = std::min (range.max.x, _max[0]);
    int minY = range.min.y;
    int maxY = std::min (range.max.y, _max[1]);

    int headerSize = NUM_SIZES_SINGLE*sizeof(Int64);
    if (inSize < headerSize) 
    {
    }

    for (int i = 0; i < NUM_SIZES_SINGLE; ++i)
    {
        Int64      *dst =  (((Int64 *)inPtr) + i);
        const char *src = (char *)(((Int64 *)inPtr) + i);

        Xdr::read<CharPtrIO> (src, *dst);
    }

    const Int64 *inPtr64 = (const Int64*) inPtr;

    Int64 version                  = *(inPtr64 + VERSION);
    Int64 unknownUncompressedSize  = *(inPtr64 + UNKNOWN_UNCOMPRESSED_SIZE);
    Int64 unknownCompressedSize    = *(inPtr64 + UNKNOWN_COMPRESSED_SIZE);
    Int64 acCompressedSize         = *(inPtr64 + AC_COMPRESSED_SIZE);
    Int64 dcCompressedSize         = *(inPtr64 + DC_COMPRESSED_SIZE);
    Int64 rleCompressedSize        = *(inPtr64 + RLE_COMPRESSED_SIZE);
    Int64 rleUncompressedSize      = *(inPtr64 + RLE_UNCOMPRESSED_SIZE);
    Int64 rleRawSize               = *(inPtr64 + RLE_RAW_SIZE);
 
    Int64 totalAcUncompressedCount = *(inPtr64 + AC_UNCOMPRESSED_COUNT); 
    Int64 totalDcUncompressedCount = *(inPtr64 + DC_UNCOMPRESSED_COUNT); 

    Int64 acCompression            = *(inPtr64 + AC_COMPRESSION); 

    Int64 compressedSize           = unknownCompressedSize + 
                                     acCompressedSize +
                                     dcCompressedSize +
                                     rleCompressedSize;

    const char *dataPtr            = inPtr + NUM_SIZES_SINGLE * sizeof(Int64);

    if (inSize < (headerSize + compressedSize) ||
        inSize < unknownCompressedSize ||
        inSize < acCompressedSize ||
        inSize < dcCompressedSize ||
        inSize < rleCompressedSize)
    {
    }

    if (unknownUncompressedSize < 0  || 
        unknownCompressedSize < 0    ||
        acCompressedSize < 0         || 
        dcCompressedSize < 0         ||
        rleCompressedSize < 0        || 
        rleUncompressedSize < 0      ||
        rleRawSize < 0               ||  
        totalAcUncompressedCount < 0 || 
        totalDcUncompressedCount < 0) 
    {
    }

    if (version < 2) 
        initializeLegacyChannelRules();
    else
    {
        unsigned short ruleSize = 0;
        Xdr::read<CharPtrIO>(dataPtr, ruleSize);

        if (ruleSize < Xdr::size<unsigned short>())
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (corrupt header file).");

        headerSize += ruleSize;
        if (inSize < headerSize + compressedSize)
            throw Iex::InputExc("Error uncompressing DWA data"
                                " (truncated file).");

        _channelRules.clear();
        ruleSize -= Xdr::size<unsigned short> ();
        while (ruleSize > 0) 
        {
            Classifier rule(dataPtr, ruleSize);
            
            _channelRules.push_back(rule);
            ruleSize -= rule.size();
        }
    }

    size_t outBufferSize = 0;
    initializeBuffers(outBufferSize);

    if (_maxScanLineSize * numScanLines() > _outBufferSize) 
    {
        _outBufferSize = _maxScanLineSize * numScanLines();
        if (_outBuffer != 0)
            delete[] _outBuffer;
        _outBuffer = new char[_maxScanLineSize * numScanLines()];
    }

    char *outBufferEnd = _outBuffer;

    char *packedAcBufferEnd = 0; 

    if (_packedAcBuffer)
        packedAcBufferEnd = _packedAcBuffer;

    char *packedDcBufferEnd = 0;

    if (_packedDcBuffer)
        packedDcBufferEnd = _packedDcBuffer;
    
    const char *compressedUnknownBuf = dataPtr;

    const char *compressedAcBuf      = compressedUnknownBuf + 
                                  static_cast<ptrdiff_t>(unknownCompressedSize);
    const char *compressedDcBuf      = compressedAcBuf +
                                  static_cast<ptrdiff_t>(acCompressedSize);
    const char *compressedRleBuf     = compressedDcBuf + 
                                  static_cast<ptrdiff_t>(dcCompressedSize);

    if ((version < 0) || (version > 2))
        throw Iex::InputExc ("Invalid version of compressed data block");    

    setupChannelData(minX, minY, maxX, maxY);

    if (unknownCompressedSize > 0)
    {
        uLongf outSize = static_cast<uLongf>(
                ceil( (float)unknownUncompressedSize * 1.01) + 100);

        if (unknownUncompressedSize < 0 || 
            outSize > _planarUncBufferSize[UNKNOWN]) 
        {
        }

        if (Z_OK != ::uncompress
                        ((Bytef *)_planarUncBuffer[UNKNOWN],
                         &outSize,
                         (Bytef *)compressedUnknownBuf,
                         (uLong)unknownCompressedSize))
        {
        }
    }

    if (acCompressedSize > 0)
    {
        if (totalAcUncompressedCount*sizeof(unsigned short) > _packedAcBufferSize)
        {
        }

        switch (acCompression)
        {
          case STATIC_HUFFMAN:

            hufUncompress
                (compressedAcBuf, 
                 (int)acCompressedSize, 
                 (unsigned short *)_packedAcBuffer, 
                 (int)totalAcUncompressedCount); 

            break;

          case DEFLATE:
            {
                uLongf destLen =
                    (int)(totalAcUncompressedCount) * sizeof (unsigned short);

                if (Z_OK != ::uncompress
                                ((Bytef *)_packedAcBuffer,
                                 &destLen,
                                 (Bytef *)compressedAcBuf,
                                 (uLong)acCompressedSize))
                {
                }

                if (totalAcUncompressedCount * sizeof (unsigned short) !=
                                destLen)
                {
                }
            }
            break;

          default:

            throw Iex::NoImplExc ("Unknown AC Compression");
            break;
        }
    }

    if (dcCompressedSize > 0)
    {
        if (totalDcUncompressedCount*sizeof(unsigned short) > _packedDcBufferSize)
        {
        }

        if (_zip->uncompress
                    (compressedDcBuf, (int)dcCompressedSize, _packedDcBuffer)
            != (int)totalDcUncompressedCount * sizeof (unsigned short))
        {
        }
    }

    if (rleRawSize > 0)
    {
        if (rleUncompressedSize > _rleBufferSize ||
            rleRawSize > _planarUncBufferSize[RLE])
        {
        }
 
        uLongf dstLen = (uLongf)rleUncompressedSize;

        if (Z_OK != ::uncompress
                        ((Bytef *)_rleBuffer,
                         &dstLen,
                         (Bytef *)compressedRleBuf,
                         (uLong)rleCompressedSize))
        {
        }

        if (dstLen != rleUncompressedSize)
            throw Iex::BaseExc("RLE data corrupted");

        if (rleUncompress
                ((int)rleUncompressedSize, 
                 (int)rleRawSize,
                 (signed char *)_rleBuffer,
                 _planarUncBuffer[RLE]) != rleRawSize)
        {        
        }
    }

    std::vector<bool> decodedChannels (_channelData.size());
    std::vector< std::vector<char *> > rowPtrs (_channelData.size());

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
        decodedChannels[chan] = false;

    outBufferEnd = _outBuffer;

    for (int y = minY; y <= maxY; ++y)
    {
        for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
        {
            ChannelData *cd = &_channelData[chan];

            if (Imath::modp (y, cd->ySampling) != 0)
                continue;

            rowPtrs[chan].push_back (outBufferEnd);
            outBufferEnd += cd->width * Imf::pixelTypeSize (cd->type);
        }
    }

    for (unsigned int csc = 0; csc < _cscSets.size(); ++csc)
    {
        int rChan = _cscSets[csc].idx[0];    
        int gChan = _cscSets[csc].idx[1];    
        int bChan = _cscSets[csc].idx[2];    


        LossyDctDecoderCsc decoder
            (rowPtrs[rChan],
             rowPtrs[gChan],
             rowPtrs[bChan],
             packedAcBufferEnd,
             packedDcBufferEnd,
             dwaCompressorToLinear,
             _channelData[rChan].width,
             _channelData[rChan].height,
             _channelData[rChan].type,
             _channelData[gChan].type,
             _channelData[bChan].type);

        decoder.execute();

        packedAcBufferEnd +=
            decoder.numAcValuesEncoded() * sizeof (unsigned short);

        packedDcBufferEnd +=
            decoder.numDcValuesEncoded() * sizeof (unsigned short);

        decodedChannels[rChan] = true;
        decodedChannels[gChan] = true;
        decodedChannels[bChan] = true;
    }

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
    {
        if (decodedChannels[chan])
            continue;

        ChannelData *cd = &_channelData[chan];
        int pixelSize = Imf::pixelTypeSize (cd->type);

        switch (cd->compression)
        {
          case LOSSY_DCT:

            {
                const unsigned short *linearLut = 0;

                if (!cd->pLinear)
                    linearLut = dwaCompressorToLinear;

                LossyDctDecoder decoder
                    (rowPtrs[chan],
                     packedAcBufferEnd,
                     packedDcBufferEnd,
                     linearLut,
                     cd->width,
                     cd->height,
                     cd->type);

                decoder.execute();   

                packedAcBufferEnd += 
                    decoder.numAcValuesEncoded() * sizeof (unsigned short);

                packedDcBufferEnd += 
                    decoder.numDcValuesEncoded() * sizeof (unsigned short);
            }

            break;

          case RLE:

            {
                int row = 0;

                for (int y = minY; y <= maxY; ++y)
                {
                    if (Imath::modp (y, cd->ySampling) != 0)
                        continue;

                    char *dst = rowPtrs[chan][row];

                    if (pixelSize == 2)
                    {
                        interleaveByte2 (dst, 
                                         cd->planarUncRleEnd[0],
                                         cd->planarUncRleEnd[1],
                                         cd->width);
                                            
                        cd->planarUncRleEnd[0] += cd->width;
                        cd->planarUncRleEnd[1] += cd->width;
                    }
                    else
                    {
                        for (int x = 0; x < cd->width; ++x)
                        {
                            for (int byte = 0; byte < pixelSize; ++byte)
                            {
                               *dst++ = *cd->planarUncRleEnd[byte]++;
                            }
                        }
                    }

                    row++;
                }
            }

            break;

          case UNKNOWN:

            {
                int row             = 0;
                int dstScanlineSize = cd->width * Imf::pixelTypeSize (cd->type);

                for (int y = minY; y <= maxY; ++y)
                {
                    if (Imath::modp (y, cd->ySampling) != 0)
                        continue;

                    if (cd->planarUncBufferEnd + dstScanlineSize - _planarUncBuffer[UNKNOWN] > _planarUncBufferSize[UNKNOWN] )
                    {
                    }

                    memcpy (rowPtrs[chan][row],
                            cd->planarUncBufferEnd,
                            dstScanlineSize);

                    cd->planarUncBufferEnd += dstScanlineSize;
                    row++;
                }
            }

            break;

          default:

            throw Iex::NoImplExc ("Unhandled compression scheme case");
            break;
        }

        decodedChannels[chan] = true;
    }

    outPtr = _outBuffer;
    return (int)(outBufferEnd - _outBuffer);
}

void
DwaCompressor::initializeFuncs()
{
    convertFloatToHalf64 = convertFloatToHalf64_scalar;
    fromHalfZigZag       = fromHalfZigZag_scalar;

    CpuId cpuId;

    if (cpuId.avx && cpuId.f16c)
    {
        convertFloatToHalf64 = convertFloatToHalf64_f16c;
        fromHalfZigZag       = fromHalfZigZag_f16c;
    } 

    dctInverse8x8_0 = dctInverse8x8_scalar<0>;
    dctInverse8x8_1 = dctInverse8x8_scalar<1>;
    dctInverse8x8_2 = dctInverse8x8_scalar<2>;
    dctInverse8x8_3 = dctInverse8x8_scalar<3>;
    dctInverse8x8_4 = dctInverse8x8_scalar<4>;
    dctInverse8x8_5 = dctInverse8x8_scalar<5>;
    dctInverse8x8_6 = dctInverse8x8_scalar<6>;
    dctInverse8x8_7 = dctInverse8x8_scalar<7>;

    if (cpuId.avx) 
    {
        dctInverse8x8_0 = dctInverse8x8_avx<0>;
        dctInverse8x8_1 = dctInverse8x8_avx<1>;
        dctInverse8x8_2 = dctInverse8x8_avx<2>;
        dctInverse8x8_3 = dctInverse8x8_avx<3>;
        dctInverse8x8_4 = dctInverse8x8_avx<4>;
        dctInverse8x8_5 = dctInverse8x8_avx<5>;
        dctInverse8x8_6 = dctInverse8x8_avx<6>;
        dctInverse8x8_7 = dctInverse8x8_avx<7>;
    } 
    else if (cpuId.sse2) 
    {
        dctInverse8x8_0 = dctInverse8x8_sse2<0>;
        dctInverse8x8_1 = dctInverse8x8_sse2<1>;
        dctInverse8x8_2 = dctInverse8x8_sse2<2>;
        dctInverse8x8_3 = dctInverse8x8_sse2<3>;
        dctInverse8x8_4 = dctInverse8x8_sse2<4>;
        dctInverse8x8_5 = dctInverse8x8_sse2<5>;
        dctInverse8x8_6 = dctInverse8x8_sse2<6>;
        dctInverse8x8_7 = dctInverse8x8_sse2<7>;
    }
}

void
DwaCompressor::initializeBuffers (size_t &outBufferSize)
{
    classifyChannels (_channels, _channelData, _cscSets);

    int maxOutBufferSize  = 0;
    int numLossyDctChans  = 0;
    int unknownBufferSize = 0;
    int rleBufferSize     = 0;

    int maxLossyDctAcSize = (int)ceil ((float)numScanLines() / 8.0f) * 
                            (int)ceil ((float)(_max[0] - _min[0] + 1) / 8.0f) *
                            63 * sizeof (unsigned short);

    int maxLossyDctDcSize = (int)ceil ((float)numScanLines() / 8.0f) * 
                            (int)ceil ((float)(_max[0] - _min[0] + 1) / 8.0f) *
                            sizeof (unsigned short);

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
    {
        switch (_channelData[chan].compression)
        {
          case LOSSY_DCT:

            maxOutBufferSize += 2 * maxLossyDctAcSize + 65536;
            numLossyDctChans++;
            break;

          case RLE:
            {
                int rleAmount = 2 * numScanLines() * (_max[0] - _min[0] + 1) *
                                Imf::pixelTypeSize (_channelData[chan].type);

                rleBufferSize += rleAmount;
            }
            break;


          case UNKNOWN:

            unknownBufferSize += numScanLines() * (_max[0] - _min[0] + 1) *
                                 Imf::pixelTypeSize (_channelData[chan].type);
            break;

          default:

            throw Iex::NoImplExc ("Unhandled compression scheme case");
            break;
        }
    }

    maxOutBufferSize += (int)(ceil (1.01f * (float)rleBufferSize) + 100);
    
    maxOutBufferSize += (int)(ceil (1.01f * (float)unknownBufferSize) + 100);

    if (_zip == 0) 
        _zip = new Zip (maxLossyDctDcSize * numLossyDctChans);
    else if (_zip->maxRawSize() < maxLossyDctDcSize * numLossyDctChans)
    {
        delete _zip;
        _zip = new Zip (maxLossyDctDcSize * numLossyDctChans);
    }


    maxOutBufferSize += _zip->maxCompressedSize();

    maxOutBufferSize += NUM_SIZES_SINGLE * sizeof (Int64); 

    outBufferSize = maxOutBufferSize;

    if (maxLossyDctAcSize * numLossyDctChans > _packedAcBufferSize)
    {
        _packedAcBufferSize = maxLossyDctAcSize * numLossyDctChans;
        if (_packedAcBuffer != 0) 
            delete[] _packedAcBuffer;
        _packedAcBuffer = new char[_packedAcBufferSize];
    }

    if (maxLossyDctDcSize * numLossyDctChans > _packedDcBufferSize)
    {
        _packedDcBufferSize = maxLossyDctDcSize * numLossyDctChans;
        if (_packedDcBuffer != 0) 
            delete[] _packedDcBuffer;
        _packedDcBuffer     = new char[_packedDcBufferSize];
    }

    if (rleBufferSize > _rleBufferSize) 
    {
        _rleBufferSize = rleBufferSize;
        if (_rleBuffer != 0) 
            delete[] _rleBuffer;
        _rleBuffer = new char[rleBufferSize];
    }


    int planarUncBufferSize[NUM_COMPRESSOR_SCHEMES];
    for (int i=0; i<NUM_COMPRESSOR_SCHEMES; ++i)
        planarUncBufferSize[i] = 0;

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
    {
        switch (_channelData[chan].compression)
        {
          case LOSSY_DCT:
            break;

          case RLE:
            planarUncBufferSize[RLE] +=
                     numScanLines() * (_max[0] - _min[0] + 1) *
                     Imf::pixelTypeSize (_channelData[chan].type);
            break;

          case UNKNOWN: 
            planarUncBufferSize[UNKNOWN] +=
                     numScanLines() * (_max[0] - _min[0] + 1) *
                     Imf::pixelTypeSize (_channelData[chan].type);
            break;

          default:
            throw Iex::NoImplExc ("Unhandled compression scheme case");
            break;
        }
    }

    if (planarUncBufferSize[UNKNOWN] > 0)
    {
        planarUncBufferSize[UNKNOWN] =
            (int) ceil (1.01f * (float)planarUncBufferSize[UNKNOWN]) + 100;
    }

    for (int i = 0; i < NUM_COMPRESSOR_SCHEMES; ++i)
    {
        if (planarUncBufferSize[i] > _planarUncBufferSize[i]) 
        {
            _planarUncBufferSize[i] = planarUncBufferSize[i];
            if (_planarUncBuffer[i] != 0) 
                delete[] _planarUncBuffer[i];
            _planarUncBuffer[i] = new char[planarUncBufferSize[i]];
        }
    }
}

void
DwaCompressor::initializeDefaultChannelRules ()
{
    _channelRules.clear();

    _channelRules.push_back (Classifier ("R",     LOSSY_DCT, HALF,   0, false));
    _channelRules.push_back (Classifier ("R",     LOSSY_DCT, FLOAT,  0, false));
    _channelRules.push_back (Classifier ("G",     LOSSY_DCT, HALF,   1, false));
    _channelRules.push_back (Classifier ("G",     LOSSY_DCT, FLOAT,  1, false));
    _channelRules.push_back (Classifier ("B",     LOSSY_DCT, HALF,   2, false));
    _channelRules.push_back (Classifier ("B",     LOSSY_DCT, FLOAT,  2, false));

    _channelRules.push_back (Classifier ("Y",     LOSSY_DCT, HALF,  -1, false));
    _channelRules.push_back (Classifier ("Y",     LOSSY_DCT, FLOAT, -1, false));
    _channelRules.push_back (Classifier ("BY",    LOSSY_DCT, HALF,  -1, false));
    _channelRules.push_back (Classifier ("BY",    LOSSY_DCT, FLOAT, -1, false));
    _channelRules.push_back (Classifier ("RY",    LOSSY_DCT, HALF,  -1, false));
    _channelRules.push_back (Classifier ("RY",    LOSSY_DCT, FLOAT, -1, false));

    _channelRules.push_back (Classifier ("A",     RLE,       UINT,  -1, false));
    _channelRules.push_back (Classifier ("A",     RLE,       HALF,  -1, false));
    _channelRules.push_back (Classifier ("A",     RLE,       FLOAT, -1, false));
}

void
DwaCompressor::initializeLegacyChannelRules ()
{
    _channelRules.clear();

    _channelRules.push_back (Classifier ("r",     LOSSY_DCT, HALF,   0, true));
    _channelRules.push_back (Classifier ("r",     LOSSY_DCT, FLOAT,  0, true));
    _channelRules.push_back (Classifier ("red",   LOSSY_DCT, HALF,   0, true));
    _channelRules.push_back (Classifier ("red",   LOSSY_DCT, FLOAT,  0, true));
    _channelRules.push_back (Classifier ("g",     LOSSY_DCT, HALF,   1, true));
    _channelRules.push_back (Classifier ("g",     LOSSY_DCT, FLOAT,  1, true));
    _channelRules.push_back (Classifier ("grn",   LOSSY_DCT, HALF,   1, true));
    _channelRules.push_back (Classifier ("grn",   LOSSY_DCT, FLOAT,  1, true));
    _channelRules.push_back (Classifier ("green", LOSSY_DCT, HALF,   1, true));
    _channelRules.push_back (Classifier ("green", LOSSY_DCT, FLOAT,  1, true));
    _channelRules.push_back (Classifier ("b",     LOSSY_DCT, HALF,   2, true));
    _channelRules.push_back (Classifier ("b",     LOSSY_DCT, FLOAT,  2, true));
    _channelRules.push_back (Classifier ("blu",   LOSSY_DCT, HALF,   2, true));
    _channelRules.push_back (Classifier ("blu",   LOSSY_DCT, FLOAT,  2, true));
    _channelRules.push_back (Classifier ("blue",  LOSSY_DCT, HALF,   2, true));
    _channelRules.push_back (Classifier ("blue",  LOSSY_DCT, FLOAT,  2, true));

    _channelRules.push_back (Classifier ("y",     LOSSY_DCT, HALF,  -1, true));
    _channelRules.push_back (Classifier ("y",     LOSSY_DCT, FLOAT, -1, true));
    _channelRules.push_back (Classifier ("by",    LOSSY_DCT, HALF,  -1, true));
    _channelRules.push_back (Classifier ("by",    LOSSY_DCT, FLOAT, -1, true));
    _channelRules.push_back (Classifier ("ry",    LOSSY_DCT, HALF,  -1, true));
    _channelRules.push_back (Classifier ("ry",    LOSSY_DCT, FLOAT, -1, true));
    _channelRules.push_back (Classifier ("a",     RLE,       UINT,  -1, true));
    _channelRules.push_back (Classifier ("a",     RLE,       HALF,  -1, true));
    _channelRules.push_back (Classifier ("a",     RLE,       FLOAT, -1, true));
}

void
DwaCompressor::relevantChannelRules (std::vector<Classifier> &rules) const 
{
    rules.clear();

    std::vector<std::string> suffixes;
    
    for (size_t cd = 0; cd < _channelData.size(); ++cd) 
    {
        std::string suffix  = _channelData[cd].name;
        size_t      lastDot = suffix.find_last_of ('.');

        if (lastDot != std::string::npos)
            suffix = suffix.substr (lastDot+1, std::string::npos);

        suffixes.push_back(suffix);
    }

    
    for (size_t i = 0; i < _channelRules.size(); ++i) 
    {
        for (size_t cd = 0; cd < _channelData.size(); ++cd) 
        {
            if (_channelRules[i].match (suffixes[cd], _channelData[cd].type ))
            {
                rules.push_back (_channelRules[i]);
                break;
            }
        }       
    }
}

void
DwaCompressor::classifyChannels
    (ChannelList channels,
     std::vector<ChannelData> &chanData,
     std::vector<CscChannelSet> &cscData)
{
    std::map<std::string, DwaCompressor::CscChannelSet> prefixMap;
    std::vector<DwaCompressor::CscChannelSet>           tmpCscSet;

    unsigned int numChan = 0;

    for (ChannelList::Iterator c = channels.begin(); c != channels.end(); ++c)
        numChan++;
    
    if (numChan)
        chanData.resize (numChan);

    unsigned int offset = 0;

    for (ChannelList::Iterator c = channels.begin(); c != channels.end(); ++c)
    {
        chanData[offset].name        = std::string (c.name());
        chanData[offset].compression = UNKNOWN;
        chanData[offset].xSampling   = c.channel().xSampling;
        chanData[offset].ySampling   = c.channel().ySampling;
        chanData[offset].type        = c.channel().type;
        chanData[offset].pLinear     = c.channel().pLinear;

        offset++;
    }

    for (offset = 0; offset<numChan; ++offset)
    {
        std::string prefix  = "";
        std::string suffix  = chanData[offset].name;
        size_t      lastDot = suffix.find_last_of ('.');

        if (lastDot != std::string::npos)
        {
            prefix = suffix.substr (0,         lastDot);
            suffix = suffix.substr (lastDot+1, std::string::npos);
        } 

        std::map<std::string, DwaCompressor::CscChannelSet>::iterator 
            theSet = prefixMap.find (prefix);

        if (theSet == prefixMap.end())
        {
            DwaCompressor::CscChannelSet tmpSet;

            tmpSet.idx[0] = 
            tmpSet.idx[1] = 
            tmpSet.idx[2] = -1;

            prefixMap[prefix] = tmpSet;
        }

        for (std::vector<Classifier>::iterator i = _channelRules.begin();
             i != _channelRules.end();
             ++i)
        {
            if ( i->match(suffix, chanData[offset].type) )
            {
                chanData[offset].compression = i->_scheme;

                if ( i->_cscIdx >= 0)
                    prefixMap[prefix].idx[i->_cscIdx] = offset;
            }
        }
    }

    for (std::map<std::string, DwaCompressor::CscChannelSet>::iterator 
         theItem = prefixMap.begin(); theItem != prefixMap.end();
         ++theItem)
    {
        int red = (*theItem).second.idx[0];
        int grn = (*theItem).second.idx[1];
        int blu = (*theItem).second.idx[2];

        if ((red < 0) || (grn < 0) || (blu < 0))
            continue;

        if ((chanData[red].xSampling != chanData[grn].xSampling) ||
            (chanData[red].xSampling != chanData[blu].xSampling) ||
            (chanData[grn].xSampling != chanData[blu].xSampling) ||
            (chanData[red].ySampling != chanData[grn].ySampling) ||
            (chanData[red].ySampling != chanData[blu].ySampling) ||
            (chanData[grn].ySampling != chanData[blu].ySampling))
        {
            continue;
        }
        
        tmpCscSet.push_back ((*theItem).second);
    }
    
    size_t numCsc = tmpCscSet.size();

    if (numCsc)
        cscData.resize(numCsc);

    for (offset = 0; offset < numCsc; ++offset)
        cscData[offset] = tmpCscSet[offset];
}

void
DwaCompressor::setupChannelData (int minX, int minY, int maxX, int maxY)
{
    char *planarUncBuffer[NUM_COMPRESSOR_SCHEMES];

    for (int i=0; i<NUM_COMPRESSOR_SCHEMES; ++i)
    {
        planarUncBuffer[i] = 0;

        if (_planarUncBuffer[i])
            planarUncBuffer[i] =  _planarUncBuffer[i];
    }

    for (unsigned int chan = 0; chan < _channelData.size(); ++chan)
    {
        ChannelData *cd = &_channelData[chan];

        cd->width  = Imf::numSamples (cd->xSampling, minX, maxX);
        cd->height = Imf::numSamples (cd->ySampling, minY, maxY);
                                
        cd->planarUncSize =
            cd->width * cd->height * Imf::pixelTypeSize (cd->type);
                                  
        cd->planarUncBuffer    = planarUncBuffer[cd->compression];
        cd->planarUncBufferEnd = cd->planarUncBuffer;

        cd->planarUncRle[0]    = cd->planarUncBuffer;
        cd->planarUncRleEnd[0] = cd->planarUncRle[0];

        for (int byte = 1; byte < Imf::pixelTypeSize(cd->type); ++byte)
        {
            cd->planarUncRle[byte] = 
                         cd->planarUncRle[byte-1] + cd->width * cd->height;

            cd->planarUncRleEnd[byte] =
                         cd->planarUncRle[byte];
        }

        cd->planarUncType = cd->type;

        if (cd->compression == LOSSY_DCT)
        {
            cd->planarUncType = FLOAT;
        }
        else
        {
            planarUncBuffer[cd->compression] +=
                cd->width * cd->height * Imf::pixelTypeSize (cd->planarUncType);
        }
    }
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
