/******************************************************************************
 ***
 *** Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
 ***
 *** Purpose          : Functions similar to FindFirstFile/FindNextFile to
 ***                    enumerate files and directories (also FindClose ;-)).
 ***
 ******************************************************************************/
#ifndef __NTFINDFILE_H_VER__
#define __NTFINDFILE_H_VER__ 2018102320
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif /* Check for #pragma support */

#include <Windows.h>
#include <tchar.h>
#include "ntnative.h"

#ifdef NTFIND_USE_CRTALLOC
#   ifdef NTFIND_ALLOC_FUNC
#       undef NTFIND_ALLOC_FUNC
#       pragma message (__FILE__ " warning: undefining NTFIND_ALLOC_FUNC because NTFIND_USE_CRTALLOC takes precedence")
#   endif /* NTFIND_ALLOC_FUNC */
#   ifdef NTFIND_FREE_FUNC
#       undef NTFIND_FREE_FUNC
#       pragma message (__FILE__ " warning: undefining NTFIND_FREE_FUNC because NTFIND_USE_CRTALLOC takes precedence")
#   endif /* NTFIND_FREE_FUNC */
#endif /* NTFIND_USE_CRTALLOC */

typedef LPVOID(*ntAlloc_t)(SIZE_T);
typedef BOOL(*ntFree_t)(LPVOID);

/* Override by defining (CPP) NTFIND_ALLOC_FUNC and NTFIND_FREE_FUNC as functions matching above prototypes */
EXTERN_C ntAlloc_t NativeFindAlloc;
EXTERN_C ntFree_t NativeFindFree;

 /* adjust these two items if you want to query another info class */
#ifndef NTFIND_DIR_INFO
#   define NTFIND_DIR_INFO FILE_FULL_DIR_INFORMATION
#endif /* NTFIND_DIR_INFO */
#ifndef NtFindInfoClass
#   define NtFindInfoClass FileInformationFullDirectory
#endif /* NtFindInfoClass */

/*
  Possible pairings:

    FILE_FULL_DIR_INFORMATION -> FileInformationFullDirectory (default)
    FILE_DIRECTORY_INFORMATION -> FileInformationDirectory
    FILE_BOTH_DIR_INFORMATION -> FileInformationBothDirectory
    FILE_ID_FULL_DIR_INFORMATION ->FileInformationIdFullDirectory
    FILE_ID_BOTH_DIR_INFORMATION ->FileInformationIdBothDirectory

  More exist, check out ntnative.h

  STATUS_INVALID_INFO_CLASS will be returned by the OS if one isn't supported.
*/

/******************************************************************************
  adjust this to use larger buffers for querying the entries from a directory

  mind the fact that directory traversal typically happens recursively, so this
  limit should be raised VERY CAREFULLY, if at all.
 ******************************************************************************/
#ifndef NTFIND_LARGE_BUFFER_SIZE
#   ifdef _DEBUG /* for debug we want to see the STATUS_BUFFER_OVERFLOW logic */
#       define NTFIND_LARGE_BUFFER_SIZE (sizeof(NTFIND_DIR_INFO))
#   else
#       define NTFIND_LARGE_BUFFER_SIZE (0x1000U * sizeof(void*))
#   endif /* _DEBUG */
#endif /* NTFIND_LARGE_BUFFER_SIZE */
#define NTFIND_LARGE_BUFFER_MAXSIZE ((1U << 21U) * (sizeof(void*) / sizeof(int)))
/* limits depend on the bitness we build for */

typedef struct _NTFIND_HANDLE
{
#ifdef _DEBUG
    LONGLONG llAdditionalQueriesNeeded;
#endif /* _DEBUG */
    HANDLE hDirectory;
    ULONG dwFlags;
    ULONG cbBuffer;
    ULONG uNextNextEntryOffset;
    PUCHAR pucBuffer;
    PUCHAR pucNextEntry;
    CRITICAL_SECTION csFindHandle;
    UNICODE_STRING usNtPathName;
    UNICODE_STRING usPartName;
    IO_STATUS_BLOCK iostat_NtQueryDirectoryFile;
    NTSTATUS ntStatus;
    BOOL bScanStarted;
    BOOL bMaxQueryBufferSizeReached;
    BOOL bNoMoreFiles;
} NTFIND_HANDLE;

