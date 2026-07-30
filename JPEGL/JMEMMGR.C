/*
 * jmemmgr.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2016, 2021-2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains the JPEG system-independent memory management
 * routines.  This code is usable across a wide variety of machines; most
 * of the system dependencies have been isolated in a separate file.
 * The major functions provided here are:
 *   * pool-based allocation and freeing of memory;
 *   * policy decisions about how to divide available memory among the
 *     virtual arrays;
 *   * control logic for swapping virtual arrays between main memory and
 *     backing storage.
 * The separate system-dependent file provides the actual backing-storage
 * access code, and it contains the policy decision about how much total
 * main memory to use.
 * This file is system-dependent in the sense that some of its functions
 * are unnecessary in some systems.  For example, if there is enough virtual
 * memory so that backing storage will never be used, much of the virtual
 * array control logic could be removed.  (Of course, if you have that much
 * memory then you shouldn't care about a little bit of unused code...)
 */

#define JPEG_INTERNALS
#define AM_MEMORY_MANAGER
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"
#if !defined(_MSC_VER) || _MSC_VER > 1600
#include <stdint.h>
#endif
#include <limits.h>

LOCAL(size_t)
round_up_pow2(size_t a, size_t b)
{
  return ((a + b - 1) & (~(b - 1)));
}

#ifndef ALIGN_SIZE              /* so can override from jconfig.h */
#ifndef WITH_SIMD
#define ALIGN_SIZE  MAX(sizeof(void *), sizeof(double))
#else
#define ALIGN_SIZE  32 /* Most of the SIMD instructions we support require
                          16-byte (128-bit) alignment, but AVX2 requires
                          32-byte alignment. */
#endif
#endif

typedef struct small_pool_struct *small_pool_ptr;

typedef struct small_pool_struct {
  small_pool_ptr next;          /* next in list of pools */
  size_t bytes_used;            /* how many bytes already used within pool */
  size_t bytes_left;            /* bytes still available in this pool */
} small_pool_hdr;

typedef struct large_pool_struct *large_pool_ptr;

typedef struct large_pool_struct {
  large_pool_ptr next;          /* next in list of pools */
  size_t bytes_used;            /* how many bytes already used within pool */
  size_t bytes_left;            /* bytes still available in this pool */
} large_pool_hdr;

typedef struct {
  struct jpeg_memory_mgr pub;   /* public fields */

  /* Each pool identifier (lifetime class) names a linked list of pools. */
  small_pool_ptr small_list[JPOOL_NUMPOOLS];
  large_pool_ptr large_list[JPOOL_NUMPOOLS];

  jvirt_sarray_ptr virt_sarray_list;
  jvirt_barray_ptr virt_barray_list;

  /* This counts total space obtained from jpeg_get_small/large */
  size_t total_space_allocated;

  JDIMENSION last_rowsperchunk; /* from most recent alloc_sarray/barray */
} my_memory_mgr;

typedef my_memory_mgr *my_mem_ptr;

struct jvirt_sarray_control {
  JSAMPARRAY mem_buffer;        /* => the in-memory buffer (if
                                   cinfo->data_precision is 12, then this is
                                   actually a J12SAMPARRAY) */
  JDIMENSION rows_in_array;     /* total virtual array height */
  JDIMENSION samplesperrow;     /* width of array (and of memory buffer) */
  JDIMENSION maxaccess;         /* max rows accessed by access_virt_sarray */
  JDIMENSION rows_in_mem;       /* height of memory buffer */
  JDIMENSION rowsperchunk;      /* allocation chunk size in mem_buffer */
  JDIMENSION cur_start_row;     /* first logical row # in the buffer */
  JDIMENSION first_undef_row;   /* row # of first uninitialized row */
  boolean pre_zero;             /* pre-zero mode requested? */
  boolean dirty;                /* do current buffer contents need written? */
  boolean b_s_open;             /* is backing-store data valid? */
  jvirt_sarray_ptr next;        /* link to next virtual sarray control block */
  backing_store_info b_s_info;  /* System-dependent control info */
};

