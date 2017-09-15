///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Original filename: lookfs.cpp
/// Project          : lookfs
/// Date of creation : 2009-02-03
/// Author(s)        : Oliver Schneider
///
/// Purpose          : Program to investigate all kinds of file system artifacts
///                    on Windows 2000 and later.
///
///////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#ifdef _DEBUG
#include <crtdbg.h>
#endif // _DEBUG
#include <tchar.h>
#include "lookfs.hpp"
#include "VersionInfo.hpp"

namespace {
    BOOL isCommandLineSwitch(_TCHAR *arg, const _TCHAR *switchName = NULL)
    {
        if(arg && wcslen(arg) > 2 && arg[0] == L'-' && arg[1] == L'-')
        {
            if(!switchName)
            {
                return TRUE;
            }
            if(0 == wcscmp(&arg[2], switchName))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    HMODULE GetInstanceHandle()
    {
        MEMORY_BASIC_INFORMATION mbi;
        static int iDummy;
        ::VirtualQuery(&iDummy, &mbi, sizeof(mbi));
        return HMODULE(mbi.AllocationBase);
    }
}

LPCTSTR getReparseType(const CReparsePoint& rp)
{
    if(!rp.isReparsePoint())
        return _T("");
    switch(rp.ReparseTag())
    {
    case IO_REPARSE_TAG_MOUNT_POINT:
        return _T("volume mount point");
    case IO_REPARSE_TAG_HSM:
        return _T("HSM reparse point");
    case IO_REPARSE_TAG_HSM2:
        return _T("HSM2 reparse point");
    case IO_REPARSE_TAG_SIS:
        return _T("SIS reparse point");
    case IO_REPARSE_TAG_WIM:
        return _T("WIM reparse point");
    case IO_REPARSE_TAG_CSV:
        return _T("CSV reparse point");
    case IO_REPARSE_TAG_DFS:
        return _T("DFS reparse point");
    case IO_REPARSE_TAG_SYMLINK:
        return _T("symbolic link");
    case IO_REPARSE_TAG_DFSR:
        return _T("DFSR reparse point");
    case IO_REPARSE_TAG_DEDUP:
        return _T("DEDUP reparse point");
    case IO_REPARSE_TAG_NFS:
        return _T("NFS reparse point");
    case IO_REPARSE_TAG_FILE_PLACEHOLDER:
        return _T("placeholder file");
    default:
        return _T("reparse point");
    }
}

int __cdecl _tmain(int argc, _TCHAR *argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);
#endif // _DEBUG
    if(2 <= argc)
    {
        bool show_logo = true, be_verbose = false, noerror = false;
        int fname_arg = 1;
        while(isCommandLineSwitch(argv[fname_arg]))
        {
            if(isCommandLineSwitch(argv[fname_arg], L"nologo"))
                show_logo = false;
            else
                if(isCommandLineSwitch(argv[fname_arg], L"verbose"))
                    be_verbose = true;
                else
                    if(isCommandLineSwitch(argv[fname_arg], L"noerror"))
                        noerror = true;
            fname_arg++;
        }

        if(show_logo)
        {
            CVersionInfo verinfo(GetInstanceHandle());
            _tprintf(_T("%s %s written by %s\n")
                , verinfo[_T("OriginalFilename")]
            , verinfo[_T("FileVersion")]
            , verinfo[_T("CompanyName")]
            );
#ifdef HG_REV_ID
            _tprintf(_T("\tRevision: %s\n")
                , verinfo[_T("Mercurial revision")]
            );
#endif
            _tprintf(_T("\n"));
        }

        CReparsePoint rp(argv[fname_arg]);
        if(ERROR_SUCCESS != rp.LastError())
        {
            _ftprintf(stderr, _T("ERROR: Win32 error code %d. Probably file (2) or path (3) not found or access issue (5)?!\n"), rp.LastError());
            return 3;
        }
        if(rp.isReparsePoint())
        {
            _tprintf(_T("'%ws' is a %sMicrosoft %s (virt == %d)\n"),
                rp.Path(),
                ((rp.isMicrosoftTag()) ? _T("") : _T("non-")),
                getReparseType(rp),
                rp.isVirtual()
                );
            if(be_verbose)
            {
#ifdef RP_QUERY_FILE_ID
                if(-1 != rp.FileIndex())
                {
                    _tprintf(_T("\tFile Index: %I64u (%08X%08X)\n"), rp.FileIndex(), rp.FileIndexHigh(), rp.FileIndexLow());
                }
#endif // RP_QUERY_FILE_ID
            }
            if(rp.isNameSurrogate())
            {
                _tprintf(_T("\tPrint name: %ws\n"), rp.PrintName());
                _tprintf(_T("\tSubst name: %ws\n"), rp.SubstName());
                _tprintf(_T("\tSubst name: %ws (w32)\n"), rp.CanonicalSubstName());
            }
            if(!rp.isMicrosoftTag() || be_verbose)
            {
                if(be_verbose)
                {
                    _tprintf(_T("\tTag       : %08X\n"), rp.ReparseTag());
                }
                _tprintf(_T("\tGUID      : {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n"),
                    rp.ReparseGuid().Data1,
                    rp.ReparseGuid().Data2,
                    rp.ReparseGuid().Data3,
                    rp.ReparseGuid().Data4[0],
                    rp.ReparseGuid().Data4[1],
                    rp.ReparseGuid().Data4[2],
                    rp.ReparseGuid().Data4[3],
                    rp.ReparseGuid().Data4[4],
                    rp.ReparseGuid().Data4[5],
                    rp.ReparseGuid().Data4[6],
                    rp.ReparseGuid().Data4[7]
                );
            }
        }
        else
        {
            if(!noerror) // show no errors if asked not to
            {
                _ftprintf(stderr, _T("ERROR: '%ws' is not a reparse point\n"), rp.Path());
            }
            if(be_verbose)
            {
#ifdef RP_QUERY_FILE_ID
                if(-1 != rp.FileIndex())
                {
                    _ftprintf(stderr, _T("\tFile Index: %I64u (%08X%08X)\n"), rp.FileIndex(), rp.FileIndexHigh(), rp.FileIndexLow());
                }
#endif // RP_QUERY_FILE_ID
            }
            return 2;
        }
        return 0;
    }
    else
    {
        _ftprintf(stderr, _T("Syntax:\n\tlookfs [--nologo] [--verbose] [--noerror] <path>\n"));
    }
    return 1;
}
