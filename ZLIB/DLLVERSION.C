#include <windows.h>
#include "zutil.h"
typedef struct _DLLVERSIONINFO
{
	DWORD cbSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformID;
} DLLVERSIONINFO;

// Platform IDs for DLLVERSIONINFO
#define DLLVER_PLATFORM_WINDOWS   0x00000001 // Windows 95
#define DLLVER_PLATFORM_NT        0x00000002 // Windows NT

typedef struct _DLLVERSIONINFO2
{
	DLLVERSIONINFO info1;
	DWORD dwFlags;
	ULONGLONG ullVersion;
} DLLVERSIONINFO2;

#define DLLVER_MAJOR_MASK   0xFFFF000000000000
#define DLLVER_MINOR_MASK   0x0000FFFF00000000
#define DLLVER_BUILD_MASK   0x00000000FFFF0000
#define DLLVER_QFE_MASK     0x000000000000FFFF

#define MAKEDLLVERULL(major, minor, build, qfe) \
	(((ULONGLONG)(major) << 48) | \
	 ((ULONGLONG)(minor) << 32) | \
	 ((ULONGLONG)(build) << 16) | \
	 ((ULONGLONG)(  qfe) <<  0))

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);
ZEXTERN HRESULT ZEXPORT DllGetVersion(DLLVERSIONINFO2 *pdvi)
{
	if (!pdvi || (pdvi->info1.cbSize != sizeof (*pdvi)))
		return (E_INVALIDARG);
	pdvi->info1.dwMajorVersion = ZLIB_VER_MAJOR;
	pdvi->info1.dwMinorVersion = ZLIB_VER_MINOR;
	pdvi->info1.dwBuildNumber = ZLIB_VER_REVISION;
	pdvi->info1.dwPlatformID = DLLVER_PLATFORM_WINDOWS;
	if (pdvi->info1.cbSize == sizeof (DLLVERSIONINFO2))
		pdvi->ullVersion = MAKEDLLVERULL (ZLIB_VER_MAJOR, ZLIB_VER_MINOR, ZLIB_VER_REVISION, 0);
	return S_OK;
}
