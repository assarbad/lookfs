///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Original filename: lookfs.cpp
/// Project          : lookfs
///
/// Purpose          : Program to investigate all kinds of file system artifacts
///                    on Windows 2000 and later.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef WINVER
#   define WINVER           0x0500
#endif
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT     0x0501
#endif

#ifdef _DEBUG
#   include <cstdlib>
#   include <crtdbg.h>
#   define _TRACE(fmt, ...) do { _ftprintf(stderr, _T("TRACE: ")); _ftprintf(stderr, fmt, __VA_ARGS__); _ftprintf(stderr, _T("\n")); } while (0)
#else
#   define _TRACE(fmt, ...) do {} while (0)
#endif // _DEBUG
#include <cstdio>
#include <tchar.h>
#include <cstring>
#define _ATL_USE_CSTRING
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>
#include "thirdparty/simpleopt/SimpleOpt.h"
#include "ntnative.h"
#include "ntfindfile.h"
#include "AlternateDataStreams.hpp"
#include "ReparsePoint.hpp"
#include "VersionInfo.hpp"

namespace
{
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
            return _T("mount point");
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
            _T("  %s [-?|-h|--help][-L|--nologo] [-v|--verbose] [-V|--version] [-E|--noerror] [-i|--case-insensitive] <path ...>\n")
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
        OPT_NOCASE,
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
        { OPT_NOCASE,   _T("-i"),           SO_NONE },
        { OPT_NOCASE,   _T("--case-insensitive"), SO_NONE },
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
                if(be_verbose)
                {
                    _tprintf(_T("\tSubst name: %ws\n"), rp.SubstName());
                    _tprintf(_T("\tSubst name: %ws (w32)\n"), rp.CanonicalSubstName());
                }
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

    class CPathFinder
    {
        BOOL m_bValid;
        HANDLE m_hFind;
        const CString m_sOriginalPath;
        CString m_sNormalizedPath;
        CString m_sDirectory; // always has a trailing backslash
        CString m_sSearchMask;
        NT_FIND_DATA m_fd;
        LONG m_lError;
        // hide these
        CPathFinder(CPathFinder&);
        CPathFinder& operator=(CPathFinder&);
    public:
        CPathFinder(TCHAR const* szPath, BOOLEAN bCaseSensitive = FALSE)
            : m_bValid(FALSE)
            , m_hFind(INVALID_HANDLE_VALUE)
            , m_sOriginalPath(szPath)
            , m_sNormalizedPath(szPath)
            , m_sSearchMask(_T("*"))
            , m_lError(ERROR_SUCCESS)
        {
            if(!szPath || m_sOriginalPath.IsEmpty())
            {
                _TRACE(_T("Invalid szPath == %s"), szPath);
                return;
            }
            if(adjustPaths_())
            {
                CString sSearchPath(m_sDirectory.GetString());
                sSearchPath += m_sSearchMask;
                memset(&m_fd, 0, sizeof(m_fd));
                m_hFind = NativeFindFirstFile(sSearchPath, &m_fd, bCaseSensitive);
                if(INVALID_HANDLE_VALUE != m_hFind)
                {
                    //_TRACE(_T("NativeFindFirstFile(\"%s\", %p, %s) -> %p"), sSearchPath.GetString(), &m_fd, bCaseSensitive ? _T("TRUE") : _T("FALSE"), m_hFind);
                    // Keep looking for the next item until we have skipped the . and .. entries
                    while(isDotDir_(m_fd.cFileName))
                    {
                        if(!next())
                        {
                            break;
                        }
                    }
                }
                else
                {
                    m_bValid = FALSE;
                    m_lError = GetLastError();
                    //_TRACE(_T("NativeFindFirstFile(\"%s\", %p, %s) -> INVALID_HANDLE_VALUE (error: %d)"), sSearchPath.GetString(), &m_fd, bCaseSensitive ? _T("TRUE") : _T("FALSE"), m_lError);
                }
            }
        }

        ~CPathFinder()
        {
            if(m_hFind != NULL && m_hFind != INVALID_HANDLE_VALUE)
            {
                //_TRACE(_T("NativeFindClose(%p)"), m_hFind);
                NativeFindClose(m_hFind);
            }
        }

