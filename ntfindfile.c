/******************************************************************************
 ***
 *** Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
 ***
 *** Purpose          : Functions similar to FindFirstFile/FindNextFile to
 ***                    enumerate files and directories (also FindClose ;-)).
 ***
 ******************************************************************************/
#ifdef NTFINDFILE_DYNAMIC
#   ifdef DYNAMIC_NTNATIVE
#       error DYNAMIC_NTNATIVE should _NOT_ be defined if NTFINDFILE_DYNAMIC is defined
#   endif /* DYNAMIC_NTNATIVE */
#   define DYNAMIC_NTNATIVE NTFINDFILE_DYNAMIC
#endif /* NTFINDFILE_DYNAMIC */
#include "ntfindfile.h"
#ifdef NTFIND_USE_CRTALLOC
#include <stdlib.h>
#endif /* NTFIND_USE_CRTALLOC */
#ifdef _DEBUG
#   include <stdio.h>
#   include <tchar.h>
#   define _TRACELVL -1
#   if 0
#   define _TRACE(x, fmt, ...) if (x <= _TRACELVL) do { _ftprintf(stderr, _T("[TRACE%d:%hs] "), _TRACELVL, __##FUNCTION##__); _ftprintf(stderr, _T(fmt), __VA_ARGS__); _ftprintf(stderr, _T("\n")); fflush(stderr); } while (0)
#   endif /* 0 */
#   define _TRACE(x, fmt, ...) do {} while (0)
#else
#   define _TRACE(x, fmt, ...) do {} while (0)
#endif /* _DEBUG */

#define FILE_ATTRIBUTE_REPARSE_DIR (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY)

#if defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC)
__declspec(thread) static NtOpenFile_t pfnNtOpenFile = NULL;
__declspec(thread) static NtClose_t pfnNtClose = NULL;
__declspec(thread) static NtQueryDirectoryFile_t pfnNtQueryDirectoryFile = NULL;
__declspec(thread) static RtlDosPathNameToNtPathName_U_t pfnRtlDosPathNameToNtPathName_U = NULL;
__declspec(thread) static RtlDosPathNameToNtPathName_U_WithStatus_t pfnRtlDosPathNameToNtPathName_U_WithStatus = NULL;
__declspec(thread) static RtlFreeUnicodeString_t pfnRtlFreeUnicodeString = NULL;
__declspec(thread) static RtlInitUnicodeString_t pfnRtlInitUnicodeString = NULL;
__declspec(thread) static RtlCreateUnicodeString_t pfnRtlCreateUnicodeString = NULL;
__declspec(thread) static RtlNtStatusToDosError_t pfnRtlNtStatusToDosError = NULL;
#else
#define pfnNtOpenFile NtOpenFile
#define pfnNtClose NtClose
#define pfnNtQueryDirectoryFile NtQueryDirectoryFile
#define pfnRtlDosPathNameToNtPathName_U RtlDosPathNameToNtPathName_U
#define pfnRtlDosPathNameToNtPathName_U_WithStatus RtlDosPathNameToNtPathName_U_WithStatus
#define pfnRtlFreeUnicodeString RtlFreeUnicodeString
#define pfnRtlInitUnicodeString RtlInitUnicodeString
#define pfnRtlCreateUnicodeString RtlCreateUnicodeString
#define pfnRtlNtStatusToDosError RtlNtStatusToDosError
#endif

#if !defined(NTFIND_ALLOC_FUNC) && !defined(NTFIND_FREE_FUNC)
static LPVOID NTFIND_ALLOC_FUNC(SIZE_T Bytes)
{
#ifndef NTFIND_USE_CRTALLOC
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Bytes);
#else
    return calloc(Bytes, 1);
#endif /* NTFIND_USE_CRTALLOC */
}

static BOOL NTFIND_FREE_FUNC(LPVOID lpMem)
{
#ifndef NTFIND_USE_CRTALLOC
    return HeapFree(GetProcessHeap(), 0, lpMem);
#else
    free(lpMem);
    return TRUE;
#endif /* NTFIND_USE_CRTALLOC */
}
#endif
#define NativeFindFreeAndNull(x) NativeFindFree(x); (x) = NULL

EXTERN_C ntAlloc_t NativeFindAlloc = NTFIND_ALLOC_FUNC;
EXTERN_C ntFree_t NativeFindFree = NTFIND_FREE_FUNC;

#define SetLastErrorFromNtError(x) SetLastError((LONG)pfnRtlNtStatusToDosError(x))

#if defined(_DEBUG) || (defined(_MSC_VER) && (_MSC_VER <= 1400))
#pragma warning(disable: 4127)
#endif

__declspec(thread) static ULONG s_uInitialBufSize = NTFIND_LARGE_BUFFER_SIZE;
#ifdef _DEBUG
__declspec(thread) static LONGLONG s_llMemoryAllocated = 0;
#endif /* _DEBUG */

#ifdef _DEBUG
EXTERN_C LONGLONG NativeFindGetAllocatedBytes()
{
    return s_llMemoryAllocated;
}
#endif /* _DEBUG */

/* Fallback function so we can simply call pfnRtlDosPathNameToNtPathName_U_WithStatus */
#if defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC)
FORCEINLINE static
#endif /* defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC) */
NTSTATUS NTAPI FallbackRtlDosPathNameToNtPathName_U_WithStatus(_In_ PCWSTR DosFileName, _Out_ PUNICODE_STRING NtFileName, _Out_opt_ PWSTR *FilePart, _Out_opt_ PRTL_RELATIVE_NAME RelativeName)
{
#if defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC)
    if (!pfnRtlDosPathNameToNtPathName_U)
    {
        return STATUS_PROCEDURE_NOT_FOUND; /* STATUS_DELAY_LOAD_FAILED may also make sense here */
    }
#endif /* defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC) */
    _TRACE(3, "Fallback function");
    if (!pfnRtlDosPathNameToNtPathName_U(DosFileName, NtFileName, FilePart, RelativeName))
    {
        return STATUS_OBJECT_NAME_INVALID; /* most likely outcome */
    }
    return STATUS_SUCCESS;
}

