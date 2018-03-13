///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Purpose          : Functions similar to FindFirstFile/FindNextFile to
///                    enumerate files and directories.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __NTFINDFILE_H_VER__
#define __NTFINDFILE_H_VER__ 2018031322
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif /* Check for #pragma support */

// Fake the SAL1 annotations where they don't exist.
#if !defined(__in_bcount) && !defined(_In_reads_bytes_)
#   define __success(x)
#   define __field_range(x, y)
#   define __field_nullterminated
#   define __in
#   define __in_z
#   define __in_bcount(x)
#   define __in_opt
#   define __inout
#   define __inout_opt
#   define __out
#   define __out_bcount(x)
#   define __out_opt
#   define __out_bcount_opt(x)
#   define __reserved
#endif

// Fake the SAL2 annotations where they don't exist.
#if defined(__in_bcount) && !defined(_In_reads_bytes_)
#   define _Success_(x) __success(x)
#   define _Field_range_(x, y) __field_range(x, y)
#   define _Field_z_ __field_nullterminated
#   define _In_ __in
#   define _In_z_ __in_z
#   define _In_reads_bytes_(x) __in_bcount(x)
#   define _In_opt_ __in_opt
#   define _Inout_ __inout
#   define _Inout_opt_ __inout_opt
#   define _Out_ __out
#   define _Out_writes_bytes_(x) __out_bcount(x)
#   define _Out_opt_ __out_opt
#   define _Out_writes_bytes_opt_(x) __out_bcount_opt(x)
#   define _Reserved_ __reserved
#endif

#ifndef _Ret_maybenull_
#   define _Ret_maybenull_
#endif

#ifndef _Ret_writes_bytes_maybenull_
#   define _Ret_writes_bytes_maybenull_(Size)
#endif

#ifndef _Post_writable_byte_size_
#   define _Post_writable_byte_size_(Size)
#endif

#ifndef _Post_invalid_
#   define _Post_invalid_
#endif

#ifndef _When_
#   define _When_(x, y)
#endif

#ifndef _Out_range_
#   define _Out_range_(x, y)
#endif

#ifndef _Frees_ptr_opt_
#   define _Frees_ptr_opt_
#endif

typedef struct _NT_FIND_DATA
{
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    LONGLONG nFileSize;
    LONGLONG nAllocSize;
    DWORD dwFileAttributes;
    DWORD dwReparseTag;
    _Field_z_ WCHAR cFileName[MAX_PATH];
} NT_FIND_DATA, *PNT_FIND_DATA, *LPNT_FIND_DATA;

_Success_(return != 0)
EXTERN_C BOOLEAN NativeFindInit(_In_ ULONG cbInitialBuffer);

_Success_(return != INVALID_HANDLE_VALUE && return != NULL)
EXTERN_C HANDLE WINAPI NativeFindFirstFile(_In_z_ LPCTSTR lpFileName, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData, BOOLEAN bCaseSensitive);

_Success_(return != 0)
EXTERN_C BOOL WINAPI NativeFindNextFile(_In_ HANDLE hFindFile, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData);

_Success_(return != 0)
EXTERN_C BOOL NativeFindClose(_In_opt_ HANDLE hFindFile);

EXTERN_C NTSTATUS WINAPI NativeFindLastStatus(_In_ HANDLE hFindFile);

#endif // __NTFINDFILE_H_VER__
