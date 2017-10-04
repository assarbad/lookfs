///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Purpose          : Functions similar to FindFirstFile/FindNextFile to
///                    enumerate files and directories.
///
///////////////////////////////////////////////////////////////////////////////
#include <tchar.h>
#include <stdio.h>
#include <Windows.h>
#include "ntfindfile.h"
#include "ntnative.h"

#define SetLastErrorFromNtError(x) SetLastError((LONG)RtlNtStatusToDosError(x))

/* adjust these two items if you want to query another info class */
typedef FILE_DIRECTORY_INFORMATION OUR_NATIVE_INFO;
#define OurNativeInfoClass FileInformationDirectory

/* adjust this to use larger buffers for querying the entries from a directory */
#ifndef LARGE_FIND_BUFFER_SIZE
#   ifdef _DEBUG /* for debug we want to see the STATUS_BUFFER_OVERFLOW logic */
#       define LARGE_FIND_BUFFER_SIZE (sizeof(OUR_NATIVE_INFO))
#   else
#       define LARGE_FIND_BUFFER_SIZE (1<<24)
#   endif // _DEBUG
#endif // LARGE_FIND_BUFFER_SIZE

typedef struct _FINDFILE_HANDLE
{
    HANDLE hDirectory;
    PVOID pBuffer;
    ULONG cbBuffer;
    PUCHAR pNextEntry;
    NTSTATUS ntStatus;
    CRITICAL_SECTION csFindHandle;
} FINDFILE_HANDLE;

_Ret_writes_bytes_maybenull_(sizeof(FINDFILE_HANDLE))
_Post_writable_byte_size_(sizeof(FINDFILE_HANDLE))
static FINDFILE_HANDLE* initFindHandle_(_In_ HANDLE hDirectory, ULONG cbInitialBuffer)
{
    FINDFILE_HANDLE* pFindHandle;

    pFindHandle = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pFindHandle));

    if(pFindHandle)
    {
        pFindHandle->hDirectory = hDirectory;
        __try
        {
            InitializeCriticalSection(&pFindHandle->csFindHandle);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastErrorFromNtError(GetExceptionCode());
            (void)HeapFree(GetProcessHeap(), 0, pFindHandle);
            return NULL;
        }
        /* no one knows about this internal buffer, yet, so we needn't lock anything here */
        if(cbInitialBuffer)
        {
            pFindHandle->pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbInitialBuffer);
            if(!pFindHandle->pBuffer)
            {
                pFindHandle->cbBuffer = 0;
                DeleteCriticalSection(&pFindHandle->csFindHandle);
                (void)HeapFree(GetProcessHeap(), 0, pFindHandle);
                return NULL;
            }
            pFindHandle->cbBuffer = cbInitialBuffer;
        }
    }

    return pFindHandle;
}

static BOOLEAN freeFindHandle_(_In_reads_bytes_(sizeof(FINDFILE_HANDLE)) FINDFILE_HANDLE* pFindHandle)
{
    if(pFindHandle)
    {
        if(TryEnterCriticalSection(&pFindHandle->csFindHandle))
        {
            if(pFindHandle->pBuffer)
            {
                HeapFree(GetProcessHeap(), 0, pFindHandle->pBuffer);
                pFindHandle->pBuffer = NULL;
            }
            pFindHandle->cbBuffer = 0;
            pFindHandle->pNextEntry = NULL;
            (void)NtClose(pFindHandle->hDirectory);
            pFindHandle->hDirectory = NULL;
            DeleteCriticalSection(&pFindHandle->csFindHandle);
            return (HeapFree(GetProcessHeap(), 0, pFindHandle)) ? TRUE : FALSE;
        }
    }
    return FALSE;
}

