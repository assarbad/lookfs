///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Purpose          : Functions similar to FindFirstFile/FindNextFile to
///                    enumerate files and directories.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __NTFINDFILE_H_VER__
#define __NTFINDFILE_H_VER__ 2017100319
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif /* Check for #pragma support */

typedef struct _NT_FIND_DATA
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    LARGE_INTEGER nFileSize;
    _Field_z_ WCHAR  cFileName[MAX_PATH];
} NT_FIND_DATA, *PNT_FIND_DATA, *LPNT_FIND_DATA;

_Success_(return != INVALID_HANDLE_VALUE && return != NULL)
EXTERN_C HANDLE WINAPI NativeFindFirstFile(_In_z_ LPCTSTR lpFileName, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData, BOOLEAN bCaseSensitive);

_Success_(return != 0)
EXTERN_C BOOL WINAPI NativeFindNextFile(_In_ HANDLE hFindFile, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData);

_Success_(return != 0)
EXTERN_C BOOL NativeFindClose(_In_opt_ HANDLE hFindFile);

#endif // __NTFINDFILE_H_VER__
