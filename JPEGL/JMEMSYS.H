/*
 * jmemsys.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1992-1997, Thomas G. Lane.
 * It was modified by The libjpeg-turbo Project to include only code and
 * information relevant to libjpeg-turbo.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This include file defines the interface between the system-independent
 * and system-dependent portions of the JPEG memory manager.  No other
 * modules need include it.  (The system-independent portion is jmemmgr.c;
 * there are several different versions of the system-dependent portion.)
 *
 * This file works as-is for the system-dependent memory managers supplied
 * in the IJG distribution.  You may need to modify it if you write a
 * custom memory manager.  If system-dependent changes are needed in
 * this file, the best method is to #ifdef them based on a configuration
 * symbol supplied in jconfig.h.
 */

EXTERN(void *) jpeg_get_small(j_common_ptr cinfo, size_t sizeofobject);
EXTERN(void) jpeg_free_small(j_common_ptr cinfo, void *object,
                             size_t sizeofobject);

EXTERN(void *) jpeg_get_large(j_common_ptr cinfo, size_t sizeofobject);
EXTERN(void) jpeg_free_large(j_common_ptr cinfo, void *object,
                             size_t sizeofobject);

#ifndef MAX_ALLOC_CHUNK         /* may be overridden in jconfig.h */
#define MAX_ALLOC_CHUNK  1000000000L
#endif

EXTERN(size_t) jpeg_mem_available(j_common_ptr cinfo, size_t min_bytes_needed,
                                  size_t max_bytes_needed,
                                  size_t already_allocated);

#define TEMP_NAME_LENGTH   64   /* max length of a temporary file's name */


#ifdef USE_MSDOS_MEMMGR         /* DOS-specific junk */

typedef unsigned short XMSH;    /* type of extended-memory handles */
typedef unsigned short EMSH;    /* type of expanded-memory handles */

typedef union {
  short file_handle;            /* DOS file handle if it's a temp file */
  XMSH xms_handle;              /* handle if it's a chunk of XMS */
  EMSH ems_handle;              /* handle if it's a chunk of EMS */
} handle_union;

#endif /* USE_MSDOS_MEMMGR */

#ifdef USE_MAC_MEMMGR           /* Mac-specific junk */
#include <Files.h>
#endif /* USE_MAC_MEMMGR */

typedef struct backing_store_struct *backing_store_ptr;

typedef struct backing_store_struct {
  /* Methods for reading/writing/closing this backing-store object */
  void (*read_backing_store) (j_common_ptr cinfo, backing_store_ptr info,
                              void *buffer_address, long file_offset,
                              long byte_count);
  void (*write_backing_store) (j_common_ptr cinfo, backing_store_ptr info,
                               void *buffer_address, long file_offset,
                               long byte_count);
  void (*close_backing_store) (j_common_ptr cinfo, backing_store_ptr info);

  /* Private fields for system-dependent backing-store management */
#ifdef USE_MSDOS_MEMMGR
  /* For the MS-DOS manager (jmemdos.c), we need: */
  handle_union handle;          /* reference to backing-store storage object */
  char temp_name[TEMP_NAME_LENGTH]; /* name if it's a file */
#else
#ifdef USE_MAC_MEMMGR
  /* For the Mac manager (jmemmac.c), we need: */
  short temp_file;              /* file reference number to temp file */
  FSSpec tempSpec;              /* the FSSpec for the temp file */
  char temp_name[TEMP_NAME_LENGTH]; /* name if it's a file */
#else
  /* For a typical implementation with temp files, we need: */
  FILE *temp_file;              /* stdio reference to temp file */
  char temp_name[TEMP_NAME_LENGTH]; /* name of temp file */
#endif
#endif
} backing_store_info;

EXTERN(void) jpeg_open_backing_store(j_common_ptr cinfo,
                                     backing_store_ptr info,
                                     long total_bytes_needed);

EXTERN(long) jpeg_mem_init(j_common_ptr cinfo);
EXTERN(void) jpeg_mem_term(j_common_ptr cinfo);
