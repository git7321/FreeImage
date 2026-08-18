/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2015, Mathieu Malaterre <mathieu.malaterre@gmail.com>
 * Copyright (c) 2015, Matthieu Darbois
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#define OPJ_SKIP_POISON
#include "opj_includes.h"

# include <malloc.h>

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

static INLINE void *opj_aligned_alloc_n(size_t alignment, size_t size)
{
    void* ptr;

    /* alignment shall be power of 2 */
    assert((alignment != 0U) && ((alignment & (alignment - 1U)) == 0U));
    /* alignment shall be at least sizeof(void*) */
    assert(alignment >= sizeof(void*));

    if (size == 0U) { /* prevent implementation defined behavior of realloc */
        return NULL;
    }

    ptr = _aligned_malloc(size, alignment);
    return ptr;
}
static INLINE void *opj_aligned_realloc_n(void *ptr, size_t alignment,
        size_t new_size)
{
    void *r_ptr;

    /* alignment shall be power of 2 */
    assert((alignment != 0U) && ((alignment & (alignment - 1U)) == 0U));
    /* alignment shall be at least sizeof(void*) */
    assert(alignment >= sizeof(void*));

    if (new_size == 0U) { /* prevent implementation defined behavior of realloc */
        return NULL;
    }

    r_ptr = _aligned_realloc(ptr, new_size, alignment);
    return r_ptr;
}
void * opj_malloc(size_t size)
{
    if (size == 0U) { /* prevent implementation defined behavior of realloc */
        return NULL;
    }
    return malloc(size);
}
void * opj_calloc(size_t num, size_t size)
{
    if (num == 0 || size == 0) {
        /* prevent implementation defined behavior of realloc */
        return NULL;
    }
    return calloc(num, size);
}

void *opj_aligned_malloc(size_t size)
{
    return opj_aligned_alloc_n(16U, size);
}
void * opj_aligned_realloc(void *ptr, size_t size)
{
    return opj_aligned_realloc_n(ptr, 16U, size);
}

void *opj_aligned_32_malloc(size_t size)
{
    return opj_aligned_alloc_n(32U, size);
}
void * opj_aligned_32_realloc(void *ptr, size_t size)
{
    return opj_aligned_realloc_n(ptr, 32U, size);
}

void opj_aligned_free(void* ptr)
{
    _aligned_free(ptr);
}

void * opj_realloc(void *ptr, size_t new_size)
{
    if (new_size == 0U) { /* prevent implementation defined behavior of realloc */
        return NULL;
    }
    return realloc(ptr, new_size);
}
void opj_free(void *ptr)
{
    free(ptr);
}
