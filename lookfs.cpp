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
#ifdef _DEBUG
#include <cstdlib>
#include <crtdbg.h>
#endif // _DEBUG
#include <cstdio>
#include <tchar.h>
#include "lookfs.hpp"
#include "VersionInfo.hpp"
#include "thirdparty/simpleopt/SimpleOpt.h"

namespace {
    static const TCHAR *
        getLastErrorText(
        int a_nError
        )
    {
        switch(a_nError)
        {
        case SO_SUCCESS:            return _T("Success");
        case SO_OPT_INVALID:        return _T("Unrecognized option");
        case SO_OPT_MULTIPLE:       return _T("Option matched multiple strings");
        case SO_ARG_INVALID:        return _T("Option does not accept argument");
        case SO_ARG_INVALID_TYPE:   return _T("Invalid argument format");
        case SO_ARG_MISSING:        return _T("Required argument is missing");
        case SO_ARG_INVALID_DATA:   return _T("Invalid argument data");
        default:                    return _T("Unknown error");
        }
    }

    static HMODULE getInstanceHandle()
    {
        MEMORY_BASIC_INFORMATION mbi;
        static int iDummy;
        ::VirtualQuery(&iDummy, &mbi, sizeof(mbi));
        return HMODULE(mbi.AllocationBase);
    }

    static LPCTSTR getReparseType(const CReparsePoint& rp)
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
            return _T("SIS (single instance storage) reparse point");
        case IO_REPARSE_TAG_WIM:
            return _T("WIM reparse point");
        case IO_REPARSE_TAG_CSV:
            return _T("CSV reparse point");
        case IO_REPARSE_TAG_DFS:
            return _T("Distributed File System reparse point");
        case IO_REPARSE_TAG_SYMLINK:
            return _T("symbolic link");
        case IO_REPARSE_TAG_DFSR:
            return _T("Distributed File System Replication reparse point");
        case IO_REPARSE_TAG_DEDUP:
            return _T("deduplication reparse point");
        case IO_REPARSE_TAG_NFS:
            return _T("NFS reparse point");
        case IO_REPARSE_TAG_FILE_PLACEHOLDER:
            return _T("file placeholder");
        case IO_REPARSE_TAG_WOF:
            return _T("WOF (WIM external provider) reparse point");
        case IO_REPARSE_TAG_WCI:
            return _T("WCI reparse point");
        case IO_REPARSE_TAG_GLOBAL_REPARSE:
            return _T("global reparse point");
        case IO_REPARSE_TAG_CLOUD:
            return _T("cloud reparse point");
        case IO_REPARSE_TAG_CLOUD_ROOT:
            return _T("cloud root reparse point");
        case IO_REPARSE_TAG_CLOUD_ON_DEMAND:
            return _T("cloud (on-demand) reparse point");
        case IO_REPARSE_TAG_CLOUD_ROOT_ON_DEMAND:
            return _T("cloud root (on-demand) reparse point");
        case IO_REPARSE_TAG_APPEXECLINK:
            return _T("APPEXECLINK reparse point");
        case IO_REPARSE_TAG_GVFS:
            return _T("Git Virtual File System reparse point");
        case IO_REPARSE_TAG_WCI_TOMBSTONE:
            return _T("WCI tombstone reparse point");
        case IO_REPARSE_TAG_UNHANDLED:
            return _T("<unhandled>");
        case IO_REPARSE_TAG_ONEDRIVE:
            return _T("OneDrive reparse point");
        case IO_REPARSE_TAG_GVFS_TOMBSTONE:
            return _T("Git Virtual File System tombstone reparse point");
        case IO_REPARSE_TAG_RESERVED_ZERO:
            return _T("<reserved reparse tag 0>");
        case IO_REPARSE_TAG_RESERVED_ONE:
            return _T("<reserved reparse tag 1>");
        case IO_REPARSE_TAG_DRIVER_EXTENDER:
            return _T("Home server drive extender reparse point");
        case IO_REPARSE_TAG_FILTER_MANAGER:
            return _T("Filter manager test harness reparse point");
        default:
            return _T("<unknown reparse tag>");
        }
    }

    static void showVersion(const CVersionInfo& verinfo)
    {
        _tprintf(
            _T("%s %s written by %s\n")
            , verinfo[_T("OriginalFilename")]
            , verinfo[_T("FileVersion")]
            , verinfo[_T("CompanyName")]
        );
#ifdef HG_REV_ID
        _tprintf(
            _T("  Revision: %s\n")
            , verinfo[_T("Mercurial revision")]
        );
#endif
        _tprintf(_T("\n"));
    }

    static void showHelp(const CVersionInfo& verinfo)
    {
        showVersion(verinfo);
        _tprintf(
            _T("Syntax:\n")
            _T("  %s [-?|-h|--help][-L|--nologo] [-v|--verbose] [-V|--version] [-E|--noerror] <path ...>\n")
            , verinfo[_T("OriginalFilename")]
        );
    }

    enum
    {
        OPT_HELP = 0,
        OPT_VERSION,
        OPT_NOLOGO,
        OPT_NOERR,
        OPT_VERBOSE,
        OPT_STOP,
    };

    CSimpleOpt::SOption g_Options[] = {
        { OPT_HELP,     _T("-?"),           SO_NONE },
        { OPT_HELP,     _T("-h"),           SO_NONE },
        { OPT_HELP,     _T("-help"),        SO_NONE },
        { OPT_HELP,     _T("--help"),       SO_NONE },
        { OPT_NOLOGO,   _T("-L"),           SO_NONE },
        { OPT_NOLOGO,   _T("--nologo"),     SO_NONE },
        { OPT_NOERR,    _T("-E"),           SO_NONE },
        { OPT_NOERR,    _T("--noerror"),    SO_NONE },
        { OPT_VERBOSE,  _T("-v"),           SO_NONE },
        { OPT_VERBOSE,  _T("--verbose"),    SO_NONE },
        { OPT_VERBOSE,  _T("-V"),           SO_NONE },
        { OPT_VERBOSE,  _T("--version"),    SO_NONE },
        { OPT_STOP,     _T("--"),           SO_NONE },
        SO_END_OF_OPTIONS,
    };

    static int showReparsePoint(WCHAR const* path, bool be_verbose, bool noerror)
    {
        CReparsePoint rp(path);
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
}

