/* iowin32.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API
   This IO API version uses the Win32 API (for Microsoft Windows)
   Version 1.01e
   Copyright (C) Gilles Vollant
*/

#include <stdlib.h>

#define ZLIB_INTERNAL
#include "zlib.h"
#include "ioapi.h"
#include "iowin32.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

typedef struct
{
    HANDLE hf;
    int error;
} WIN32FILE_IOWIN;

static voidpf ZEXPORT win32_open_file_func (voidpf opaque, const char* filename, int mode)
{
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes;
    HANDLE hFile = 0;
    voidpf ret=NULL;
    (void)opaque;

    dwDesiredAccess = dwShareMode = dwFlagsAndAttributes = dwCreationDisposition = 0;

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        dwDesiredAccess = GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
        dwShareMode = FILE_SHARE_READ;
    }
    else
    if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
    }
    else
    if (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        dwCreationDisposition = CREATE_ALWAYS;
    }

    if ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL,
                      dwCreationDisposition, dwFlagsAndAttributes, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        hFile = NULL;

    if (hFile != NULL)
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(sizeof(WIN32FILE_IOWIN));
        if (ret==NULL)
            CloseHandle(hFile);
        else *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    return ret;
}

static uLong ZEXPORT win32_read_file_func (voidpf opaque, voidpf stream, void* buf, uLong size)
{
    uLong ret=0;
    HANDLE hFile = NULL;
    (void)opaque;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    if (hFile != NULL)
        if (!ReadFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
        }

    return ret;
}

static uLong ZEXPORT win32_write_file_func (voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    uLong ret=0;
    HANDLE hFile = NULL;
    (void)opaque;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    if (hFile !=NULL)
        if (!WriteFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
        }

    return ret;
}

static long ZEXPORT win32_tell_file_func (voidpf opaque, voidpf stream)
{
    long ret=-1;
    HANDLE hFile = NULL;
    (void)opaque;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    if (hFile != NULL)
    {
        DWORD dwSet = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
        if (dwSet == INVALID_SET_FILE_POINTER)
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
            ret = -1;
        }
        else
            ret=(long)dwSet;
    }
    return ret;
}

static long ZEXPORT win32_seek_file_func (voidpf opaque, voidpf stream, uLong offset, int origin)
{
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;

    long ret=-1;
    (void)opaque;
    if (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        break;
    default: return -1;
    }

    if (hFile != NULL)
    {
        DWORD dwSet = SetFilePointer(hFile, offset, NULL, dwMoveMethod);
        if (dwSet == INVALID_SET_FILE_POINTER)
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(int)dwErr;
            ret = -1;
        }
        else
            ret=0;
    }
    return ret;
}

static int ZEXPORT win32_close_file_func (voidpf opaque, voidpf stream)
{
    int ret=-1;
    (void)opaque;

    if (stream!=NULL)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        if (hFile != NULL)
        {
            CloseHandle(hFile);
            ret=0;
        }
        free(stream);
    }
    return ret;
}

static int ZEXPORT win32_error_file_func (voidpf opaque, voidpf stream)
{
    int ret=-1;
    (void)opaque;
    if (stream!=NULL)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    return ret;
}

ZEXTERN void ZEXPORT fill_win32_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque=NULL;
}
