# Microsoft Visual C++ generated build script - Do not modify

PROJ = ZLIB
DEBUG = 0
PROGTYPE = 1
CALLER =
ARGS =
DLLS =
RCDEFINES = -dNDEBUG -dZLIB_WINAPI
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = .
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG =
CPPCREATEPCHFLAG =
CUSEPCHFLAG =
CPPUSEPCHFLAG =
FIRSTC = ADLER32.C
FIRSTCPP =
RC = rc
CFLAGS_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /D "WINDOWS" /D "ZLIB_WINAPI" /FR /GD
LFLAGS_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /NOE /ALIGN:16 /MAP:FULL
LIBS_WDLL = libw commdlg shell olecli olesvr ldllcew
RCFLAGS = /nologo
RUNFLAGS =
DEFFILE = ZLIB16.DEF
OBJS_EXT =
LIBS_EXT =
CFLAGS = $(CFLAGS_WDLL) /I.
LFLAGS = $(LFLAGS_WDLL)
LIBS = $(LIBS_WDLL)
MAPFILE = nul
RCDEFINES = $(RCDEFINES)
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = ADLER32.SBR \
	COMPRESS.SBR \
	CRC32.SBR \
	DEFLATE.SBR \
	GZIO.SBR \
	INFBACK.SBR \
	INFFAST.SBR \
	INFLATE.SBR \
	INFTREES.SBR \
	TREES.SBR \
	UNCOMPR.SBR \
	ZUTIL.SBR \
	ZIP.SBR \
	UNZIP.SBR \
	IOAPI.SBR

ADLER32_DEP = .\zlib.h \
	.\zconf.h

COMPRESS_DEP = .\zlib.h \
	.\zconf.h

CRC32_DEP = .\zlib.h \
	.\zconf.h \
	.\crc32.h

DEFLATE_DEP = .\deflate.h \
	.\zutil.h \
	.\zlib.h \
	.\zconf.h

GZIO_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h

INFBACK_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h \
	.\inftrees.h \
	.\inflate.h \
	.\inffast.h \
	.\inffixed.h

INFFAST_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h \
	.\inftrees.h \
	.\inflate.h \
	.\inffast.h

INFLATE_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h \
	.\inftrees.h \
	.\inflate.h \
	.\inffast.h \
	.\inffixed.h

INFTREES_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h \
	.\inftrees.h

TREES_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h \
	.\deflate.h \
	.\trees.h

UNCOMPR_DEP = .\zlib.h \
	.\zconf.h

ZUTIL_DEP = .\zutil.h \
	.\zlib.h \
	.\zconf.h

ZLIB16_RCDEP =

ZIP_DEP = .\zlib.h \
	.\zconf.h \
	.\minizip\zip.h

UNZIP_DEP = .\zlib.h \
	.\zconf.h \
	.\minizip\unzip.h

IOAPI_DEP = .\zlib.h \
	.\zconf.h \
	.\minizip\ioapi.h

all:	$(PROJ).DLL $(PROJ).BSC

ADLER32.OBJ:	ADLER32.C $(ADLER32_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ADLER32.C

COMPRESS.OBJ:	COMPRESS.C $(COMPRESS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c COMPRESS.C

CRC32.OBJ:	CRC32.C $(CRC32_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CRC32.C

DEFLATE.OBJ:	DEFLATE.C $(DEFLATE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DEFLATE.C

GZIO.OBJ:	GZIO.C $(GZIO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GZIO.C

INFBACK.OBJ:	INFBACK.C $(INFBACK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INFBACK.C

INFFAST.OBJ:	INFFAST.C $(INFFAST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INFFAST.C

INFLATE.OBJ:	INFLATE.C $(INFLATE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INFLATE.C

INFTREES.OBJ:	INFTREES.C $(INFTREES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INFTREES.C

TREES.OBJ:	TREES.C $(TREES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TREES.C

UNCOMPR.OBJ:	UNCOMPR.C $(UNCOMPR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c UNCOMPR.C

ZUTIL.OBJ:	ZUTIL.C $(ZUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ZUTIL.C

ZLIB.RES:	ZLIB.RC $(ZLIB16_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r ZLIB.RC

ZIP.OBJ:	MINIZIP\ZIP.C $(ZIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MINIZIP\ZIP.C

UNZIP.OBJ:	MINIZIP\UNZIP.C $(UNZIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MINIZIP\UNZIP.C

IOAPI.OBJ:	MINIZIP\IOAPI.C $(IOAPI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MINIZIP\IOAPI.C

$(PROJ).DLL::	ZLIB.RES

$(PROJ).DLL::	ADLER32.OBJ COMPRESS.OBJ CRC32.OBJ DEFLATE.OBJ GZIO.OBJ INFBACK.OBJ \
	INFFAST.OBJ INFLATE.OBJ INFTREES.OBJ TREES.OBJ UNCOMPR.OBJ ZUTIL.OBJ \
	ZIP.OBJ UNZIP.OBJ IOAPI.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
ADLER32.OBJ +
COMPRESS.OBJ +
CRC32.OBJ +
DEFLATE.OBJ +
GZIO.OBJ +
INFBACK.OBJ +
INFFAST.OBJ +
INFLATE.OBJ +
INFTREES.OBJ +
TREES.OBJ +
UNCOMPR.OBJ +
ZUTIL.OBJ +
ZIP.OBJ +
UNZIP.OBJ +
IOAPI.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RCFLAGS) ZLIB.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	ZLIB.RES
	if not exist MSVC.BND 	$(RC) $(RCFLAGS) ZLIB.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)

$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<

clean:
	-del *.res
	-del *.sbr
	-del *.obj
	-del *.lib
	-del *.dll
	-del *.exe
	-del *.map
	-del *.idb
	-del *.pdb
	-del *.ncb
	-del *.exp
	-del *.opt
	-del *.plg
	-del *.bsc
	-del *.bnd
