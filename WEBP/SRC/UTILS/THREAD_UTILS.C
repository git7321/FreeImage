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

#include <assert.h>
#include <string.h>
#include "src/utils/thread_utils.h"
#include "src/utils/utils.h"

#if defined(_WIN32)

#include <windows.h>
typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;

typedef struct {
  HANDLE waiting_sem_;
  HANDLE received_sem_;
  HANDLE signal_event_;
} pthread_cond_t;

#ifndef WINAPI_FAMILY_PARTITION
#define WINAPI_PARTITION_DESKTOP 1
#define WINAPI_FAMILY_PARTITION(x) x
#endif

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define USE_CREATE_THREAD
#endif

#else

#include <pthread.h>

#endif

typedef struct {
  pthread_mutex_t mutex_;
  pthread_cond_t  condition_;
  pthread_t       thread_;
} WebPWorkerImpl;

#if defined(_WIN32)

#include <process.h>

#define THREADFN unsigned int __stdcall
#define THREAD_RETURN(val) (unsigned int)((DWORD_PTR)val)

#if _WIN32_WINNT >= 0x0501
#define WaitForSingleObject(obj, timeout) \
  WaitForSingleObjectEx(obj, timeout, FALSE)
#endif

static int pthread_create(pthread_t* const thread, const void* attr,
                          unsigned int (__stdcall* start)(void*), void* arg) {
  (void)attr;
  *thread = CreateThread(NULL,
                         0,
                         start,
                         arg,
                         0,
                         NULL);
  if (*thread == NULL) return 1;
  SetThreadPriority(*thread, THREAD_PRIORITY_ABOVE_NORMAL);
  return 0;
}

static int pthread_join(pthread_t thread, void** value_ptr) {
  (void)value_ptr;
  return (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0 ||
          CloseHandle(thread) == 0);
}

// Mutex
static int pthread_mutex_init(pthread_mutex_t* const mutex, void* mutexattr) {
  (void)mutexattr;
  InitializeCriticalSection(mutex);
  return 0;
}

static int pthread_mutex_lock(pthread_mutex_t* const mutex) {
  EnterCriticalSection(mutex);
  return 0;
}

static int pthread_mutex_unlock(pthread_mutex_t* const mutex) {
  LeaveCriticalSection(mutex);
  return 0;
}

static int pthread_mutex_destroy(pthread_mutex_t* const mutex) {
  DeleteCriticalSection(mutex);
  return 0;
}

// Condition
static int pthread_cond_destroy(pthread_cond_t* const condition) {
  int ok = 1;
  ok &= (CloseHandle(condition->waiting_sem_) != 0);
  ok &= (CloseHandle(condition->received_sem_) != 0);
  ok &= (CloseHandle(condition->signal_event_) != 0);
  return !ok;
}

static int pthread_cond_init(pthread_cond_t* const condition, void* cond_attr) {
  (void)cond_attr;
  condition->waiting_sem_ = CreateSemaphore(NULL, 0, 1, NULL);
  condition->received_sem_ = CreateSemaphore(NULL, 0, 1, NULL);
  condition->signal_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (condition->waiting_sem_ == NULL ||
      condition->received_sem_ == NULL ||
      condition->signal_event_ == NULL) {
    pthread_cond_destroy(condition);
    return 1;
  }
  return 0;
}

static int pthread_cond_signal(pthread_cond_t* const condition) {
  int ok = 1;
  if (WaitForSingleObject(condition->waiting_sem_, 0) == WAIT_OBJECT_0) {
    ok = SetEvent(condition->signal_event_);
    ok &= (WaitForSingleObject(condition->received_sem_, INFINITE) !=
           WAIT_OBJECT_0);
  }
  return !ok;
}

static int pthread_cond_wait(pthread_cond_t* const condition,
                             pthread_mutex_t* const mutex) {
  int ok;
  if (!ReleaseSemaphore(condition->waiting_sem_, 1, NULL)) return 1;
  pthread_mutex_unlock(mutex);
  ok = (WaitForSingleObject(condition->signal_event_, INFINITE) ==
        WAIT_OBJECT_0);
  ok &= ReleaseSemaphore(condition->received_sem_, 1, NULL);
  pthread_mutex_lock(mutex);
  return !ok;
}

#else
# define THREADFN void*
# define THREAD_RETURN(val) val
#endif