struct jvirt_barray_control {
  JBLOCKARRAY mem_buffer;       /* => the in-memory buffer */
  JDIMENSION rows_in_array;     /* total virtual array height */
  JDIMENSION blocksperrow;      /* width of array (and of memory buffer) */
  JDIMENSION maxaccess;         /* max rows accessed by access_virt_barray */
  JDIMENSION rows_in_mem;       /* height of memory buffer */
  JDIMENSION rowsperchunk;      /* allocation chunk size in mem_buffer */
  JDIMENSION cur_start_row;     /* first logical row # in the buffer */
  JDIMENSION first_undef_row;   /* row # of first uninitialized row */
  boolean pre_zero;             /* pre-zero mode requested? */
  boolean dirty;                /* do current buffer contents need written? */
  boolean b_s_open;             /* is backing-store data valid? */
  jvirt_barray_ptr next;        /* link to next virtual barray control block */
  backing_store_info b_s_info;  /* System-dependent control info */
};

LOCAL(void)
out_of_memory(j_common_ptr cinfo, int which)
{
}

static const size_t first_pool_slop[JPOOL_NUMPOOLS] = {
  1600,                         /* first PERMANENT pool */
  16000                         /* first IMAGE pool */
};

static const size_t extra_pool_slop[JPOOL_NUMPOOLS] = {
  0,                            /* additional PERMANENT pools */
  5000                          /* additional IMAGE pools */
};

#define MIN_SLOP  50            /* greater than 0 to avoid futile looping */

METHODDEF(void *)
alloc_small(j_common_ptr cinfo, int pool_id, size_t sizeofobject)
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  small_pool_ptr hdr_ptr, prev_hdr_ptr;
  char *data_ptr;
  size_t min_request, slop;

  if (sizeofobject > MAX_ALLOC_CHUNK) {
    out_of_memory(cinfo, 7);
  }
  sizeofobject = round_up_pow2(sizeofobject, ALIGN_SIZE);

  /* Check for unsatisfiable request (do now to ensure no overflow below) */
  if ((sizeof(small_pool_hdr) + sizeofobject + ALIGN_SIZE - 1) >
      MAX_ALLOC_CHUNK)
    out_of_memory(cinfo, 1);    /* request exceeds malloc's ability */

  /* See if space is available in any existing pool */
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id); /* safety check */
  prev_hdr_ptr = NULL;
  hdr_ptr = mem->small_list[pool_id];
  while (hdr_ptr != NULL) {
    if (hdr_ptr->bytes_left >= sizeofobject)
      break;                    /* found pool with enough space */
    prev_hdr_ptr = hdr_ptr;
    hdr_ptr = hdr_ptr->next;
  }

  /* Time to make a new pool? */
  if (hdr_ptr == NULL) {
    /* min_request is what we need now, slop is what will be leftover */
    min_request = sizeof(small_pool_hdr) + sizeofobject + ALIGN_SIZE - 1;
    if (prev_hdr_ptr == NULL)   /* first pool in class? */
      slop = first_pool_slop[pool_id];
    else
      slop = extra_pool_slop[pool_id];
    /* Don't ask for more than MAX_ALLOC_CHUNK */
    if (slop > (size_t)(MAX_ALLOC_CHUNK - min_request))
      slop = (size_t)(MAX_ALLOC_CHUNK - min_request);
    /* Try to get space, if fail reduce slop and try again */
    for (;;) {
      hdr_ptr = (small_pool_ptr)jpeg_get_small(cinfo, min_request + slop);
      if (hdr_ptr != NULL)
        break;
      slop /= 2;
      if (slop < MIN_SLOP)      /* give up when it gets real small */
        out_of_memory(cinfo, 2); /* jpeg_get_small failed */
    }
    mem->total_space_allocated += min_request + slop;
    /* Success, initialize the new pool header and add to end of list */
    hdr_ptr->next = NULL;
    hdr_ptr->bytes_used = 0;
    hdr_ptr->bytes_left = sizeofobject + slop;
    if (prev_hdr_ptr == NULL)   /* first pool in class? */
      mem->small_list[pool_id] = hdr_ptr;
    else
      prev_hdr_ptr->next = hdr_ptr;
  }

  /* OK, allocate the object from the current pool */
  data_ptr = (char *)hdr_ptr; /* point to first data byte in pool... */
  data_ptr += sizeof(small_pool_hdr); /* ...by skipping the header... */
  if ((size_t)data_ptr % ALIGN_SIZE) /* ...and adjust for alignment */
    data_ptr += ALIGN_SIZE - (size_t)data_ptr % ALIGN_SIZE;
  data_ptr += hdr_ptr->bytes_used; /* point to place for object */
  hdr_ptr->bytes_used += sizeofobject;
  hdr_ptr->bytes_left -= sizeofobject;

  return (void *)data_ptr;
}

