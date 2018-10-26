///////////////////////////////////////////////////////////////////////////////
///
/// Delay-load helper for ntobjx.
///   This code allows us to delay resolving of functions from ntdll.dll to
///   runtime by using a little trick.
///
/// Dual-licensed under MS-PL and MIT license (see below).
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016 Oliver Schneider (assarbad.net)
///
/// Permission is hereby granted, free of charge, to any person obtaining a
/// copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation
/// the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
///////////////////////////////////////////////////////////////////////////////

#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#if defined(_CONSOLE) || defined(_CONSOLE_APP)
#   include <Windows.h>
#   include <tchar.h>
#   define DelayLoadError _tprintf
#else
#   include "stdafx.h" /* Must alias DelayLoadError to a function that behaves like _tprintf */
#endif
#ifndef FACILITY_VISUALCPP
#   define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif // !FACILITY_VISUALCPP
#pragma warning(disable:4201)
#include <delayimp.h>
#pragma warning(default:4201)

#pragma comment(lib, "delayimp")

#ifdef __cplusplus
namespace
{
#endif // __cplusplus
    LPCSTR lpszNtDllName = "ntdll.dll";
    LPCSTR lpszNtDelayedDllName = "ntdll-delayed.dll";
#ifdef __cplusplus
}
#endif // __cplusplus

EXTERN_C NTSTATUS NTAPI FallbackRtlDosPathNameToNtPathName_U_WithStatus(PCWSTR DosFileName, PVOID NtFileName, PVOID *FilePart, PVOID RelativeName);

static LONG WINAPI DelayLoadFilter(PEXCEPTION_POINTERS pExcPointers)
{
    LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;
    PDelayLoadInfo pdli = (PDelayLoadInfo)(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

    switch(pExcPointers->ExceptionRecord->ExceptionCode)
    {
    case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
        (void)DelayLoadError(_T("The DLL %hs could not be loaded."), pdli->szDll);
        break;
    case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
        {
            LPCSTR lpszDllName = pdli->szDll;
            if(0 == lstrcmpiA(pdli->szDll, lpszNtDelayedDllName))
            {
                lpszDllName = lpszNtDllName;
            }
            if (pdli->dlp.fImportByName)
            {
                (void)DelayLoadError(_T("Function %hs::%hs not found."), lpszDllName, pdli->dlp.szProcName);
            }
            else
            {
                (void)DelayLoadError(_T("Function %hs::#%u not found."), lpszDllName, pdli->dlp.dwOrdinal);
            }
        }
        break;
    default:
        lDisposition = EXCEPTION_CONTINUE_SEARCH;
        break;
    }
    return lDisposition;
}

EXTERN_C void force_resolve_all(void)
{
    __try
    {
        /* Force all delay-loaded symbols to be resolved at once */
        (void)__HrLoadAllImportsForDll(lpszNtDelayedDllName);
    }
    __except(DelayLoadFilter(GetExceptionInformation()))
    {
        ExitProcess(1);
    }

}

static FARPROC WINAPI NtdllDliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    switch(dliNotify)
    {
    case dliNotePreLoadLibrary:
        if(0 == lstrcmpiA(pdli->szDll, lpszNtDelayedDllName))
        {
            HMODULE hNtDll;
            if(NULL != (hNtDll = GetModuleHandleA(lpszNtDllName)))
            {
                /*lint -save -e611 */
#               pragma warning(suppress:4055)
                return (FARPROC)(hNtDll);
                /*lint -restore */
            }
        }
        break; /* proceed with default processing  */
    case dliNotePreGetProcAddress:
        if (pdli->dlp.fImportByName && (0 == lstrcmpiA("RtlDosPathNameToNtPathName_U_WithStatus", pdli->dlp.szProcName)))
        {
            FARPROC func = GetProcAddress(pdli->hmodCur, pdli->dlp.szProcName);
            if (!func)
            {
                func = (FARPROC)FallbackRtlDosPathNameToNtPathName_U_WithStatus;
            }
            return func;
        }
        break; /* proceed with default processing  */
    default:
        break;
    }
    return NULL;
}

PfnDliHook __pfnDliNotifyHook2 = NtdllDliHook;