#define NTFIND_SUPPRESS_SELF_AND_PARENT     0x00000001U
#define NTFIND_CASE_SENSITIVE               0x00000002U
/* The items below have no effect as of yet FIXME/TODO */
#define NTFIND_ENUM_ADS                     0x00000004U /* this may considerably slow down a directory traversal! */
#define NTFIND_SKIP_DATASTREAM              0x00000008U
#define NTFIND_RECURSIVE                    0x00000010U
#define NTFIND_SKIP_REPARSE_DIRECTORIES     0x00000020U /* if you don't give this, you _must_ handle this condition yourself in the NewContext callback */
#define NTFIND_ENUM_ADS_TYPICAL            (NTFIND_ENUM_ADS | NTFIND_SKIP_DATASTREAM) /* this may considerably slow down a directory traversal! */
/* ATTENTION: if you ignore to set NTFIND_SKIP_REPARSE_DIRECTORIES and don't handle
   it appropriately when asked for a new context, either, you will find yourself
   in an infinite recursion ... well, I suppose it won't be infinite since you'll
   bust the stack eventually. So heed my warning! */

#define FACILITY_NTFIND 0x666 /* not yet used */
#define __NTFIND_ERROR(x) ((NTSTATUS) (((x) & 0x0000FFFF) | (FACILITY_NTFIND << 16) | ERROR_SEVERITY_ERROR))
#define STATUS_NTFIND_SKIP_RECURSION __NTFIND_ERROR(1)


/******************************************************************************
  An alternative version of WIN32_FIND_DATA.
  Used for NativeFindFirstFile and NativeFindNextFile.
 ******************************************************************************/
typedef struct _NTFIND_DATA
{
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    LONGLONG nFileSize;
    LONGLONG nAllocSize;
    DWORD dwFileAttributes;
    DWORD dwReparseTag;
    _Field_z_ WCHAR cFileName[MAX_PATH];
} NTFIND_DATA, *PNTFIND_DATA;


/******************************************************************************
  A function that initializes function pointers to functions from ntdll.dll
  when NTFINDFILE_DYNAMIC is defined.
  If NTFINDFILE_DYNAMIC has not been defined, it is optional to call this
  function. But calling it allows to override the initial buffer size held by
  a search handle (the one returned by NativeFindFirstFile).
 ******************************************************************************/
_Success_(return != FALSE)
EXTERN_C BOOLEAN NativeFindInit(_In_ ULONG cbInitialBuffer);

/******************************************************************************
  A function that mimics FindFirstFile, but not very closely. It mainly exists
  in order to make use of this library easier when FindFirstFile and
  FindNextFile have been used before.
  The lpszPathName should contain the search mask.
  Note: this function follows NT semantics for path matching (not DOS rules)!
 ******************************************************************************/
_Success_(return != INVALID_HANDLE_VALUE)
EXTERN_C HANDLE WINAPI NativeFindFirstFile(_In_z_ LPCWSTR lpszPathName, _Out_writes_bytes_(sizeof(NTFIND_DATA)) NTFIND_DATA* lpFindFileData, ULONG dwFlags);

/******************************************************************************
  A function that mimics FindNextFile, but not very closely. It mainly exists
  in order to make use of this library easier when FindFirstFile and
  FindNextFile have been used before.
  Note: this function follows NT semantics for path matching (not DOS rules)!
 ******************************************************************************/
_Success_(return != FALSE)
EXTERN_C BOOL WINAPI NativeFindNextFile(_In_ HANDLE hFindFile, _Out_writes_bytes_(sizeof(NTFIND_DATA)) NTFIND_DATA* lpFindFileData);

/******************************************************************************
  A function that mimics FindCloseFile, but not very closely. It mainly exists
  in order to make use of this library easier when FindFirstFile,
  FindNextFile and FindCloseFile have been used before.
  This function frees the memory associated with the search handle.
 ******************************************************************************/
_Success_(return != FALSE)
EXTERN_C BOOL NativeFindClose(_Frees_ptr_ HANDLE hFindFile);

/******************************************************************************
  A function that retrieves the last seen NTSTATUS which is held inside the
  search handle.
 ******************************************************************************/
EXTERN_C NTSTATUS WINAPI NativeFindLastStatus(_In_ HANDLE hFindFile);

#ifdef _DEBUG
/******************************************************************************
  A function that returns the amount of Bytes currently allocated by
  NativeFindFirstFile, NativeFindNextFile and freed by FindCloseFile.
 ******************************************************************************/