METHODDEF(void *)
alloc_large(j_common_ptr cinfo, int pool_id, size_t sizeofobject)
/* Allocate a "large" object */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  large_pool_ptr hdr_ptr;
  char *data_ptr;

  if (sizeofobject > MAX_ALLOC_CHUNK) {
    out_of_memory(cinfo, 8);
  }
  sizeofobject = round_up_pow2(sizeofobject, ALIGN_SIZE);

  /* Check for unsatisfiable request (do now to ensure no overflow below) */
  if ((sizeof(large_pool_hdr) + sizeofobject + ALIGN_SIZE - 1) >
      MAX_ALLOC_CHUNK)
    out_of_memory(cinfo, 3);    /* request exceeds malloc's ability */

  /* Always make a new pool */
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id); /* safety check */

  hdr_ptr = (large_pool_ptr)jpeg_get_large(cinfo, sizeofobject +
                                           sizeof(large_pool_hdr) +
                                           ALIGN_SIZE - 1);
  if (hdr_ptr == NULL)
    out_of_memory(cinfo, 4);    /* jpeg_get_large failed */
  mem->total_space_allocated += sizeofobject + sizeof(large_pool_hdr) +
                                ALIGN_SIZE - 1;

  /* Success, initialize the new pool header and add to list */
  hdr_ptr->next = mem->large_list[pool_id];
  hdr_ptr->bytes_used = sizeofobject;
  hdr_ptr->bytes_left = 0;
  mem->large_list[pool_id] = hdr_ptr;

  data_ptr = (char *)hdr_ptr; /* point to first data byte in pool... */
  data_ptr += sizeof(small_pool_hdr); /* ...by skipping the header... */
  if ((size_t)data_ptr % ALIGN_SIZE) /* ...and adjust for alignment */
    data_ptr += ALIGN_SIZE - (size_t)data_ptr % ALIGN_SIZE;

  return (void *)data_ptr;
}

METHODDEF(JSAMPARRAY)
alloc_sarray(j_common_ptr cinfo, int pool_id, JDIMENSION samplesperrow,
             JDIMENSION numrows)
/* Allocate a 2-D sample array */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  JSAMPARRAY result;
  JSAMPROW workspace;
  JDIMENSION rowsperchunk, currow, i;
  long ltemp;
  J12SAMPARRAY result12;
  J12SAMPROW workspace12;
#if defined(C_LOSSLESS_SUPPORTED) || defined(D_LOSSLESS_SUPPORTED)
  J16SAMPARRAY result16;
  J16SAMPROW workspace16;
#endif
  int data_precision = cinfo->is_decompressor ?
                        ((j_decompress_ptr)cinfo)->data_precision :
                        ((j_compress_ptr)cinfo)->data_precision;
  size_t sample_size = data_precision == 16 ?
                       sizeof(J16SAMPLE) : (data_precision == 12 ?
                                            sizeof(J12SAMPLE) :
                                            sizeof(JSAMPLE));

  /* Make sure each row is properly aligned */
  if ((ALIGN_SIZE % sample_size) != 0)
    out_of_memory(cinfo, 5);

  if (samplesperrow > MAX_ALLOC_CHUNK) {
    out_of_memory(cinfo, 9);
  }
  samplesperrow = (JDIMENSION)round_up_pow2(samplesperrow, (2 * ALIGN_SIZE) /
                                                           sample_size);

  /* Calculate max # of rows allowed in one allocation chunk */
  ltemp = (MAX_ALLOC_CHUNK - sizeof(large_pool_hdr)) /
          ((long)samplesperrow * (long)sample_size);
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if (ltemp < (long)numrows)
    rowsperchunk = (JDIMENSION)ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

  if (data_precision == 16) {
#if defined(C_LOSSLESS_SUPPORTED) || defined(D_LOSSLESS_SUPPORTED)
    /* Get space for row pointers (small object) */
    result16 = (J16SAMPARRAY)alloc_small(cinfo, pool_id,
                                         (size_t)(numrows *
                                                  sizeof(J16SAMPROW)));

    /* Get the rows themselves (large objects) */
    currow = 0;
    while (currow < numrows) {
      rowsperchunk = MIN(rowsperchunk, numrows - currow);
      workspace16 = (J16SAMPROW)alloc_large(cinfo, pool_id,
        (size_t)((size_t)rowsperchunk * (size_t)samplesperrow * sample_size));
      for (i = rowsperchunk; i > 0; i--) {
        result16[currow++] = workspace16;
        workspace16 += samplesperrow;
      }
    }

    return (JSAMPARRAY)result16;
#else
    return NULL;
#endif
  } else if (data_precision == 12) {
    /* Get space for row pointers (small object) */
    result12 = (J12SAMPARRAY)alloc_small(cinfo, pool_id,
                                         (size_t)(numrows *
                                                  sizeof(J12SAMPROW)));

    /* Get the rows themselves (large objects) */
    currow = 0;
    while (currow < numrows) {
      rowsperchunk = MIN(rowsperchunk, numrows - currow);
      workspace12 = (J12SAMPROW)alloc_large(cinfo, pool_id,
        (size_t)((size_t)rowsperchunk * (size_t)samplesperrow * sample_size));
      for (i = rowsperchunk; i > 0; i--) {
        result12[currow++] = workspace12;
        workspace12 += samplesperrow;
      }
    }

    return (JSAMPARRAY)result12;
  } else {
    /* Get space for row pointers (small object) */
    result = (JSAMPARRAY)alloc_small(cinfo, pool_id,
                                     (size_t)(numrows * sizeof(JSAMPROW)));

    /* Get the rows themselves (large objects) */
    currow = 0;
    while (currow < numrows) {
      rowsperchunk = MIN(rowsperchunk, numrows - currow);
      workspace = (JSAMPROW)alloc_large(cinfo, pool_id,
        (size_t)((size_t)rowsperchunk * (size_t)samplesperrow * sample_size));
      for (i = rowsperchunk; i > 0; i--) {
        result[currow++] = workspace;
        workspace += samplesperrow;
      }
    }

    return result;
  }
}

