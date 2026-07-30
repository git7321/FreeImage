/* iowin32.h -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API
   This IO API version uses the Win32 API (for Microsoft Windows)
   Version 1.01e
   Copyright (C) Gilles Vollant
*/

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXTERN void ZEXPORT fill_win32_filefunc (zlib_filefunc_def* pzlib_filefunc_def);

#ifdef __cplusplus
}
#endif