/* doubles the size of the internal buffer ... leaves it zeroed out! (i.e. this is destructive!) */
static BOOLEAN doubleFindHandleBufferSize_(_In_reads_bytes_(sizeof(FINDFILE_HANDLE)) FINDFILE_HANDLE* pFindHandle)
{
    if(pFindHandle)
    {
        if(TryEnterCriticalSection(&pFindHandle->csFindHandle))
        {
            if(pFindHandle->pBuffer)
            {
                const ULONG cbBuffer = pFindHandle->cbBuffer * 2;
                /* we free before allocating a bigger chunk of memory */
                if(!HeapFree(GetProcessHeap(), 0, pFindHandle->pBuffer))
                {
                    LeaveCriticalSection(&pFindHandle->csFindHandle);
                    return FALSE; /* buffer still valid, but no bigger than before */
                }
                pFindHandle->pBuffer = NULL;
                pFindHandle->pNextEntry = NULL;
                /* be cautious about integer overflows */
                if(cbBuffer > pFindHandle->cbBuffer)
                {
                    PVOID pNew = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuffer);
                    if(!pNew)
                    {
                        LeaveCriticalSection(&pFindHandle->csFindHandle);
                        return FALSE; /* we assume this is an OOM condition */
                    }
                    pFindHandle->cbBuffer = cbBuffer;
                    pFindHandle->pBuffer = pNew;
                    LeaveCriticalSection(&pFindHandle->csFindHandle);
                    return TRUE;
                }
                /* integer overflow occurred */
            }
            LeaveCriticalSection(&pFindHandle->csFindHandle);
        }
    }
    return FALSE;
}

/* returns offset to next entry (or 0) */
__inline void populateFindFileData_(NT_FIND_DATA* lpFindFileData, OUR_NATIVE_INFO* dirinfo, FINDFILE_HANDLE* pFindHandle)
{
    /* populate the find data */
    lpFindFileData->dwFileAttributes = dirinfo->FileAttributes;
    lpFindFileData->ftCreationTime = *(FILETIME*)&dirinfo->CreationTime;
    lpFindFileData->ftLastAccessTime = *(FILETIME*)&dirinfo->LastAccessTime;
    lpFindFileData->ftLastWriteTime = *(FILETIME*)&dirinfo->LastWriteTime;
    lpFindFileData->nFileSize = dirinfo->AllocationSize.QuadPart;

    (void)memmove(&lpFindFileData->cFileName, &dirinfo->FileName, dirinfo->FileNameLength);
    lpFindFileData->cFileName[dirinfo->FileNameLength / sizeof(WCHAR)] = 0;

    if(dirinfo->NextEntryOffset)
    {
        pFindHandle->pNextEntry = (PUCHAR)dirinfo + dirinfo->NextEntryOffset;
    }
    else
    {
        pFindHandle->pNextEntry = NULL;
    }
}

/* wraps NtQueryDirectoryFile and handles resizing the buffer inside the find handle */
__inline NTSTATUS WrapNtQueryDirectoryFile_(FINDFILE_HANDLE* pFindHandle, PUNICODE_STRING FileName)
{
    IO_STATUS_BLOCK iostat;
    NTSTATUS ntStatus = NtQueryDirectoryFile(
        pFindHandle->hDirectory,
        NULL,
        NULL,
        NULL,
        &iostat,
        pFindHandle->pBuffer,
        pFindHandle->cbBuffer,
        OurNativeInfoClass,
        FALSE,
        FileName,
        FALSE
    );

    while(STATUS_BUFFER_OVERFLOW == ntStatus)
    {
        /* resize the buffer */
        if(!doubleFindHandleBufferSize_(pFindHandle))
        {
            return STATUS_NO_MEMORY;
        }
        /* retry with bigger buffer */
        ntStatus = NtQueryDirectoryFile(
            pFindHandle->hDirectory,
            NULL,
            NULL,
            NULL,
            &iostat,
            pFindHandle->pBuffer,
            pFindHandle->cbBuffer,
            OurNativeInfoClass,
            FALSE,
            FileName,
            FALSE
        );
    }
    pFindHandle->ntStatus = ntStatus;
    return ntStatus;
}