_Success_(return != FALSE)
EXTERN_C BOOLEAN NativeFindInit(_In_ ULONG cbInitialBuffer)
{
#if defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC)
    __declspec(thread) static HMODULE hNtDll = NULL;
#endif
    s_uInitialBufSize = (cbInitialBuffer) ? cbInitialBuffer : NTFIND_LARGE_BUFFER_SIZE;
#if defined(NTFINDFILE_DYNAMIC) && (NTFINDFILE_DYNAMIC)
    if (!hNtDll)
    {
        hNtDll = GetModuleHandle(_T("ntdll.dll"));
        if (!hNtDll)
        {
            SetLastError(ERROR_MOD_NOT_FOUND);
            return FALSE;
        }
#define NTFIND_DEFFUNC(x) pfn##x = (x##_t)NTNATIVE_GETPROCADDR(hNtDll, x)
        NTFIND_DEFFUNC(NtOpenFile);
        NTFIND_DEFFUNC(NtClose);
        NTFIND_DEFFUNC(NtQueryDirectoryFile);
        NTFIND_DEFFUNC(RtlDosPathNameToNtPathName_U);
        NTFIND_DEFFUNC(RtlDosPathNameToNtPathName_U_WithStatus);
        NTFIND_DEFFUNC(RtlFreeUnicodeString);
        NTFIND_DEFFUNC(RtlInitUnicodeString);
        NTFIND_DEFFUNC(RtlCreateUnicodeString);
        NTFIND_DEFFUNC(RtlNtStatusToDosError);
#undef NTFIND_DEFFUNC
        if (!pfnRtlDosPathNameToNtPathName_U_WithStatus)
        {
            /* We have a fallback for that one */
            pfnRtlDosPathNameToNtPathName_U_WithStatus = FallbackRtlDosPathNameToNtPathName_U_WithStatus;
        }
        if (
            !pfnNtOpenFile || !pfnNtClose || !pfnNtQueryDirectoryFile
            || !pfnRtlDosPathNameToNtPathName_U_WithStatus
            || !pfnRtlFreeUnicodeString || !pfnRtlInitUnicodeString
            || !pfnRtlNtStatusToDosError || !pfnRtlCreateUnicodeString
            )
        {
            SetLastError(ERROR_INVALID_FUNCTION);
            hNtDll = NULL;
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
#else
    return TRUE;
#endif
}



_Success_(return != NULL)
_Ret_writes_bytes_maybenull_(sizeof(NTFIND_HANDLE))
_Post_writable_byte_size_(sizeof(NTFIND_HANDLE))
__inline static NTFIND_HANDLE* initFindHandle_(_In_ HANDLE hDirectory, _In_reads_bytes_(sizeof(UNICODE_STRING)) PCUNICODE_STRING pusNtPathName, _In_reads_bytes_(sizeof(UNICODE_STRING)) PCUNICODE_STRING pusPartName, ULONG dwFlags)
{
    NTFIND_HANDLE* pFindHandle;

    pFindHandle = NativeFindAlloc(sizeof(*pFindHandle));

    if (pFindHandle)
    {
#ifdef _DEBUG
        s_llMemoryAllocated += sizeof(*pFindHandle);
#endif /* _DEBUG */
        __try
        {
            InitializeCriticalSection(&pFindHandle->csFindHandle);
        }
        __except ((GetExceptionCode() != EXCEPTION_POSSIBLE_DEADLOCK) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            SetLastErrorFromNtError(GetExceptionCode());
            (void)NativeFindFree(pFindHandle);
#ifdef _DEBUG
            s_llMemoryAllocated -= sizeof(*pFindHandle);
#endif /* _DEBUG */
            return NULL;
        }
        pFindHandle->hDirectory = hDirectory;
        pFindHandle->dwFlags = dwFlags;
        pFindHandle->usNtPathName = *pusNtPathName;
        pFindHandle->usPartName = *pusPartName;
    }

    return pFindHandle;
}

_Success_(return != FALSE)
FORCEINLINE static BOOLEAN freeFindHandle_(_Frees_ptr_ NTFIND_HANDLE* pFindHandle)
{
    if (pFindHandle)
    {
        EnterCriticalSection(&pFindHandle->csFindHandle);
        if (pFindHandle->pucBuffer)
        {
            (void)NativeFindFree(pFindHandle->pucBuffer);
#ifdef _DEBUG
            s_llMemoryAllocated -= pFindHandle->cbBuffer;
#endif /* _DEBUG */
            pFindHandle->pucBuffer = NULL;
        }
        pFindHandle->cbBuffer = 0;
        pFindHandle->pucNextEntry = NULL;
        (void)pfnNtClose(pFindHandle->hDirectory);
        pFindHandle->hDirectory = NULL;
        LeaveCriticalSection(&pFindHandle->csFindHandle);
        DeleteCriticalSection(&pFindHandle->csFindHandle);
#ifdef _DEBUG
        s_llMemoryAllocated -= sizeof(*pFindHandle);
#endif /* _DEBUG */
        return NativeFindFree(pFindHandle) ? TRUE : FALSE;
    }
    return FALSE;
}

/* increases (currently doubles, up to a maximum) the size of the internal buffer */
/* Return value of TRUE means the buffer size was increased, FALSE mean it has been kept intact */
_Success_(return != FALSE)
FORCEINLINE static BOOLEAN increaseQueryBufferSizeAndZerofill_(_In_reads_bytes_(sizeof(NTFIND_HANDLE)) NTFIND_HANDLE* pFindHandle)
{
    if (pFindHandle)
    {
        /* fast path, to avoid the critsec */
        if (pFindHandle->bMaxQueryBufferSizeReached)
        {
            pFindHandle->ntStatus = STATUS_ALLOTTED_SPACE_EXCEEDED;
            return FALSE; /* nothing for us to do */
        }

        EnterCriticalSection(&pFindHandle->csFindHandle);
        {
            ULONG cbBuffer = 0;
            if (pFindHandle->pucBuffer && pFindHandle->cbBuffer)
            {
                cbBuffer = pFindHandle->cbBuffer * 2;
                if (cbBuffer >= NTFIND_LARGE_BUFFER_MAXSIZE)
                {
                    if (cbBuffer == pFindHandle->cbBuffer)
                    {
                        _TRACE(2, "Already allocated maximum query buffer size (%u)", pFindHandle->cbBuffer);
                        pFindHandle->ntStatus = STATUS_ALLOTTED_SPACE_EXCEEDED;
                        pFindHandle->bMaxQueryBufferSizeReached = TRUE;
                        LeaveCriticalSection(&pFindHandle->csFindHandle);
                        return FALSE; /* nothing for us to do */
                    }
                    cbBuffer = NTFIND_LARGE_BUFFER_MAXSIZE; /* adjust to maximum */
                    _TRACE(2, "Setting amount to allocate to fixed limit (%u)", cbBuffer);
                }
            }
            else /* Buffer not yet allocated? */
            {
                /*_ASSERTE(!pFindHandle->pBuffer);*/
                /*_ASSERTE(!pFindHandle->cbBuffer);*/
                cbBuffer = s_uInitialBufSize;
                _TRACE(2, "Setting amount to allocate to initial size (%u)", cbBuffer);
            }

            /* be cautious about integer overflows, unimportant at the moment, but someone may tinker with the limits and this may happen */
            if (cbBuffer > pFindHandle->cbBuffer)
            {
                /* By allocating an additional sizeof(WCHAR) we can still safely zero-terminate even the last entry */
                PVOID pNew = NativeFindAlloc(cbBuffer + sizeof(WCHAR));

                if (!pNew)
                {
                    _TRACE(0, "[ERR] Failed to allocate pNew (%u), assuming OOM", cbBuffer);
                    pFindHandle->ntStatus = STATUS_NO_MEMORY;
                    LeaveCriticalSection(&pFindHandle->csFindHandle);
                    return FALSE; /* we assume this is an OOM condition */
                }

                _TRACE(2, "Bumped query buffer size to %u Byte(s) [from %u]", cbBuffer, pFindHandle->cbBuffer);
#ifdef _DEBUG
                s_llMemoryAllocated += cbBuffer;
#endif /* _DEBUG */

                if (pFindHandle->pucBuffer && pFindHandle->cbBuffer)
                {
                    /* we free after allocating a bigger chunk of memory */
                    if (!NativeFindFree(pFindHandle->pucBuffer))
                    {
                        _TRACE(0, "[ERR] Failed to free original buffer");
                        pFindHandle->ntStatus = STATUS_UNABLE_TO_FREE_VM;
                        (void)NativeFindFree(pNew);
                        LeaveCriticalSection(&pFindHandle->csFindHandle);
                        return FALSE; /* buffer still valid, but no bigger than before */
                    }

#ifdef _DEBUG
                    s_llMemoryAllocated -= pFindHandle->cbBuffer;
#endif /* _DEBUG */
                }

                if (cbBuffer > s_uInitialBufSize)
                {
                    /* Bump the initial buffer size to the biggest we've seen so far */
                    s_uInitialBufSize = cbBuffer;
                }
                pFindHandle->bMaxQueryBufferSizeReached = (cbBuffer >= NTFIND_LARGE_BUFFER_MAXSIZE);
                pFindHandle->cbBuffer = cbBuffer;
                pFindHandle->pucBuffer = (PUCHAR)pNew;
                pFindHandle->pucNextEntry = NULL;
                LeaveCriticalSection(&pFindHandle->csFindHandle);
                _TRACE(2, "Successfully allocated query buffer");
                return TRUE;
            }
            else
            {
                _TRACE(0, "[ERR] Integer overflow");
                pFindHandle->ntStatus = STATUS_INTEGER_OVERFLOW;
            }
            LeaveCriticalSection(&pFindHandle->csFindHandle);
        }
        if (NT_SUCCESS(pFindHandle->ntStatus)) /* no success at this point */
        {
            _TRACE(0, "[ERR] Nondescript error state");
            pFindHandle->ntStatus = STATUS_UNSUCCESSFUL; /* most generic status code ... duh */
        }
    }
    return FALSE;
}

/* wraps NtQueryDirectoryFile and handles resizing the buffer inside the find handle */
/* This function updates pFindHandle->ntStatus and therefore has no return value */
FORCEINLINE void wrapNtQueryDirectoryFile_(_Inout_updates_(sizeof(NTFIND_HANDLE)) NTFIND_HANDLE* pFindHandle)
{
    NTSTATUS ntStatus;
    BOOLEAN bFirstScan = (BOOLEAN)(NULL == pFindHandle->pucBuffer);
    PUNICODE_STRING pusFileMask = (bFirstScan) ? &pFindHandle->usPartName : NULL;

    if (bFirstScan)
    {
        _TRACE(2, "Initial allocation of query buffer");
        if (!increaseQueryBufferSizeAndZerofill_(pFindHandle))
        {
            _TRACE(2, "Initial allocation of query buffer FAILED!");
            /* pFindHandle->ntStatus contains the status code */
            return;
        }
    }
    else
    {
        /* make sure we don't attempt to write beyond the allocated buffer */
        ULONG cbFilled = (ULONG)pFindHandle->iostat_NtQueryDirectoryFile.Information;
        if (cbFilled > pFindHandle->cbBuffer)
        {
            cbFilled = pFindHandle->cbBuffer;
        }
        /* zero-fill what had previously been written */
        memset(pFindHandle->pucBuffer, 0, cbFilled);
    }

    if (!pFindHandle->pucBuffer || !pFindHandle->cbBuffer)
    {
        pFindHandle->ntStatus = STATUS_NO_MEMORY;
        return;
    }

    ntStatus = pFindHandle->ntStatus = pfnNtQueryDirectoryFile(
        pFindHandle->hDirectory,
        NULL,
        NULL,
        NULL,
        &pFindHandle->iostat_NtQueryDirectoryFile,
        pFindHandle->pucBuffer,
        pFindHandle->cbBuffer,
        NtFindInfoClass,
        FALSE, /* ReturnSingleEntry */
        pusFileMask,
        bFirstScan /* RestartScan */
    );
    _TRACE(2, "NtQueryDirectoryFile(): 0x%08X [restart = %d, bufsz = %u, 0x%08X]", ntStatus, bFirstScan, pFindHandle->cbBuffer, pFindHandle->iostat_NtQueryDirectoryFile.Status);

    while ((STATUS_BUFFER_OVERFLOW == ntStatus) || (STATUS_BUFFER_TOO_SMALL == ntStatus) || (STATUS_SUCCESS == ntStatus))
    {
        BOOLEAN bBreak = FALSE;
        /* resize the buffer */
        if (!increaseQueryBufferSizeAndZerofill_(pFindHandle))
        {
            /* Reached maximum size for one individual query buffer, but still not big enough to fit all entries */
            if (STATUS_ALLOTTED_SPACE_EXCEEDED == pFindHandle->ntStatus)
            {
#ifdef _DEBUG
                InterlockedIncrement64(&pFindHandle->llAdditionalQueriesNeeded);
#endif /* _DEBUG */
                bBreak = TRUE;
            }
            else
            {
                /* pFindHandle->ntStatus contains the status code */
                return;
            }
        }
        /* retry with bigger buffer */
        ntStatus = pFindHandle->ntStatus = pfnNtQueryDirectoryFile(
            pFindHandle->hDirectory,
            NULL,
            NULL,
            NULL,
            &pFindHandle->iostat_NtQueryDirectoryFile,
            pFindHandle->pucBuffer,
            pFindHandle->cbBuffer,
            NtFindInfoClass,
            FALSE, /* ReturnSingleEntry */
            pusFileMask,
            bFirstScan /* RestartScan */
        );
        _TRACE(2, "NtQueryDirectoryFile(): 0x%08X [restart = %d, bufsz = %u, 0x%08X]", ntStatus, bFirstScan, pFindHandle->cbBuffer, pFindHandle->iostat_NtQueryDirectoryFile.Status);
        /* This logic ensures we break out of the loop when the buffer is big enough to fit all entries */
        if ((STATUS_SUCCESS == ntStatus) && ((pFindHandle->cbBuffer - (ULONG)pFindHandle->iostat_NtQueryDirectoryFile.Information) > sizeof(NTFIND_DIR_INFO)))
        {
            _TRACE(2, "bufsz[%u] - written[%u] = slack[%u] > sizeof(NTFIND_DIR_INFO)", pFindHandle->cbBuffer, (ULONG)pFindHandle->iostat_NtQueryDirectoryFile.Information, pFindHandle->cbBuffer - (ULONG)pFindHandle->iostat_NtQueryDirectoryFile.Information);
            break;
        }
        if (bBreak)
        {
            break;
        }
    }

    pFindHandle->bNoMoreFiles = (STATUS_NO_MORE_FILES == pFindHandle->ntStatus);
}

FORCEINLINE void updateNextEntryPointer_(_Inout_updates_(sizeof(NTFIND_HANDLE)) NTFIND_HANDLE* pFindHandle, _In_reads_bytes_(sizeof(NTFIND_DIR_INFO)) NTFIND_DIR_INFO* dirinfo)
{
/*-----------------------------------------------------------------------------
  Explanation:

  At least on Windows 7 x64 I observed that NTFIND_DIR_INFO::FileName ends up
  NOT zero-terminated. This happens apparently whenever the end of the
  (unterminated) string falls exactly on a 16-byte alignment boundary _and_ ends
  up falling exactly onto the same address as start of the next entry.
  
  It makes sense, because the alignment has certain performance implications and
  placing the start of the _next_ entry properly aligned is best.

  However, this necessitates a little workaround, so we can provide the caller
  with a zero-terminated string, no matter what.

  The workaround has the nice side-effect of invalidating NextEntryOffset so it
  becomes harder for callers to poke around the NTFIND_DIR_INFO entry chain.
  -----------------------------------------------------------------------------*/

    /* This is part one of the workaround for file names not being zero-
       terminated inside the NTFIND_DIR_INFO structure in some instances. */
    if (!dirinfo->NextEntryOffset && pFindHandle->uNextNextEntryOffset)
    {
        dirinfo->NextEntryOffset = pFindHandle->uNextNextEntryOffset;
    }
    pFindHandle->uNextNextEntryOffset = 0;

    if (dirinfo->NextEntryOffset)
    {
        NTFIND_DIR_INFO* dirinfoNext;
        LPWSTR lpszFileNameEnd = &dirinfo->FileName[dirinfo->FileNameLength / sizeof(WCHAR)];

        /* update the "next" pointer to point to the next entry */
        pFindHandle->pucNextEntry = (PUCHAR)(dirinfo) + dirinfo->NextEntryOffset;

        /* peek into it */
        dirinfoNext = (NTFIND_DIR_INFO*)pFindHandle->pucNextEntry;

        /* store the next (next) entry offset, because we'll invalidate it */
        pFindHandle->uNextNextEntryOffset = dirinfoNext->NextEntryOffset;

        /* this will take care of those instances where the next entry starts
           right at the end of the file name buffer, thus leaving the file
           name buffer without zero-termination */
        *lpszFileNameEnd = 0;
        dirinfoNext->NextEntryOffset = 0;
        /* additional advantage: we make it harder for the caller of the zero
           copy enumeration to poke around the NTFIND_DIR_INFO entry chain */

#ifdef _DEBUG
        if (0 != *lpszFileNameEnd)
        {
            _TRACE(2, "end %p .|. %p next", ((PUCHAR)&dirinfo->FileName) + dirinfo->FileNameLength, pFindHandle->pucNextEntry);
            _TRACE(0, "[ERR] UNEXPECTED: file name %*s (%zu -> %u) not zero-terminated!", (int)(dirinfo->FileNameLength / sizeof(WCHAR)), dirinfo->FileName, dirinfo->FileNameLength / sizeof(WCHAR), dirinfo->FileNameLength);
        }
#endif /* _DEBUG */
    }
    else
    {
        /* ... or signal end of entry list */
        pFindHandle->pucNextEntry = NULL;
    }
}

__inline static NTFIND_DIR_INFO* getNextDirInfoEntry_(_In_reads_bytes_(sizeof(NTFIND_HANDLE)) NTFIND_HANDLE* pFindHandle)
{
    NTFIND_DIR_INFO* dirinfo;

    /* Nothing has been queried before or we ran out of buffer space and need to query the next one */
    if (!pFindHandle->pucBuffer || (!pFindHandle->pucNextEntry && !pFindHandle->bNoMoreFiles))
    {
        wrapNtQueryDirectoryFile_(pFindHandle);
        if (!NT_SUCCESS(pFindHandle->ntStatus))
        {
            return NULL;
        }
        pFindHandle->pucNextEntry = pFindHandle->pucBuffer;
    }

    dirinfo = (NTFIND_DIR_INFO*)pFindHandle->pucNextEntry;
    if (dirinfo)
    {
        updateNextEntryPointer_(pFindHandle, dirinfo);
        if (NTFIND_SUPPRESS_SELF_AND_PARENT == (pFindHandle->dwFlags & NTFIND_SUPPRESS_SELF_AND_PARENT))
        {
            if (
                (dirinfo->FileNameLength == sizeof(WCHAR) && dirinfo->FileName[0] == L'.')
                || (dirinfo->FileNameLength == 2 * sizeof(WCHAR) && dirinfo->FileName[0] == L'.' && dirinfo->FileName[1] == L'.')
                )
            {
                return getNextDirInfoEntry_(pFindHandle);
            }
        }
    }

    return dirinfo;
}

FORCEINLINE static NTSTATUS openDirectoryForSearch_(_Out_writes_bytes_(sizeof(HANDLE)) HANDLE* phDirectory, _In_opt_ HANDLE hParentDir, _In_z_ LPCWSTR lpszPathName, _Out_writes_bytes_(sizeof(UNICODE_STRING)) PUNICODE_STRING pusNtPathName, _Out_writes_bytes_(sizeof(UNICODE_STRING)) PUNICODE_STRING pusPartName, BOOLEAN bCaseSensitive)
{
    UNICODE_STRING usNtPathName = { 0, 0, NULL }, usPartName = { 0, 0, NULL };
    NTSTATUS ntStatus;
    OBJECT_ATTRIBUTES oa;
    HANDLE hDirectory;
    IO_STATUS_BLOCK iostat;
    BOOLEAN bStrippedBackslash = FALSE;
    LPWSTR lpszFilePart;

    /* this is used for walking immediate subdirectories of the given directory */
    if (hParentDir)
    {
        LPWSTR lpszCurrChar, lpszEnd;
        size_t count = 0;
        /* We use this instead of RtlInitUnicodeString, because this buffer will get freed down the call path */
        if (!pfnRtlCreateUnicodeString(&usNtPathName, lpszPathName) || !usNtPathName.Buffer)
        {
            return STATUS_NO_MEMORY;
        }
        lpszEnd = &usNtPathName.Buffer[usNtPathName.Length / sizeof(WCHAR)];
        /* walk through the string from its back */
        for (lpszCurrChar =  &lpszEnd[-1]; lpszCurrChar >= usNtPathName.Buffer; lpszCurrChar--)
        {
            if (*lpszCurrChar == L'\\')
            {
                if (lpszCurrChar == usNtPathName.Buffer) /* path starts with backslash, so not relative*/
                {
                    count += 2;
                    break;
                }
                /* have we seen a backslash before? */
                if (!count)
                {
                    /* make sure the backslash isn't the last character before the (original) terminating zero */
                    if (&lpszCurrChar[1] < lpszEnd)
                    {
                        /* set terminating zero instead of last backslash */
                        lpszCurrChar[0] = 0;
                        /* adjust length of our UNICODE_STRING to match the zero-termination */
                        usNtPathName.Length -= (USHORT)(0xFFFFU & ((lpszEnd - lpszCurrChar) * sizeof(WCHAR)));
                        /* create the other needed UNICODE_STRING */
                        if (!pfnRtlCreateUnicodeString(&usPartName, &lpszCurrChar[1]))
                        {
                            pfnRtlFreeUnicodeString(&usNtPathName);
                            return STATUS_NO_MEMORY;
                        }
                    }
                    else
                    {
                        pfnRtlFreeUnicodeString(&usNtPathName);
                        return STATUS_DATA_OVERRUN;
                    }
                }
                count++;
            }
        }
        /* more or less than one backslash seen? less means we have no search mask, more means it's not an immediate subdirectory. */
        if (count != 1)
        {
            pfnRtlFreeUnicodeString(&usNtPathName);
            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        RTL_RELATIVE_NAME relName;
        USHORT uLen;

        /* Note how we're passing the Buffer of usPartName here, it will point into the Buffer of usNtPathName if we succeed */
        ntStatus = pfnRtlDosPathNameToNtPathName_U_WithStatus(lpszPathName, &usNtPathName, &usPartName.Buffer, &relName);

        if (!NT_SUCCESS(ntStatus))
        {
            return ntStatus;
        }

        if (usPartName.Buffer)
        {
            usPartName.Length = usNtPathName.Length - (USHORT)(0xFFFFU & ((ULONG_PTR)usPartName.Buffer - (ULONG_PTR)usNtPathName.Buffer));
        }

        if (relName.RelativeName.Length && relName.RelativeName.Buffer != usPartName.Buffer)
        {
            if (usPartName.Buffer)
            {
                usNtPathName.Buffer = relName.RelativeName.Buffer;
                usNtPathName.Length = (USHORT)(0xFFFFU & ((ULONG_PTR)usPartName.Buffer - (ULONG_PTR)relName.RelativeName.Buffer));
            }
            hParentDir = relName.ContainingDirectory;
        }
        else
        {
            if (usPartName.Buffer)
            {
                usNtPathName.Length = (USHORT)(0xFFFFU & ((ULONG_PTR)usPartName.Buffer - (ULONG_PTR)usNtPathName.Buffer));
            }
        }

        uLen = usNtPathName.Length / sizeof(WCHAR);
        /* not the root directory of a "drive"? */
        if (L':' != usNtPathName.Buffer[uLen - 2] && L'\\' != usNtPathName.Buffer[uLen - 1])
        {
            /* strip trailing slash and make a note of it */
            usNtPathName.Length -= sizeof(WCHAR);
            bStrippedBackslash = TRUE;
        }
    }

    /* not strictly necessary, but makes some things easier */
    lpszFilePart = usPartName.Buffer;
    if (!pfnRtlCreateUnicodeString(&usPartName, lpszFilePart))
    {
        pfnRtlFreeUnicodeString(&usNtPathName);
        return STATUS_NO_MEMORY;
    }
    *lpszFilePart = 0; /* zero-terminate here */

    InitializeObjectAttributes(&oa, &usNtPathName, (bCaseSensitive) ? 0 : OBJ_CASE_INSENSITIVE, hParentDir, NULL);

    ntStatus = pfnNtOpenFile(
        &hDirectory,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &oa,
        &iostat,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT
    );

    /* if it fails in this specific way and we did strip the backslash, try to put it back and try opening again */
    if (bStrippedBackslash && (ntStatus == STATUS_NOT_A_DIRECTORY || ntStatus == STATUS_INVALID_PARAMETER))
    {
        /* put back slash for a single NtOpenFile() call */
        usNtPathName.Length += sizeof(WCHAR);
        ntStatus = pfnNtOpenFile(
            &hDirectory,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            &oa,
            &iostat,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT
        );
        usNtPathName.Length -= sizeof(WCHAR);
    }

    if (!NT_SUCCESS(ntStatus))
    {
        pfnRtlFreeUnicodeString(&usNtPathName);
        pfnRtlFreeUnicodeString(&usPartName);
        return ntStatus;
    }

    _TRACE(2, "Opened directory %wZ as %p", &usNtPathName, hDirectory);

    if (usPartName.Buffer)
    {
        const WCHAR szWildcardAll[] = L"*.*";

        if (0 == memcmp(usPartName.Buffer, szWildcardAll, _countof(szWildcardAll)))
        {
            /* compress to single asterisk */
            usPartName.Buffer[1] = 0;
            usPartName.Length = sizeof(WCHAR);
        }
    }

    *phDirectory = hDirectory; /* it's on the caller now to close this handle */
    *pusNtPathName = usNtPathName; /* it's on the caller now to free this */
    *pusPartName = usPartName; /* it's on the caller now to free this */

    return ntStatus;

}

/* returns offset to next entry (or 0) */
FORCEINLINE static void populateFindFileData_(_Out_writes_bytes_(sizeof(NTFIND_DATA)) NTFIND_DATA* lpFindFileData, _In_reads_bytes_(sizeof(NTFIND_DIR_INFO)) NTFIND_DIR_INFO* dirinfo)
{
    /* populate the find data */
    DWORD dwAttr = lpFindFileData->dwFileAttributes = dirinfo->FileAttributes;
    lpFindFileData->ftCreationTime = *(FILETIME*)&dirinfo->CreationTime;
    lpFindFileData->ftLastAccessTime = *(FILETIME*)&dirinfo->LastAccessTime;
    lpFindFileData->ftLastWriteTime = *(FILETIME*)&dirinfo->LastWriteTime;
    lpFindFileData->nAllocSize = dirinfo->AllocationSize.QuadPart;
    lpFindFileData->nFileSize = dirinfo->EndOfFile.QuadPart;
    lpFindFileData->dwReparseTag = (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? dirinfo->EaSize : 0;

    /* TBD: should we perhaps check FileNameLength against the (hardcoded) size of cFileName? */
    (void)memmove(&lpFindFileData->cFileName, &dirinfo->FileName, dirinfo->FileNameLength);
    lpFindFileData->cFileName[dirinfo->FileNameLength / sizeof(WCHAR)] = 0; /* zero-terminate the string */
}

_Success_(return != INVALID_HANDLE_VALUE)
EXTERN_C HANDLE WINAPI NativeFindFirstFile(_In_z_ LPCWSTR lpszPathName, _Out_writes_bytes_(sizeof(NTFIND_DATA)) NTFIND_DATA* lpFindFileData, ULONG dwFlags)
{
    UNICODE_STRING usNtPathName = { 0, 0, NULL }, usPartName = { 0, 0, NULL };
    NTSTATUS ntStatus;
    HANDLE hDirectory;
    NTFIND_HANDLE* pFindHandle;
    NTFIND_DIR_INFO* dirinfo;

    if (!lpFindFileData)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    /* Attempt to open the given path */
    ntStatus = openDirectoryForSearch_(&hDirectory, NULL, lpszPathName, &usNtPathName, &usPartName, (BOOLEAN)(0 != (dwFlags & NTFIND_CASE_SENSITIVE)));

    if (!NT_SUCCESS(ntStatus))
    {
        SetLastErrorFromNtError(ntStatus);
        return INVALID_HANDLE_VALUE;
    }

    /* If successful, this transfers ownership of usNtPathName's storage and hDirectory to pFindHandle */
    pFindHandle = initFindHandle_(hDirectory, &usNtPathName, &usPartName, dwFlags);
    if (!pFindHandle)
    {
        pfnRtlFreeUnicodeString(&usNtPathName);
        pfnRtlFreeUnicodeString(&usPartName);
        (void)pfnNtClose(hDirectory);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    /* point to first entry */
    dirinfo = getNextDirInfoEntry_(pFindHandle);

    if (!dirinfo)
    {
        /* Copy last NTSTATUS we saw */
        ntStatus = pFindHandle->ntStatus;
        (void)freeFindHandle_(pFindHandle);
        SetLastErrorFromNtError(ntStatus);
        return INVALID_HANDLE_VALUE;
    }

    populateFindFileData_(lpFindFileData, dirinfo);

    return (HANDLE)pFindHandle;
}

_Success_(return != FALSE)
EXTERN_C BOOL WINAPI NativeFindNextFile(_In_ HANDLE hFindFile, _Out_writes_bytes_(sizeof(NTFIND_DATA)) NTFIND_DATA* lpFindFileData)
{
    NTFIND_HANDLE* pFindHandle = (NTFIND_HANDLE*)hFindFile;
    BOOL bRetVal = FALSE;
    if (INVALID_HANDLE_VALUE == hFindFile || !hFindFile)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return bRetVal;
    }
    EnterCriticalSection(&pFindHandle->csFindHandle);
    __try
    {
        NTFIND_DIR_INFO* dirinfo = getNextDirInfoEntry_(pFindHandle);
        if (dirinfo)
        {
            /* simply copy an entry from our buffer, if there is more data */
            populateFindFileData_(lpFindFileData, dirinfo);
            bRetVal = TRUE;
        }
        else
        {
            SetLastErrorFromNtError(pFindHandle->ntStatus);
        }
    }
    __finally
    {
        LeaveCriticalSection(&pFindHandle->csFindHandle);
    }
    return bRetVal;
}

_Success_(return != FALSE)
EXTERN_C BOOL NativeFindClose(_Frees_ptr_ HANDLE hFindFile)
{
    if (INVALID_HANDLE_VALUE == hFindFile || !hFindFile)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    __try
    {
        NTFIND_HANDLE* pFindHandle = (NTFIND_HANDLE*)hFindFile;
        return freeFindHandle_(pFindHandle);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastErrorFromNtError(GetExceptionCode());
        return FALSE;
    }
}

EXTERN_C NTSTATUS WINAPI NativeFindLastStatus(_In_ HANDLE hFindFile)
{
    NTFIND_HANDLE* pFindHandle = (NTFIND_HANDLE*)hFindFile;
    if (pFindHandle)
    {
        return pFindHandle->ntStatus;
    }
    return STATUS_INVALID_PARAMETER;
}

typedef struct _WRAPPED_CONTEXT
{
    NTFIND_CALLBACK_CONTEXT cbctx;
    NativeFindZeroCopyEntryCallback_t pfnEntry;
    NativeFindZeroCopyDrillDownCallback_t pfnDrillDown;
} WRAPPED_CONTEXT;

__inline static NTSTATUS recurseIntoDirectory_(_In_opt_ HANDLE hParentDir, _In_ WRAPPED_CONTEXT const* wctx)
{
    UNICODE_STRING usNtPathName = { 0, 0, NULL }, usPartName = { 0, 0, NULL };
    HANDLE hCurrentDir;
    NTSTATUS ntStatus;
    NTFIND_HANDLE* pFindHandle;
    NTFIND_DIR_INFO* dirinfo;

    /* attempt to open the given path */
    ntStatus = openDirectoryForSearch_(&hCurrentDir, hParentDir, wctx->cbctx.lpszPathName, &usNtPathName, &usPartName, (BOOLEAN)(0 != (wctx->cbctx.dwFlags & NTFIND_CASE_SENSITIVE)));

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

#ifdef _DEBUG
    if (wctx->cbctx.pParentContext)
    {
        _TRACE(1, "'%ws\\%wZ' == %08p", wctx->cbctx.pParentContext->lpszPathName, &usNtPathName, hCurrentDir);
    }
    else
    {
        _TRACE(1, "'\\%wZ' == %08p", &usNtPathName, hCurrentDir);
    }
#endif /* _DEBUG */

    /* if successful, this transfers ownership of usNtPathName's storage and hCurrentDir to pFindHandle */
    pFindHandle = initFindHandle_(hCurrentDir, &usNtPathName, &usPartName, wctx->cbctx.dwFlags);
    if (!pFindHandle)
    {
        pfnRtlFreeUnicodeString(&usNtPathName);
        pfnRtlFreeUnicodeString(&usPartName);
        (void)pfnNtClose(hCurrentDir);
        return STATUS_NO_MEMORY;
    }

    /* pFindHandle now owns the usNtPathName::Buffer, usPartName::Buffer and hCurrentDir */
    while (NULL != (dirinfo = getNextDirInfoEntry_(pFindHandle)))
    {
        ntStatus = wctx->pfnEntry(&wctx->cbctx, dirinfo);
        if (!NT_SUCCESS(ntStatus))
        {
            if (STATUS_NTFIND_SKIP_RECURSION == ntStatus)
            {
                continue;
            }
            _TRACE(2, "Callback returned non-success code 0x%08X", ntStatus);
            (void)freeFindHandle_(pFindHandle);
            return ntStatus;
        }
        if ((NTFIND_SKIP_REPARSE_DIRECTORIES & pFindHandle->dwFlags) && FILE_ATTRIBUTE_REPARSE_DIR == (FILE_ATTRIBUTE_REPARSE_DIR & dirinfo->FileAttributes))
        {
            continue;
        }
        if ((NTFIND_RECURSIVE & pFindHandle->dwFlags) && (FILE_ATTRIBUTE_DIRECTORY & dirinfo->FileAttributes))
        {
            NTSTATUS ntTempStatus;
            const ULONG cbNeeded = ((usPartName.Length) ? usPartName.Length : sizeof(WCHAR)) /* for an asterisk or whatever search mask was passed */
                + dirinfo->FileNameLength
                + sizeof(WCHAR) * 2; /* we need one backslash and a terminating zero */
            LPWSTR lpszSubdirPathName;
            WRAPPED_CONTEXT wctxSubDir = {
                {
                    sizeof(NTFIND_CALLBACK_CONTEXT),
                    0,
                    NULL,
                    NULL,
                    NULL,
                },
                0,
            };
            ntStatus = STATUS_REQUEST_ABORTED;
            if (!wctx->pfnDrillDown(ntfcbrNewContext, &wctx->cbctx, dirinfo, &wctxSubDir.cbctx.lpCallerContext, &ntStatus))
            {
                _TRACE(2, "Callback (NewContext) decision: abort with status code 0x%08X", ntStatus);
                (void)freeFindHandle_(pFindHandle);
                return ntStatus;
            }
            lpszSubdirPathName = (LPWSTR)NativeFindAlloc(cbNeeded);
            if (lpszSubdirPathName)
            {
                size_t Index = 0;
                memcpy(lpszSubdirPathName, dirinfo->FileName, dirinfo->FileNameLength);
                Index += dirinfo->FileNameLength / sizeof(WCHAR);
                if (usPartName.Length)
                {
                    memcpy(&lpszSubdirPathName[Index], L"\\", sizeof(WCHAR));
                    Index++;
                    memcpy(&lpszSubdirPathName[Index], usPartName.Buffer, usPartName.Length);
                }
                else
                {
                    memcpy(&lpszSubdirPathName[Index], L"\\*", 2 * sizeof(WCHAR));
                }
                wctxSubDir.cbctx.lpszPathName = lpszSubdirPathName;
            }
            else
            {
                ntStatus = STATUS_NO_MEMORY;
                (void)wctx->pfnDrillDown(ntfcbrFreeContext, &wctx->cbctx, dirinfo, &wctxSubDir.cbctx.lpCallerContext, &ntTempStatus);
                _TRACE(2, "Recursive descent failed during preparation: out of memory (0x%08X)", ntStatus);
                if (!wctx->pfnDrillDown(ntfcbrRecurseError, &wctx->cbctx, dirinfo, NULL, &ntStatus))
                {
                    _TRACE(2, "Callback (RecurseError) decision: abort with status code 0x%08X", ntStatus);
                    (void)freeFindHandle_(pFindHandle);
                    return ntStatus;
                }
            }
            wctxSubDir.cbctx.dwFlags = wctx->cbctx.dwFlags;
            wctxSubDir.cbctx.pParentContext = &wctx->cbctx;
            wctxSubDir.pfnEntry = wctx->pfnEntry;
            wctxSubDir.pfnDrillDown = wctx->pfnDrillDown;
            _TRACE(0, "Recurse into: '%ws' ['%wZ' == %08p]", dirinfo->FileName, &usNtPathName, hCurrentDir);
            ntStatus = recurseIntoDirectory_(pFindHandle->hDirectory, &wctxSubDir);
            (void)NativeFindFreeAndNull(lpszSubdirPathName);
            (void)wctx->pfnDrillDown(ntfcbrFreeContext, &wctx->cbctx, dirinfo, &wctxSubDir.cbctx.lpCallerContext, &ntTempStatus);
            if (!NT_SUCCESS(ntStatus))
            {
                _TRACE(2, "Recursive descent returned non-success status code 0x%08X", ntStatus);
                if (!wctx->pfnDrillDown(ntfcbrRecurseError, &wctx->cbctx, dirinfo, NULL, &ntStatus))
                {
                    _TRACE(2, "Callback (RecurseError) decision: abort with status code 0x%08X", ntStatus);
                    (void)freeFindHandle_(pFindHandle);
                    return ntStatus;
                }
            }
        }
    }
    _TRACE(1, "+++++++++++++++ Needed %I64d additional queries for %wZ", pFindHandle->llAdditionalQueriesNeeded, &pFindHandle->usNtPathName);
    (void)freeFindHandle_(pFindHandle);

    return STATUS_SUCCESS;
}

EXTERN_C NTSTATUS WINAPI NativeFindFilesZeroCopy(_In_ NTFIND_CALLBACK_CONTEXT const* ctx, _In_ NativeFindZeroCopyEntryCallback_t pfnEntry, _In_ NativeFindZeroCopyDrillDownCallback_t pfnDrillDown)
{
    WRAPPED_CONTEXT wctx;

    if (
        !pfnEntry
        || !pfnDrillDown
        || !ctx || (ctx->cbSize != sizeof(*ctx))
        || (ctx->pParentContext != NULL)
        || (ctx->lpszPathName == NULL)
        )
    {
        return STATUS_INVALID_PARAMETER;
    }

    wctx.cbctx = *ctx;
    wctx.pfnEntry = pfnEntry;
    wctx.pfnDrillDown = pfnDrillDown;
    return recurseIntoDirectory_(NULL, &wctx);
}
