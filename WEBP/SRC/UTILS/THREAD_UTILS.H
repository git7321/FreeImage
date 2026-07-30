// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Multi-threaded worker
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_UTILS_THREAD_UTILS_H_
#define WEBP_UTILS_THREAD_UTILS_H_

#include "src/webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  NOT_OK = 0,
  OK,
  WORK
} WebPWorkerStatus;

typedef int (*WebPWorkerHook)(void*, void*);

typedef struct {
  void* impl_;
  WebPWorkerStatus status_;
  WebPWorkerHook hook;
  void* data1;
  void* data2;
  int had_error;
} WebPWorker;

typedef struct {
  void (*Init)(WebPWorker* const worker);
  int (*Reset)(WebPWorker* const worker);
  int (*Sync)(WebPWorker* const worker);
  void (*Launch)(WebPWorker* const worker);
  void (*Execute)(WebPWorker* const worker);
  void (*End)(WebPWorker* const worker);
} WebPWorkerInterface;

WEBP_EXTERN int WebPSetWorkerInterface(
    const WebPWorkerInterface* const winterface);

WEBP_EXTERN const WebPWorkerInterface* WebPGetWorkerInterface(void);

#ifdef __cplusplus
}
#endif

#endif