int __cdecl _tmain(int argc, _TCHAR *argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);
#endif // _DEBUG
    CVersionInfo verinfo(getInstanceHandle());
    CSimpleOpt args(argc, argv, g_Options, SO_O_NOERR | SO_O_EXACT);

    bool show_logo = true, be_verbose = false, noerror = false;

    while(args.Next())
    {
        if(args.LastError() != SO_SUCCESS)
        {
            _ftprintf(
                stderr
                , _T("%s: '%s' (use --help to get command line help)\n")
                , getLastErrorText(args.LastError()), args.OptionText()
            );
            continue;
        }
        switch(args.OptionId())
        {
        case OPT_HELP:
            showHelp(verinfo);
            return 0;
        case OPT_VERSION:
            showVersion(verinfo);
            return 0;
        case OPT_NOLOGO:
            show_logo = false;
            break;
        case OPT_NOERR:
            noerror = true;
            break;
        case OPT_VERBOSE:
            be_verbose = true;
            break;
        case OPT_STOP:
            args.Stop();
            break;
        }
    }

    if(show_logo)
    {
        showVersion(verinfo);
    }

    int retval = 0;

    for(int n = 0; n < args.FileCount(); n++)
    {
#ifdef _DEBUG
        _CrtCheckMemory();
#endif // _DEBUG
        int err = showReparsePoint(args.File(n), be_verbose, noerror);
        if(err > retval)
        {
            retval = err;
        }
    }

    return retval;
}