METHODDEF(JBLOCKARRAY)
alloc_barray(j_common_ptr cinfo, int pool_id, JDIMENSION blocksperrow,
             JDIMENSION numrows)
/* Allocate a 2-D coefficient-block array */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  JDIMENSION rowsperchunk, currow, i;
  long ltemp;

  /* Make sure each row is properly aligned */
  if ((sizeof(JBLOCK) % ALIGN_SIZE) != 0)
    out_of_memory(cinfo, 6);

  /* Calculate max # of rows allowed in one allocation chunk */
  ltemp = (MAX_ALLOC_CHUNK - sizeof(large_pool_hdr)) /
          ((long)blocksperrow * sizeof(JBLOCK));
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if (ltemp < (long)numrows)
    rowsperchunk = (JDIMENSION)ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

  /* Get space for row pointers (small object) */
  result = (JBLOCKARRAY)alloc_small(cinfo, pool_id,
                                    (size_t)(numrows * sizeof(JBLOCKROW)));

  /* Get the rows themselves (large objects) */
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = (JBLOCKROW)alloc_large(cinfo, pool_id,
        (size_t)((size_t)rowsperchunk * (size_t)blocksperrow *
                  sizeof(JBLOCK)));
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += blocksperrow;
    }
  }

  return result;
}

METHODDEF(jvirt_sarray_ptr)
request_virt_sarray(j_common_ptr cinfo, int pool_id, boolean pre_zero,
                    JDIMENSION samplesperrow, JDIMENSION numrows,
                    JDIMENSION maxaccess)
/* Request a virtual 2-D sample array */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  jvirt_sarray_ptr result;

  /* Only IMAGE-lifetime virtual arrays are currently supported */
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);

  /* get control block */
  result = (jvirt_sarray_ptr)alloc_small(cinfo, pool_id,
                                         sizeof(struct jvirt_sarray_control));

  result->mem_buffer = NULL;
  result->rows_in_array = numrows;
  result->samplesperrow = samplesperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;
  result->next = mem->virt_sarray_list;
  mem->virt_sarray_list = result;

  return result;
}

METHODDEF(jvirt_barray_ptr)
request_virt_barray(j_common_ptr cinfo, int pool_id, boolean pre_zero,
                    JDIMENSION blocksperrow, JDIMENSION numrows,
                    JDIMENSION maxaccess)
/* Request a virtual 2-D coefficient-block array */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  jvirt_barray_ptr result;

  /* Only IMAGE-lifetime virtual arrays are currently supported */
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);

  /* get control block */
  result = (jvirt_barray_ptr)alloc_small(cinfo, pool_id,
                                         sizeof(struct jvirt_barray_control));

  result->mem_buffer = NULL;
  result->rows_in_array = numrows;
  result->blocksperrow = blocksperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;
  result->next = mem->virt_barray_list;
  mem->virt_barray_list = result;

  return result;
}

