/* ioapi.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API
   Version 1.01e
   Copyright (C) Gilles Vollant
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZLIB_INTERNAL
#include "zlib.h"
#include "ioapi.h"

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

static voidpf ZEXPORT fopen_file_func (voidpf opaque, const char* filename, int mode)
{
    FILE* file = NULL;
    const char* mode_fopen = NULL;
    (void)opaque;
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    if ((filename!=NULL) && (mode_fopen != NULL))
        file = fopen(filename, mode_fopen);
    return file;
}

static uLong ZEXPORT fread_file_func (voidpf opaque, voidpf stream, void* buf, uLong size)
{
    uLong ret;
    (void)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    return ret;
}

static uLong ZEXPORT fwrite_file_func (voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    uLong ret;
    (void)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    return ret;
}

static long ZEXPORT ftell_file_func (voidpf opaque, voidpf stream)
{
    long ret;
    (void)opaque;
    ret = ftell((FILE *)stream);
    return ret;
}

static long ZEXPORT fseek_file_func (voidpf opaque, voidpf stream, uLong offset, int origin)
{
    int fseek_origin=0;
    long ret;
    (void)opaque;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;
    default: return -1;
    }
    ret = 0;
    if (fseek((FILE *)stream, offset, fseek_origin) != 0)
        ret = -1;
    return ret;
}

static int ZEXPORT fclose_file_func (voidpf opaque, voidpf stream)
{
    int ret;
    (void)opaque;
    ret = fclose((FILE *)stream);
    return ret;
}

static int ZEXPORT ferror_file_func (voidpf opaque, voidpf stream)
{
    int ret;
    (void)opaque;
    ret = ferror((FILE *)stream);
    return ret;
}

ZEXTERN void ZEXPORT fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
