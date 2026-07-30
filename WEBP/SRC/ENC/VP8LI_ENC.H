// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Lossless encoder: internal header.
//
// Author: Vikas Arora (vikaas.arora@gmail.com)

#ifndef WEBP_ENC_VP8LI_ENC_H_
#define WEBP_ENC_VP8LI_ENC_H_

#ifndef WEBP_NEAR_LOSSLESS
#define WEBP_NEAR_LOSSLESS 1
#endif

#include "src/enc/backward_references_enc.h"
#include "src/enc/histogram_enc.h"
#include "src/utils/bit_writer_utils.h"
#include "src/webp/encode.h"
#include "src/webp/format_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TRANSFORM_BITS 6

typedef enum {
  kEncoderNone = 0,
  kEncoderARGB,
  kEncoderNearLossless,
  kEncoderPalette
} VP8LEncoderARGBContent;

typedef struct {
  const WebPConfig* config_;
  const WebPPicture* pic_;

  uint32_t* argb_;
  VP8LEncoderARGBContent argb_content_;
  uint32_t* argb_scratch_;
  uint32_t* transform_data_;
  uint32_t* transform_mem_;
  size_t    transform_mem_size_;
  int       current_width_;

  int histo_bits_;
  int transform_bits_;
  int cache_bits_;

  int use_cross_color_;
  int use_subtract_green_;
  int use_predict_;
  int use_palette_;
  int palette_size_;
  uint32_t palette_[MAX_PALETTE_SIZE];

  struct VP8LBackwardRefs refs_[4];
  VP8LHashChain hash_chain_;
} VP8LEncoder;

int VP8LEncodeImage(const WebPConfig* const config,
                    const WebPPicture* const picture);

WebPEncodingError VP8LEncodeStream(const WebPConfig* const config,
                                   const WebPPicture* const picture,
                                   VP8LBitWriter* const bw, int use_cache);

#if (WEBP_NEAR_LOSSLESS == 1)
int VP8ApplyNearLossless(const WebPPicture* const picture, int quality,
                         uint32_t* const argb_dst);
#endif

void VP8LResidualImage(int width, int height, int bits, int low_effort,
                       uint32_t* const argb, uint32_t* const argb_scratch,
                       uint32_t* const image, int near_lossless, int exact,
                       int used_subtract_green);

void VP8LColorSpaceTransform(int width, int height, int bits, int quality,
                             uint32_t* const argb, uint32_t* image);

#ifdef __cplusplus
}
#endif

#endif