METHODDEF(void)
realize_virt_arrays(j_common_ptr cinfo)
/* Allocate the in-memory buffers for any unrealized virtual arrays */
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  size_t space_per_minheight, maximum_space, avail_mem;
  size_t minheights, max_minheights;
  jvirt_sarray_ptr sptr;
  jvirt_barray_ptr bptr;
  int data_precision = cinfo->is_decompressor ?
                        ((j_decompress_ptr)cinfo)->data_precision :
                        ((j_compress_ptr)cinfo)->data_precision;
  size_t sample_size = data_precision == 16 ?
                       sizeof(J16SAMPLE) : (data_precision == 12 ?
                                            sizeof(J12SAMPLE) :
                                            sizeof(JSAMPLE));

  space_per_minheight = 0;
  maximum_space = 0;
  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) {
      size_t new_space = (long)sptr->rows_in_array *
                         (long)sptr->samplesperrow * sample_size;

      space_per_minheight += (long)sptr->maxaccess *
                             (long)sptr->samplesperrow * sample_size;
      if (SIZE_MAX - maximum_space < new_space)
        out_of_memory(cinfo, 10);
      maximum_space += new_space;
    }
  }
  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) {
      size_t new_space = (long)bptr->rows_in_array *
                         (long)bptr->blocksperrow * sizeof(JBLOCK);

      space_per_minheight += (long)bptr->maxaccess *
                             (long)bptr->blocksperrow * sizeof(JBLOCK);
      if (SIZE_MAX - maximum_space < new_space)
        out_of_memory(cinfo, 11);
      maximum_space += new_space;
    }
  }

  if (space_per_minheight <= 0)
    return;

  /* Determine amount of memory to actually use; this is system-dependent. */
  avail_mem = jpeg_mem_available(cinfo, space_per_minheight, maximum_space,
                                 mem->total_space_allocated);

  if (avail_mem >= maximum_space)
    max_minheights = 1000000000L;
  else {
    max_minheights = avail_mem / space_per_minheight;
    if (max_minheights <= 0)
      max_minheights = 1;
  }

  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) {
      minheights = ((long)sptr->rows_in_array - 1L) / sptr->maxaccess + 1L;
      if (minheights <= max_minheights) {
        sptr->rows_in_mem = sptr->rows_in_array;
      } else {
        sptr->rows_in_mem = (JDIMENSION)(max_minheights * sptr->maxaccess);
        jpeg_open_backing_store(cinfo, &sptr->b_s_info,
                                (long)sptr->rows_in_array *
                                (long)sptr->samplesperrow *
                                (long)sample_size);
        sptr->b_s_open = TRUE;
      }
      sptr->mem_buffer = alloc_sarray(cinfo, JPOOL_IMAGE,
                                      sptr->samplesperrow, sptr->rows_in_mem);
      sptr->rowsperchunk = mem->last_rowsperchunk;
      sptr->cur_start_row = 0;
      sptr->first_undef_row = 0;
      sptr->dirty = FALSE;
    }
  }

  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) {
      minheights = ((long)bptr->rows_in_array - 1L) / bptr->maxaccess + 1L;
      if (minheights <= max_minheights) {
        /* This buffer fits in memory */
        bptr->rows_in_mem = bptr->rows_in_array;
      } else {
        /* It doesn't fit in memory, create backing store. */
        bptr->rows_in_mem = (JDIMENSION)(max_minheights * bptr->maxaccess);
        jpeg_open_backing_store(cinfo, &bptr->b_s_info,
                                (long)bptr->rows_in_array *
                                (long)bptr->blocksperrow *
                                (long)sizeof(JBLOCK));
        bptr->b_s_open = TRUE;
      }
      bptr->mem_buffer = alloc_barray(cinfo, JPOOL_IMAGE,
                                      bptr->blocksperrow, bptr->rows_in_mem);
      bptr->rowsperchunk = mem->last_rowsperchunk;
      bptr->cur_start_row = 0;
      bptr->first_undef_row = 0;
      bptr->dirty = FALSE;
    }
  }
}