_Success_(return != INVALID_HANDLE_VALUE && return != NULL)
EXTERN_C HANDLE WINAPI NativeFindFirstFile(_In_z_ LPCTSTR lpszFileName, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData, BOOLEAN bCaseSensitive)
{
    const WCHAR szWildcardAll[] = L"*.*";
    UNICODE_STRING usWin32FileName, usNtFileName;
    NTSTATUS ntStatus;
    RTL_RELATIVE_NAME relName;
    PVOID pBufToFree;
    FINDFILE_HANDLE* pFindHandle;
    USHORT uLen;
    OBJECT_ATTRIBUTES oa;
    HANDLE hDirectory;
    IO_STATUS_BLOCK iostat;
    BOOLEAN bStrippedSlash = FALSE;

    RtlInitUnicodeString(&usWin32FileName, lpszFileName);

    if(!RtlDosPathNameToNtPathName_U(lpszFileName, &usNtFileName, &usWin32FileName.Buffer, &relName))
    {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    pBufToFree = usNtFileName.Buffer;

    if(usWin32FileName.Buffer)
    {
        usWin32FileName.Length = usNtFileName.Length - (USHORT)((ULONG_PTR)usWin32FileName.Buffer - (ULONG_PTR)usNtFileName.Buffer);
    }
    else
    {
        usWin32FileName.Length = 0;
    }
    usWin32FileName.MaximumLength = usWin32FileName.Length;

    if(relName.RelativeName.Length && relName.RelativeName.Buffer != usWin32FileName.Buffer)
    {
        if(usWin32FileName.Buffer)
        {
            usNtFileName.Buffer = relName.RelativeName.Buffer;
            usNtFileName.Length = (USHORT)((ULONG_PTR)usWin32FileName.Buffer - (ULONG_PTR)relName.RelativeName.Buffer);
            usNtFileName.MaximumLength = usNtFileName.Length;
        }
    }
    else
    {
        relName.ContainingDirectory = NULL;
        if(usWin32FileName.Buffer)
        {
            usNtFileName.Length = (USHORT)((ULONG_PTR)usWin32FileName.Buffer - (ULONG_PTR)usNtFileName.Buffer);
            usNtFileName.MaximumLength = usNtFileName.Length;
        }
    }

    uLen = usNtFileName.Length / sizeof(WCHAR);
    if(L':' != usNtFileName.Buffer[uLen - 2] &&  L'\\' != usNtFileName.Buffer[uLen - 1])
    {
        usNtFileName.Length -= sizeof(WCHAR);
        bStrippedSlash = TRUE;
    }

    InitializeObjectAttributes(&oa, &usNtFileName, bCaseSensitive ? 0 : OBJ_CASE_INSENSITIVE, relName.ContainingDirectory, NULL);

    ntStatus = NtOpenFile(
        &hDirectory,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &oa,
        &iostat,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT
    );

    if(bStrippedSlash && (ntStatus == STATUS_NOT_A_DIRECTORY || ntStatus == STATUS_INVALID_PARAMETER))
    {
        /* put back slash for a single NtOpenFile() call */
        usNtFileName.Length += sizeof(WCHAR);
        ntStatus = NtOpenFile(
            &hDirectory,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            &oa,
            &iostat,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT
        );
        usNtFileName.Length -= sizeof(WCHAR);
    }

    if(!NT_SUCCESS(ntStatus))
    {
        HeapFree(GetProcessHeap(), 0, pBufToFree);
        SetLastErrorFromNtError(ntStatus);
        return INVALID_HANDLE_VALUE;
    }

    if(usWin32FileName.Buffer)
    {
        if(0 == memcmp(usWin32FileName.Buffer, szWildcardAll, sizeof(szWildcardAll) - sizeof(szWildcardAll[0])))
        {
            usWin32FileName.Length = sizeof(WCHAR); /* compress to single asterisk */
        }
    }

    pFindHandle = initFindHandle_(hDirectory, LARGE_FIND_BUFFER_SIZE);
    if(!pFindHandle)
    {
        HeapFree(GetProcessHeap(), 0, pBufToFree);
        (void)NtClose(hDirectory);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    ntStatus = WrapNtQueryDirectoryFile_(pFindHandle, &usWin32FileName);
    HeapFree(GetProcessHeap(), 0, pBufToFree);

    if(!NT_SUCCESS(ntStatus))
    {
        (void)freeFindHandle_(pFindHandle);
        SetLastErrorFromNtError(ntStatus);
        return INVALID_HANDLE_VALUE;
    }

    /* point to first entry */
    populateFindFileData_(lpFindFileData, (OUR_NATIVE_INFO*)pFindHandle->pBuffer, pFindHandle);

    return (HANDLE)pFindHandle;
}

_Success_(return != 0)
EXTERN_C BOOL WINAPI NativeFindNextFile(_In_ HANDLE hFindFile, _Out_writes_bytes_(sizeof(NT_FIND_DATA)) NT_FIND_DATA* lpFindFileData)
{
    FINDFILE_HANDLE* pFindHandle = (FINDFILE_HANDLE*)hFindFile;
    BOOL bRetVal = FALSE;
    if(INVALID_HANDLE_VALUE == hFindFile || !hFindFile)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return bRetVal;
    }
    if(TryEnterCriticalSection(&pFindHandle->csFindHandle))
    __try
    {
        /* simply copy an entry from our buffer, if there is more data */
        if(pFindHandle->pNextEntry)
        {
            OUR_NATIVE_INFO* currEntry = (OUR_NATIVE_INFO*)pFindHandle->pNextEntry;
            populateFindFileData_(lpFindFileData, currEntry, pFindHandle);
            bRetVal = TRUE;
        }
        else /* query the next batch of entries */
        {
            IO_STATUS_BLOCK iostat;
            NTSTATUS ntStatus;
            ntStatus = NtQueryDirectoryFile(
                pFindHandle->hDirectory,
                NULL,
                NULL,
                NULL,
                &iostat,
                pFindHandle->pBuffer,
                pFindHandle->cbBuffer,
                OurNativeInfoClass,
                FALSE,
                NULL,
                FALSE
            );
            while(STATUS_BUFFER_OVERFLOW == ntStatus)
            {
                /* resize the buffer */
                if(!doubleFindHandleBufferSize_(pFindHandle))
                {
                    ntStatus = STATUS_NO_MEMORY;
                    break;
                }
                /* retry with bigger buffer */
                ntStatus = NtQueryDirectoryFile(
                    pFindHandle->hDirectory,
                    NULL,
                    NULL,
                    NULL,
                    &iostat,
                    pFindHandle->pBuffer,
                    pFindHandle->cbBuffer,
                    OurNativeInfoClass,
                    FALSE,
                    NULL,
                    FALSE
                );
            }
            pFindHandle->ntStatus = ntStatus;
            if(NT_SUCCESS(ntStatus))
            {
                populateFindFileData_(lpFindFileData, (OUR_NATIVE_INFO*)pFindHandle->pBuffer, pFindHandle);
                bRetVal = TRUE;
            }
            else
            {
                SetLastErrorFromNtError(ntStatus);
            }
        }
    }
    __finally
    {
        LeaveCriticalSection(&pFindHandle->csFindHandle);
    }
    return bRetVal;
}

_Success_(return != 0)
EXTERN_C BOOL NativeFindClose(_In_opt_ HANDLE hFindFile)
{
    if(INVALID_HANDLE_VALUE == hFindFile || !hFindFile)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    __try
    {
        FINDFILE_HANDLE* pFindHandle = (FINDFILE_HANDLE*)hFindFile;
        if(TryEnterCriticalSection(&pFindHandle->csFindHandle))
        {
            LeaveCriticalSection(&pFindHandle->csFindHandle);
            return freeFindHandle_(pFindHandle);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastErrorFromNtError(GetExceptionCode());
        return FALSE;
    }
    return FALSE;
}
