// Copyright 2016 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Internal header for animation related functions.
//
// Author: Hui Su (huisu@google.com)

#ifndef WEBP_MUX_ANIMI_H_
#define WEBP_MUX_ANIMI_H_

#include "src/webp/mux.h"

#ifdef __cplusplus
extern "C" {
#endif

int WebPAnimEncoderRefineRect(
    const struct WebPPicture* const prev_canvas,
    const struct WebPPicture* const curr_canvas,
    int is_lossless, float quality, int* const x_offset, int* const y_offset,
    int* const width, int* const height);

#ifdef __cplusplus
}
#endif

#endif  // WEBP_MUX_ANIMI_H_
