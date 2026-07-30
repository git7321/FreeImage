// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Misc. common utility functions
//
// Authors: Skal (pascal.massimino@gmail.com)
//          Urvang (urvang@google.com)

#ifndef WEBP_UTILS_UTILS_H_
#define WEBP_UTILS_UTILS_H_

#include <assert.h>
#include <limits.h>

#include "src/dsp/dsp.h"
#include "src/webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WEBP_MAX_ALLOCABLE_MEMORY
#if SIZE_MAX > (1UI64 << 34)
#define WEBP_MAX_ALLOCABLE_MEMORY (1UI64 << 34)
#else
#define WEBP_MAX_ALLOCABLE_MEMORY ((1UI64 << 31) - (1 << 16))
#endif
#endif

WEBP_EXTERN void* WebPSafeMalloc(uint64_t nmemb, size_t size);
WEBP_EXTERN void* WebPSafeCalloc(uint64_t nmemb, size_t size);
WEBP_EXTERN void WebPSafeFree(void* const ptr);

#define WEBP_ALIGN_CST 31
#define WEBP_ALIGN(PTR) (((uintptr_t)(PTR) + WEBP_ALIGN_CST) & ~WEBP_ALIGN_CST)

#include <string.h>
static WEBP_INLINE uint32_t WebPMemToUint32(const uint8_t* const ptr) {
  uint32_t A;
  memcpy(&A, ptr, sizeof(A));
  return A;
}
static WEBP_INLINE void WebPUint32ToMem(uint8_t* const ptr, uint32_t val) {
  memcpy(ptr, &val, sizeof(val));
}

static WEBP_INLINE int GetLE16(const uint8_t* const data) {
  return (int)(data[0] << 0) | (data[1] << 8);
}

static WEBP_INLINE int GetLE24(const uint8_t* const data) {
  return GetLE16(data) | (data[2] << 16);
}

static WEBP_INLINE uint32_t GetLE32(const uint8_t* const data) {
  return GetLE16(data) | ((uint32_t)GetLE16(data + 2) << 16);
}

static WEBP_INLINE void PutLE16(uint8_t* const data, int val) {
  assert(val < (1 << 16));
  data[0] = (val >> 0) & 0xff;
  data[1] = (val >> 8) & 0xff;
}

static WEBP_INLINE void PutLE24(uint8_t* const data, int val) {
  assert(val < (1 << 24));
  PutLE16(data, val & 0xffff);
  data[2] = (val >> 16) & 0xff;
}

static WEBP_INLINE void PutLE32(uint8_t* const data, uint32_t val) {
  PutLE16(data, (int)(val & 0xffff));
  PutLE16(data + 2, (int)(val >> 16));
}

#define WEBP_NEED_LOG_TABLE_8BIT
extern const uint8_t WebPLogTable8bit[256];
static WEBP_INLINE int WebPLog2FloorC(uint32_t n) {
  int log_value = 0;
  while (n >= 256) {
    log_value += 8;
    n >>= 8;
  }
  return log_value + WebPLogTable8bit[n];
}

static WEBP_INLINE int BitsLog2Floor(uint32_t n) { return WebPLog2FloorC(n); }

struct WebPPicture;

WEBP_EXTERN void WebPCopyPlane(const uint8_t* src, int src_stride,
                               uint8_t* dst, int dst_stride,
                               int width, int height);

WEBP_EXTERN void WebPCopyPixels(const struct WebPPicture* const src,
                                struct WebPPicture* const dst);

WEBP_EXTERN int WebPGetColorPalette(const struct WebPPicture* const pic,
                                    uint32_t* const palette);

#ifdef __cplusplus
}
#endif

#endif