EXTERN_C LONGLONG NativeFindGetAllocatedBytes();
#endif /* _DEBUG */

/******************************************************************************
  TODO/FIXME
 ******************************************************************************/
 typedef struct _NTFIND_CALLBACK_CONTEXT
{
    ULONG cbSize;
    ULONG dwFlags; /* copied during initialization */
    struct _NTFIND_CALLBACK_CONTEXT const* pParentContext; /* useful whenever you recurse down into directories */
    LPCWSTR lpszPathName;
    LPVOID lpCallerContext;
} NTFIND_CALLBACK_CONTEXT;

 /******************************************************************************
   Prototype for the callback function used in NativeFindFilesZeroCopy to signal
   a new directory entry (be it a file or directory or whatever).

   NB: The file name is guaranteed to be zero-terminated!
  ******************************************************************************/

typedef NTSTATUS (WINAPI *NativeFindZeroCopyEntryCallback_t)(_In_ NTFIND_CALLBACK_CONTEXT const* ctx, _In_ NTFIND_DIR_INFO* dirinfo);

typedef enum _NtFindCallbackReason
{
    ntfcbrNewContext, /* we're about to recurse into a subdirectory and are asking the caller for a new caller-provided context, callee is supposed to fill *pCallerCtx */
    ntfcbrFreeContext, /* ask the caller if the context should be freed, hand them the context pointer they gave us */
    ntfcbrRecurseError, /* there was an error during the attempt to recurse into a subdirectory */
} NtFindCallbackReason;
/******************************************************************************
  Prototype for the callback function used in NativeFindFilesZeroCopy to signal
  a subdirectory is about to be entered, but also to notify the callee of errors
  during the attempt to enter a directory (such as STATUS_ACCESS_DENIED) and
  give the chance to abort (by returning FALSE instead of TRUE).

  In case of ntfcbrNewContext the callee is expected to fill the new context
  pointer (or leave it set to NULL) and return TRUE to request the recursion to
  proceed. Furthermore the callee should fill *ntStatus, which is the return
  code to the caller of NativeFindFilesZeroCopy() _if_ the callback returns
  FALSE!
  If left alone the status code will be set to STATUS_REQUEST_ABORTED.
  NB: If you use this method to filter infinite recursion via reparse points,
  please returnset *ntStatus=STATUS_NTFIND_SKIP_RECURSION and return FALSE!

  In case of ntfcbrFreeContext the callee is expected to free the context
  pointer (or it be). If you leave it, you should be very sure of what you're
  doing (such as having it in some linked list that will later get freed) or you
  risk a memory leak. The return code with this reason code is ignored.
  NB: make sure to set *pCallerCtx=NULL if you happened to free/release it.

  In case of ntfcbrRecurseError the callback can decide to continue regardless
  of the error code (to be found in *ntStatus) and return TRUE or indicate to
  abort the traversal with FALSE and optionally modify the status code.

  The adjusted status codes should follow the HRESULT/NTSTATUS layout and be
  compatible with the SUCCEEDED/FAILED and NT_SUCCESS macros as well as with
  those to extract the code and the facility and severity. However, the library
  will only care if the code is an error.
 ******************************************************************************/
typedef BOOL (WINAPI *NativeFindZeroCopyDrillDownCallback_t)(NtFindCallbackReason reason, _In_ NTFIND_CALLBACK_CONTEXT const* ctx, _In_ NTFIND_DIR_INFO* dirinfo, _Inout_updates_opt_(sizeof(LPVOID)) LPVOID* pCallerCtx, NTSTATUS* ntStatus);

 /******************************************************************************
  This function allows to recursively enumerate files and directories inside the
  directory described by NTFIND_CALLBACK_CONTEXT::lpszPathName.

  NB: the above callbacks are _not_ called for the root item given, they are
  only ever called for files and subdirectories underneath.
  ******************************************************************************/
EXTERN_C NTSTATUS WINAPI NativeFindFilesZeroCopy(_In_ NTFIND_CALLBACK_CONTEXT const* ctx, _In_ NativeFindZeroCopyEntryCallback_t pfnEntry, _In_ NativeFindZeroCopyDrillDownCallback_t pfnDrillDown);

#endif /* __NTFINDFILE_H_VER__ */