        inline bool next()
        {
            if(!m_bValid)
            {
                m_lError = ERROR_INVALID_PARAMETER;
                return false;
            }
            BOOL bRetVal = NativeFindNextFile(m_hFind, &m_fd);
            if(bRetVal)
            {
                // Keep looking for the next item until we have skipped the . and .. entries
                while(isDotDir_(m_fd.cFileName))
                {
                    bRetVal = NativeFindNextFile(m_hFind, &m_fd);
                    if(!bRetVal)
                    {
                        break;
                    }
                }
            }
            if(!bRetVal)
            {
                memset(&m_fd, 0, sizeof(m_fd));
                m_lError = GetLastError();
            }
            return FALSE != bRetVal;
        }

        inline NT_FIND_DATA const& getFindData() const
        {
            return m_fd;
        }

        inline CString getFullPathName() const
        {
            CString p(m_sDirectory);
            p += m_fd.cFileName;
            return p;
        }

        inline LPCTSTR getSearchMask() const
        {
            return m_sSearchMask.GetString();
        }

        inline operator bool() const
        {
            return m_bValid != FALSE;
        }

        inline bool operator!() const
        {
            return !this->operator bool();
        }

        inline LONG LastError() const
        {
            return m_lError;
        }
    private:
        static BOOL isDotDir_(LPCTSTR path)
        {
            if(!path)
                return FALSE;
            if(path[0] == _T('.'))
            {
                if(!path[1]) // directory entry '.'
                    return TRUE;
                if(path[1] == _T('.') && !path[2]) // directory entry '..'
                    return TRUE;
            }
            return FALSE;
        }

        BOOL adjustPaths_()
        {
            // Replace forward slashes by backward slashes
            for(int i = 0; i < m_sNormalizedPath.GetLength(); i++)
            {
                if(m_sNormalizedPath.GetAt(i) == _T('/'))
                {
                    m_sNormalizedPath.SetAt(i, _T('\\'));
                }
            }
            DWORD dwAttr = ::GetFileAttributes(m_sNormalizedPath);
            if((INVALID_FILE_ATTRIBUTES == dwAttr) || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                // For all we know it's a file or a path with wildcard characters at this point
                int iLastSlash = m_sNormalizedPath.ReverseFind(_T('\\'));
                // If there was no slash before the wildcard/filename, something else is fishy ... bail out
                if(-1 == iLastSlash)
                {
                    _TRACE(_T("[ERROR] Could not find last slash in %s"), m_sNormalizedPath.GetString());
                    m_bValid = FALSE;
                    return m_bValid;
                }
                TCHAR* lpszString = m_sNormalizedPath.GetBuffer();
                m_sSearchMask = &lpszString[iLastSlash + 1];
                lpszString[iLastSlash + 1] = 0; // zero-terminate
                m_sDirectory = lpszString;
            }
            if((INVALID_FILE_ATTRIBUTES != dwAttr) && (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
            {
                m_sDirectory = m_sNormalizedPath;
                if(m_sDirectory.GetAt(m_sDirectory.GetLength() - 1) != _T('\\'))
                {
                    m_sDirectory.AppendChar(_T('\\'));
                }
            }
            m_bValid = TRUE;
            return m_bValid;
        }
    };

    int traversePath(WCHAR const* szPath, bool be_verbose, bool noerror, bool casesensitive)
    {
        if(!szPath)
            return 1; // error
        CPathFinder pathFinder(szPath, casesensitive);
        do
        {
            if(!pathFinder)
            {
                _ftprintf(stderr, _T("[ERROR:%d] Failed to read contents of %s\n"), pathFinder.LastError(), pathFinder.getFullPathName().GetString());
                return 0;
            }
            NT_FIND_DATA const& fd = pathFinder.getFindData();
            CString path(pathFinder.getFullPathName());
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                {
                    path.AppendFormat(_T("\\%s"), pathFinder.getSearchMask());
                    // Recurse into subdirectory
                    (void)traversePath(path, be_verbose, noerror, casesensitive);
                }
                else
                {
                    // Don't follow reparse points in any case
                    (void)showReparsePoint(path, be_verbose, noerror);
                }
            }
        } while (pathFinder.next());
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

    bool show_logo = true, be_verbose = false, noerror = false, casesensitive = true;

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
        case OPT_NOCASE:
            casesensitive = false;
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
        
        int err = traversePath(args.File(n), be_verbose, noerror, casesensitive);
        if(err > retval)
        {
            retval = err;
        }
    }

    return retval;
}