static THREADFN ThreadLoop(void* ptr) {
  WebPWorker* const worker = (WebPWorker*)ptr;
  WebPWorkerImpl* const impl = (WebPWorkerImpl*)worker->impl_;
  int done = 0;
  while (!done) {
    pthread_mutex_lock(&impl->mutex_);
    while (worker->status_ == OK) {
      pthread_cond_wait(&impl->condition_, &impl->mutex_);
    }
    if (worker->status_ == WORK) {
      WebPGetWorkerInterface()->Execute(worker);
      worker->status_ = OK;
    } else if (worker->status_ == NOT_OK) {
      done = 1;
    }
    pthread_mutex_unlock(&impl->mutex_);
    pthread_cond_signal(&impl->condition_);
  }
  return THREAD_RETURN(NULL);
}

static void ChangeState(WebPWorker* const worker, WebPWorkerStatus new_status) {
  WebPWorkerImpl* const impl = (WebPWorkerImpl*)worker->impl_;
  if (impl == NULL) return;

  pthread_mutex_lock(&impl->mutex_);
  if (worker->status_ >= OK) {
    while (worker->status_ != OK) {
      pthread_cond_wait(&impl->condition_, &impl->mutex_);
    }
    if (new_status != OK) {
      worker->status_ = new_status;
      pthread_mutex_unlock(&impl->mutex_);
      pthread_cond_signal(&impl->condition_);
      return;
    }
  }
  pthread_mutex_unlock(&impl->mutex_);
}

static void Init(WebPWorker* const worker) {
  memset(worker, 0, sizeof(*worker));
  worker->status_ = NOT_OK;
}

static int Sync(WebPWorker* const worker) {
  ChangeState(worker, OK);
  assert(worker->status_ <= OK);
  return !worker->had_error;
}

static int Reset(WebPWorker* const worker) {
  int ok = 1;
  worker->had_error = 0;
  if (worker->status_ < OK) {
    WebPWorkerImpl* const impl =
        (WebPWorkerImpl*)WebPSafeCalloc(1, sizeof(WebPWorkerImpl));
    worker->impl_ = (void*)impl;
    if (worker->impl_ == NULL) {
      return 0;
    }
    if (pthread_mutex_init(&impl->mutex_, NULL)) {
      goto Error;
    }
    if (pthread_cond_init(&impl->condition_, NULL)) {
      pthread_mutex_destroy(&impl->mutex_);
      goto Error;
    }
    pthread_mutex_lock(&impl->mutex_);
    ok = !pthread_create(&impl->thread_, NULL, ThreadLoop, worker);
    if (ok) worker->status_ = OK;
    pthread_mutex_unlock(&impl->mutex_);
    if (!ok) {
      pthread_mutex_destroy(&impl->mutex_);
      pthread_cond_destroy(&impl->condition_);
 Error:
      WebPSafeFree(impl);
      worker->impl_ = NULL;
      return 0;
    }
  } else if (worker->status_ > OK) {
    ok = Sync(worker);
  }
  assert(!ok || (worker->status_ == OK));
  return ok;
}

static void Execute(WebPWorker* const worker) {
  if (worker->hook != NULL) {
    worker->had_error |= !worker->hook(worker->data1, worker->data2);
  }
}

static void Launch(WebPWorker* const worker) {
  ChangeState(worker, WORK);
}

static void End(WebPWorker* const worker) {
  if (worker->impl_ != NULL) {
    WebPWorkerImpl* const impl = (WebPWorkerImpl*)worker->impl_;
    ChangeState(worker, NOT_OK);
    pthread_join(impl->thread_, NULL);
    pthread_mutex_destroy(&impl->mutex_);
    pthread_cond_destroy(&impl->condition_);
    WebPSafeFree(impl);
    worker->impl_ = NULL;
  }
  assert(worker->status_ == NOT_OK);
}

static WebPWorkerInterface g_worker_interface = {
  Init, Reset, Sync, Launch, Execute, End
};

int WebPSetWorkerInterface(const WebPWorkerInterface* const winterface) {
  if (winterface == NULL ||
      winterface->Init == NULL || winterface->Reset == NULL ||
      winterface->Sync == NULL || winterface->Launch == NULL ||
      winterface->Execute == NULL || winterface->End == NULL) {
    return 0;
  }
  g_worker_interface = *winterface;
  return 1;
}

const WebPWorkerInterface* WebPGetWorkerInterface(void) {
  return &g_worker_interface;
}
