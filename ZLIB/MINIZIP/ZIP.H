/* zip.h -- IO for compress .zip files using zlib
   Version 1.01e
   Copyright (C) Gilles Vollant
*/

#ifndef _zip_H
#define _zip_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zlib.h"
#include "ioapi.h"

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagzipFile__ { int unused; } zipFile__;
typedef zipFile__ *zipFile;
#else
typedef voidp zipFile;
#endif

#define ZIP_OK            (0)
#define ZIP_EOF           (0)
#define ZIP_ERRNO         (Z_ERRNO)
#define ZIP_PARAMERROR    (-102)
#define ZIP_BADZIPFILE    (-103)
#define ZIP_INTERNALERROR (-104)

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL MAX_MEM_LEVEL
#  endif
#endif
/* default memLevel */

/* tm_zip contain date/time info */
typedef struct tm_zip_s
{
    uInt tm_sec;        /* seconds after the minute - [0,59] */
    uInt tm_min;        /* minutes after the hour - [0,59] */
    uInt tm_hour;       /* hours since midnight - [0,23] */
    uInt tm_mday;       /* day of the month - [1,31] */
    uInt tm_mon;        /* months since January - [0,11] */
    uInt tm_year;       /* years - [1980..2044] */
} tm_zip;

typedef struct
{
    tm_zip tmz_date;    /* date in understandable format      */
    uLong  dosDate;     /* if dos_date == 0, tmu_date is used */

    uLong  internal_fa; /* internal file attributes   2 bytes */
    uLong  external_fa; /* external file attributes   4 bytes */
} zip_fileinfo;

typedef const char* zipcharpc;

#define APPEND_STATUS_CREATE        (0)
#define APPEND_STATUS_CREATEAFTER   (1)
#define APPEND_STATUS_ADDINZIP      (2)

ZEXTERN zipFile ZEXPORT zipOpen (const char *pathname, int append);
/*
  Create a zipfile.
     pathname contain on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
       an Unix computer "zlib/zlib113.zip".
     if the file pathname exist and append==APPEND_STATUS_CREATEAFTER, the zip
       will be created at the end of the file.
         (useful if the file contain a self extractor code)
     if the file pathname exist and append==APPEND_STATUS_ADDINZIP, we will
       add files in existing zip (be sure you don't add file that doesn't exist)
     If the zipfile cannot be opened, the return value is NULL.
     Else, the return value is a zipFile Handle, usable with other function
       of this zip package.
*/

/* Note : there is no delete function into a zipfile.
   If you want delete file into a zipfile, you must open a zipfile, and create another
   Of couse, you can use RAW reading and writing to copy the file you did not want delte
*/

ZEXTERN zipFile ZEXPORT zipOpen2 (const char *pathname,
                                   int append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc_def* pzlib_filefunc_def);

ZEXTERN int ZEXPORT zipOpenNewFileInZip (zipFile file,
                       const char* filename,
                       const zip_fileinfo* zipfi,
                       const void* extrafield_local,
                       uInt size_extrafield_local,
                       const void* extrafield_global,
                       uInt size_extrafield_global,
                       const char* comment,
                       int method,
                       int level);
/*
  Open a file in the ZIP for writing.
  filename : the filename in zip (if NULL, '-' without quote will be used
  *zipfi contain supplemental information
  if extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
    contains the extrafield data the the local header
  if extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
    contains the extrafield data the the local header
  if comment != NULL, comment contain the comment string
  method contain the compression method (0 for store, Z_DEFLATED for deflate)
  level contain the level of compression (can be Z_DEFAULT_COMPRESSION)
*/

ZEXTERN int ZEXPORT zipOpenNewFileInZip2 (zipFile file,
                                            const char* filename,
                                            const zip_fileinfo* zipfi,
                                            const void* extrafield_local,
                                            uInt size_extrafield_local,
                                            const void* extrafield_global,
                                            uInt size_extrafield_global,
                                            const char* comment,
                                            int method,
                                            int level,
                                            int raw);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

ZEXTERN int ZEXPORT zipOpenNewFileInZip3 (zipFile file,
                                            const char* filename,
                                            const zip_fileinfo* zipfi,
                                            const void* extrafield_local,
                                            uInt size_extrafield_local,
                                            const void* extrafield_global,
                                            uInt size_extrafield_global,
                                            const char* comment,
                                            int method,
                                            int level,
                                            int raw,
                                            int windowBits,
                                            int memLevel,
                                            int strategy,
                                            const char* password,
                                            uLong crcForCrypting);
/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

ZEXTERN int ZEXPORT zipWriteInFileInZip (zipFile file,
                       const void* buf,
                       unsigned len);
/*
  Write data in the zipfile
*/

ZEXTERN int ZEXPORT zipCloseFileInZip (zipFile file);
/*
  Close the current file in the zipfile
*/

ZEXTERN int ZEXPORT zipCloseFileInZipRaw (zipFile file,
                                            uLong uncompressed_size,
                                            uLong crc32);
/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
*/

ZEXTERN int ZEXPORT zipClose (zipFile file,
                const char* global_comment);
/*
  Close the zipfile
*/

#ifdef __cplusplus
}
#endif

#endif /* _zip_H */
