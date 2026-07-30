// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Utilities for building and looking up Huffman trees.
//
// Author: Urvang Joshi (urvang@google.com)

#ifndef WEBP_UTILS_HUFFMAN_UTILS_H_
#define WEBP_UTILS_HUFFMAN_UTILS_H_

#include <assert.h>
#include "src/webp/format_constants.h"
#include "src/webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HUFFMAN_TABLE_BITS      8
#define HUFFMAN_TABLE_MASK      ((1 << HUFFMAN_TABLE_BITS) - 1)

#define LENGTHS_TABLE_BITS      7
#define LENGTHS_TABLE_MASK      ((1 << LENGTHS_TABLE_BITS) - 1)

typedef struct {
  uint8_t bits;
  uint16_t value;
} HuffmanCode;

typedef struct {
  int bits;
  uint32_t value;
} HuffmanCode32;

#define HUFFMAN_PACKED_BITS 6
#define HUFFMAN_PACKED_TABLE_SIZE (1u << HUFFMAN_PACKED_BITS)

typedef struct HTreeGroup HTreeGroup;
struct HTreeGroup {
  HuffmanCode* htrees[HUFFMAN_CODES_PER_META_CODE];
  int      is_trivial_literal;
  uint32_t literal_arb;
  int is_trivial_code;
  int use_packed_table;
  HuffmanCode32 packed_table[HUFFMAN_PACKED_TABLE_SIZE];
};

HTreeGroup* VP8LHtreeGroupsNew(int num_htree_groups);

void VP8LHtreeGroupsFree(HTreeGroup* const htree_groups);
int VP8LBuildHuffmanTable(HuffmanCode* const root_table, int root_bits,
                          const int code_lengths[], int code_lengths_size);

#ifdef __cplusplus
}
#endif

#endif