LOCAL(void)
do_sarray_io(j_common_ptr cinfo, jvirt_sarray_ptr ptr, boolean writing)
/* Do backing store read or write of a virtual sample array */
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;
  int data_precision = cinfo->is_decompressor ?
                        ((j_decompress_ptr)cinfo)->data_precision :
                        ((j_compress_ptr)cinfo)->data_precision;
  size_t sample_size = data_precision == 16 ?
                       sizeof(J16SAMPLE) : (data_precision == 12 ?
                                            sizeof(J12SAMPLE) :
                                            sizeof(JSAMPLE));

  bytesperrow = (long)ptr->samplesperrow * (long)sample_size;
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < (long)ptr->rows_in_mem; i += ptr->rowsperchunk) {
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN((long)ptr->rowsperchunk, (long)ptr->rows_in_mem - i);
    /* Transfer no more than is currently defined */
    thisrow = (long)ptr->cur_start_row + i;
    rows = MIN(rows, (long)ptr->first_undef_row - thisrow);
    /* Transfer no more than fits in file */
    rows = MIN(rows, (long)ptr->rows_in_array - thisrow);
    if (rows <= 0)
      break;
    byte_count = rows * bytesperrow;
    if (data_precision == 16) {
#if defined(C_LOSSLESS_SUPPORTED) || defined(D_LOSSLESS_SUPPORTED)
      J16SAMPARRAY mem_buffer16 = (J16SAMPARRAY)ptr->mem_buffer;

      if (writing)
        (*ptr->b_s_info.write_backing_store) (cinfo, &ptr->b_s_info,
                                              (void *)mem_buffer16[i],
                                              file_offset, byte_count);
      else
        (*ptr->b_s_info.read_backing_store) (cinfo, &ptr->b_s_info,
                                             (void *)mem_buffer16[i],
                                             file_offset, byte_count);
#else
      ERREXIT1(cinfo, JERR_BAD_PRECISION, data_precision);
#endif
    } else if (data_precision == 12) {
      J12SAMPARRAY mem_buffer12 = (J12SAMPARRAY)ptr->mem_buffer;

      if (writing)
        (*ptr->b_s_info.write_backing_store) (cinfo, &ptr->b_s_info,
                                              (void *)mem_buffer12[i],
                                              file_offset, byte_count);
      else
        (*ptr->b_s_info.read_backing_store) (cinfo, &ptr->b_s_info,
                                             (void *)mem_buffer12[i],
                                             file_offset, byte_count);
    } else {
      if (writing)
        (*ptr->b_s_info.write_backing_store) (cinfo, &ptr->b_s_info,
                                              (void *)ptr->mem_buffer[i],
                                              file_offset, byte_count);
      else
        (*ptr->b_s_info.read_backing_store) (cinfo, &ptr->b_s_info,
                                             (void *)ptr->mem_buffer[i],
                                             file_offset, byte_count);
    }
    file_offset += byte_count;
  }
}

LOCAL(void)
do_barray_io(j_common_ptr cinfo, jvirt_barray_ptr ptr, boolean writing)
/* Do backing store read or write of a virtual coefficient-block array */
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;

  bytesperrow = (long)ptr->blocksperrow * sizeof(JBLOCK);
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < (long)ptr->rows_in_mem; i += ptr->rowsperchunk) {
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN((long)ptr->rowsperchunk, (long)ptr->rows_in_mem - i);
    /* Transfer no more than is currently defined */
    thisrow = (long)ptr->cur_start_row + i;
    rows = MIN(rows, (long)ptr->first_undef_row - thisrow);
    /* Transfer no more than fits in file */
    rows = MIN(rows, (long)ptr->rows_in_array - thisrow);
    if (rows <= 0)              /* this chunk might be past end of file! */
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      (*ptr->b_s_info.write_backing_store) (cinfo, &ptr->b_s_info,
                                            (void *)ptr->mem_buffer[i],
                                            file_offset, byte_count);
    else
      (*ptr->b_s_info.read_backing_store) (cinfo, &ptr->b_s_info,
                                           (void *)ptr->mem_buffer[i],
                                           file_offset, byte_count);
    file_offset += byte_count;
  }
}

METHODDEF(JSAMPARRAY)
access_virt_sarray(j_common_ptr cinfo, jvirt_sarray_ptr ptr,
                   JDIMENSION start_row, JDIMENSION num_rows, boolean writable)
{
  JDIMENSION end_row = start_row + num_rows;
  JDIMENSION undef_row;
  int data_precision = cinfo->is_decompressor ?
                        ((j_decompress_ptr)cinfo)->data_precision :
                        ((j_compress_ptr)cinfo)->data_precision;
  size_t sample_size = data_precision == 16 ?
                       sizeof(J16SAMPLE) : (data_precision == 12 ?
                                            sizeof(J12SAMPLE) :
                                            sizeof(JSAMPLE));

  /* debugging check */
  if (end_row > ptr->rows_in_array || num_rows > ptr->maxaccess ||
      ptr->mem_buffer == NULL)
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

  /* Make the desired part of the virtual array accessible */
  if (start_row < ptr->cur_start_row ||
      end_row > ptr->cur_start_row + ptr->rows_in_mem) {
    if (!ptr->b_s_open)
      ERREXIT(cinfo, JERR_VIRTUAL_BUG);
    /* Flush old buffer contents if necessary */
    if (ptr->dirty) {
      do_sarray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
      /* use long arithmetic here to avoid overflow & unsigned problems */
      long ltemp;

      ltemp = (long)end_row - (long)ptr->rows_in_mem;
      if (ltemp < 0)
        ltemp = 0;              /* don't fall off front end of file */
      ptr->cur_start_row = (JDIMENSION)ltemp;
    }
    do_sarray_io(cinfo, ptr, FALSE);
  }
  if (ptr->first_undef_row < end_row) {
    if (ptr->first_undef_row < start_row) {
      if (writable)             /* writer skipped over a section of array */
      undef_row = start_row;    /* but reader is allowed to read ahead */
    } else {
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) {
      size_t bytesperrow = (size_t)ptr->samplesperrow * sample_size;
      undef_row -= ptr->cur_start_row;
      end_row -= ptr->cur_start_row;
      while (undef_row < end_row) {
        jzero_far((void *)ptr->mem_buffer[undef_row], bytesperrow);
        undef_row++;
      }
    } else {
      if (!writable)            /* reader looking at undefined data */
        ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}

METHODDEF(JBLOCKARRAY)
access_virt_barray(j_common_ptr cinfo, jvirt_barray_ptr ptr,
                   JDIMENSION start_row, JDIMENSION num_rows, boolean writable)
{
  JDIMENSION end_row = start_row + num_rows;
  JDIMENSION undef_row;

  /* debugging check */
  if (end_row > ptr->rows_in_array || num_rows > ptr->maxaccess ||
      ptr->mem_buffer == NULL)
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

  /* Make the desired part of the virtual array accessible */
  if (start_row < ptr->cur_start_row ||
      end_row > ptr->cur_start_row + ptr->rows_in_mem) {
    if (!ptr->b_s_open)
      ERREXIT(cinfo, JERR_VIRTUAL_BUG);
    /* Flush old buffer contents if necessary */
    if (ptr->dirty) {
      do_barray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
      /* use long arithmetic here to avoid overflow & unsigned problems */
      long ltemp;

      ltemp = (long)end_row - (long)ptr->rows_in_mem;
      if (ltemp < 0)
        ltemp = 0;              /* don't fall off front end of file */
      ptr->cur_start_row = (JDIMENSION)ltemp;
    }
    do_barray_io(cinfo, ptr, FALSE);
  }
  if (ptr->first_undef_row < end_row) {
    if (ptr->first_undef_row < start_row) {
      if (writable)             /* writer skipped over a section of array */
      undef_row = start_row;    /* but reader is allowed to read ahead */
    } else {
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) {
      size_t bytesperrow = (size_t)ptr->blocksperrow * sizeof(JBLOCK);
      undef_row -= ptr->cur_start_row; /* make indexes relative to buffer */
      end_row -= ptr->cur_start_row;
      while (undef_row < end_row) {
        jzero_far((void *)ptr->mem_buffer[undef_row], bytesperrow);
        undef_row++;
      }
    } else {
      if (!writable)            /* reader looking at undefined data */
        ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}

METHODDEF(void)
free_pool(j_common_ptr cinfo, int pool_id)
{
  my_mem_ptr mem = (my_mem_ptr)cinfo->mem;
  small_pool_ptr shdr_ptr;
  large_pool_ptr lhdr_ptr;
  size_t space_freed;

  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);

  /* If freeing IMAGE pool, close any virtual arrays first */
  if (pool_id == JPOOL_IMAGE) {
    jvirt_sarray_ptr sptr;
    jvirt_barray_ptr bptr;

    for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
      if (sptr->b_s_open) {     /* there may be no backing store */
        sptr->b_s_open = FALSE; /* prevent recursive close if error */
        (*sptr->b_s_info.close_backing_store) (cinfo, &sptr->b_s_info);
      }
    }
    mem->virt_sarray_list = NULL;
    for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
      if (bptr->b_s_open) {     /* there may be no backing store */
        bptr->b_s_open = FALSE; /* prevent recursive close if error */
        (*bptr->b_s_info.close_backing_store) (cinfo, &bptr->b_s_info);
      }
    }
    mem->virt_barray_list = NULL;
  }

  /* Release large objects */
  lhdr_ptr = mem->large_list[pool_id];
  mem->large_list[pool_id] = NULL;

  while (lhdr_ptr != NULL) {
    large_pool_ptr next_lhdr_ptr = lhdr_ptr->next;
    space_freed = lhdr_ptr->bytes_used +
                  lhdr_ptr->bytes_left +
                  sizeof(large_pool_hdr) + ALIGN_SIZE - 1;
    jpeg_free_large(cinfo, (void *)lhdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    lhdr_ptr = next_lhdr_ptr;
  }

  /* Release small objects */
  shdr_ptr = mem->small_list[pool_id];
  mem->small_list[pool_id] = NULL;

  while (shdr_ptr != NULL) {
    small_pool_ptr next_shdr_ptr = shdr_ptr->next;
    space_freed = shdr_ptr->bytes_used + shdr_ptr->bytes_left +
                  sizeof(small_pool_hdr) + ALIGN_SIZE - 1;
    jpeg_free_small(cinfo, (void *)shdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    shdr_ptr = next_shdr_ptr;
  }
}

METHODDEF(void)
self_destruct(j_common_ptr cinfo)
{
  int pool;

  for (pool = JPOOL_NUMPOOLS - 1; pool >= JPOOL_PERMANENT; pool--) {
    free_pool(cinfo, pool);
  }

  /* Release the memory manager control block too. */
  jpeg_free_small(cinfo, (void *)cinfo->mem, sizeof(my_memory_mgr));
  cinfo->mem = NULL;            /* ensures I will be called only once */

  jpeg_mem_term(cinfo);         /* system-dependent cleanup */
}

GLOBAL(void)
jinit_memory_mgr(j_common_ptr cinfo)
{
  my_mem_ptr mem;
  long max_to_use;
  int pool;
  size_t test_mac;

  cinfo->mem = NULL;

  if ((ALIGN_SIZE & (ALIGN_SIZE - 1)) != 0)
    ERREXIT(cinfo, JERR_BAD_ALIGN_TYPE);
  test_mac = (size_t)MAX_ALLOC_CHUNK;
  if ((long)test_mac != MAX_ALLOC_CHUNK ||
      (MAX_ALLOC_CHUNK % ALIGN_SIZE) != 0)
    ERREXIT(cinfo, JERR_BAD_ALLOC_CHUNK);

  max_to_use = jpeg_mem_init(cinfo);

  /* Attempt to allocate memory manager's control block */
  mem = (my_mem_ptr)jpeg_get_small(cinfo, sizeof(my_memory_mgr));

  if (mem == NULL) {
    jpeg_mem_term(cinfo);
  }

  /* OK, fill in the method pointers */
  mem->pub.alloc_small = alloc_small;
  mem->pub.alloc_large = alloc_large;
  mem->pub.alloc_sarray = alloc_sarray;
  mem->pub.alloc_barray = alloc_barray;
  mem->pub.request_virt_sarray = request_virt_sarray;
  mem->pub.request_virt_barray = request_virt_barray;
  mem->pub.realize_virt_arrays = realize_virt_arrays;
  mem->pub.access_virt_sarray = access_virt_sarray;
  mem->pub.access_virt_barray = access_virt_barray;
  mem->pub.free_pool = free_pool;
  mem->pub.self_destruct = self_destruct;

  /* Make MAX_ALLOC_CHUNK accessible to other modules */
  mem->pub.max_alloc_chunk = MAX_ALLOC_CHUNK;

  /* Initialize working state */
  mem->pub.max_memory_to_use = max_to_use;

  for (pool = JPOOL_NUMPOOLS - 1; pool >= JPOOL_PERMANENT; pool--) {
    mem->small_list[pool] = NULL;
    mem->large_list[pool] = NULL;
  }
  mem->virt_sarray_list = NULL;
  mem->virt_barray_list = NULL;

  mem->total_space_allocated = sizeof(my_memory_mgr);

  /* Declare ourselves open for business */
  cinfo->mem = &mem->pub;
}
