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
#include <fcntl.h>
#include <io.h>
#define _ATL_USE_CSTRING
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>
#include "lookfs.hpp"
#include "thirdparty/simpleopt/SimpleOpt.h"
#include "ntnative.h"
#include "ntfindfile.h"
#include "priv.h"
#include "AlternateDataStreams.hpp"
#include "ReparsePoint.hpp"
#include "VersionInfo.hpp"

namespace
{
    typedef struct _cmdlnargs_t
    {
        bool logo;
        bool verbose;
        bool noerror;
        bool casesensitive;
        bool showall;
        bool printname;
    } settings_t;

    static settings_t g_settings = { true, false, false, true, false, false };

    class CFileStreamWrapper
    {
        FILE* m_file;
    public:
        CFileStreamWrapper(TCHAR const* lpszPath = NULL)
            : m_file(NULL)
        {
            (void)open(lpszPath);
        }

        virtual ~CFileStreamWrapper()
        {
            close_();
        }

        bool open(TCHAR const* lpszPath)
        {
            close_();
            if (lpszPath)
            {
                errno_t err = _tfopen_s(&m_file, lpszPath, _T("w"));
                if (err)
                {
#ifdef _DEBUG
                    TCHAR buf[MAX_PATH] = { 0 };
                    errno_t converr = _tcserror_s(buf, _countof(buf), err);
                    if (converr)
                    {
                        _TRACE(_T("errno = %d <failed to retrieve error string>\n"), err);
                    }
                    else
                    {
                        _TRACE(_T("errno = %d, %s\n"), err, buf);
                    }
#endif // _DEBUG
                    return false;
                }
#if defined(UNICODE) || defined(_UNICODE)
                // We want to output as UTF-8
                if (m_file)
                {
                    // Insert a byte-order mark if this is the start of the file
                    if (0 == ftell(m_file))
                    {
                        (void)_setmode(_fileno(m_file), _O_BINARY);
                        fwrite("\xEF\xBB\xBF", sizeof(char), 3, m_file);
                        fflush(m_file);
                    }
                    (void)_setmode(_fileno(m_file), _O_U8TEXT);
                }
#endif
                return true;
            }
            return false;
        }

        operator FILE*() const
        {
            return m_file;
        }

        operator bool() const
        {
            return (m_file != NULL);
        }

        bool operator!() const
        {
            return (m_file != NULL);
        }

    private:
        // Hide these
        CFileStreamWrapper(CFileStreamWrapper&);
        CFileStreamWrapper& operator=(CFileStreamWrapper&);

        void close_()
        {
            if (m_file)
            {
                fclose(m_file);
                m_file = NULL;
            }
        }
    };

    CFileStreamWrapper g_outfile;

    static const TCHAR *getSimpleOptLastErrorText(int a_nError)
    {
        switch (a_nError)
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

    static DWORD wrapFormatMessage_(HMODULE lpSource, DWORD dwMessageId, TCHAR** lpBuffer, va_list* Arguments)
    {
        DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
        if (lpSource)
        {
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
        }
        return ::FormatMessage(dwFlags, lpSource, dwMessageId, 0, (LPTSTR)(lpBuffer), 0x20, Arguments);
    }

    static CString formatMessage(HMODULE hMod, DWORD MessageID, ...)
    {
        TCHAR* retBuf = 0;
        va_list ptr;
        va_start(ptr, MessageID);
        if (wrapFormatMessage_(
            hMod,
            MessageID,
            (TCHAR**)(&retBuf),
            &ptr
        ) && (0 != retBuf))
        {
            CString s(retBuf);
            LocalFree((HLOCAL)(retBuf));
            va_end(ptr);
            s.TrimRight(_T("\r\n"));
            return s;
        }
        va_end(ptr);
        return CString();
    }

    static LPCTSTR getReparseType(const CReparsePoint& rp)
    {
        if (!rp.isReparsePoint())
            return _T("");
        switch (rp.ReparseTag())
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
        case IO_REPARSE_TAG_DRIVE_EXTENDER:
            return _T("Home server drive extender reparse point");
        case IO_REPARSE_TAG_FILTER_MANAGER:
            return _T("Filter manager test harness reparse point");
        default:
            return _T("<unknown reparse tag>");
        }
    }

    static int showReparsePoint(WCHAR const* path, settings_t const& sett)
    {
        CReparsePoint rp(path);
        if (ERROR_SUCCESS != rp.LastError())
        {
            if (!sett.noerror)
            {
                _ftprintf(stderr, _T("ERROR: Win32 error code %d. Probably file (2) or path (3) not found or access issue (5)?!\n"), rp.LastError());
            }
            return 3;
        }
        if (rp.isReparsePoint())
        {
            _tprintf(_T("%ws\n"), rp.Path());
            if (sett.verbose)
            {
                _tprintf(_T("\tA %sMicrosoft %s (virt == %d)\n"),
                    ((rp.isMicrosoftTag()) ? _T("") : _T("non-")),
                    getReparseType(rp),
                    rp.isVirtual()
                );
#ifdef RP_QUERY_FILE_ID
                if (-1 != rp.FileIndex())
                {
                    _tprintf(_T("\tFile Index: %I64u (%08X%08X)\n"), rp.FileIndex(), rp.FileIndexHigh(), rp.FileIndexLow());
                }
#endif // RP_QUERY_FILE_ID
            }
            if (rp.isNameSurrogate())
            {
                if (!sett.verbose)
                {
                    if (!sett.printname)
                        _tprintf(_T("\t-> %ws\n"), rp.SubstName());
                    else
                        _tprintf(_T("\t-> %ws\n"), rp.PrintName());
                }
                else
                {
                    _tprintf(_T("\tPrint name: %ws\n"), rp.PrintName());
                    _tprintf(_T("\tSubst name: %ws\n"), rp.SubstName());
                    _tprintf(_T("\tSubst name: %ws (w32)\n"), rp.CanonicalSubstName());
                }
            }
            if (!rp.isMicrosoftTag() || sett.verbose)
            {
                if (sett.verbose)
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
            if (!sett.noerror) // show no errors if asked not to
            {
                _ftprintf(stderr, _T("ERROR: '%ws' is not a reparse point\n"), rp.Path());
            }
            if (sett.verbose)
            {
#ifdef RP_QUERY_FILE_ID
                if (-1 != rp.FileIndex())
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
        NTFIND_DATA m_fd;
        LONG m_lError;
        // hide these
        CPathFinder(CPathFinder&); // hide
        CPathFinder& operator=(CPathFinder&); // hide
    public:
        CPathFinder(TCHAR const* szPath, BOOLEAN bCaseSensitive = FALSE)
            : m_bValid(FALSE)
            , m_hFind(INVALID_HANDLE_VALUE)
            , m_sOriginalPath(szPath)
            , m_sNormalizedPath(szPath)
            , m_sSearchMask(_T("*"))
            , m_lError(ERROR_SUCCESS)
        {
            memset(&m_fd, 0, sizeof(m_fd));
            if (!szPath || m_sOriginalPath.IsEmpty())
            {
                _TRACE(_T("Invalid szPath == %s"), szPath);
                return;
            }
            if (adjustPaths_())
            {
                CString sSearchPath(m_sDirectory);
                sSearchPath += m_sSearchMask;
                m_hFind = NativeFindFirstFile(sSearchPath, &m_fd, bCaseSensitive);
                if (INVALID_HANDLE_VALUE != m_hFind)
                {
                    //_TRACE(_T("NativeFindFirstFile(\"%s\", %p, %s) -> %p"), sSearchPath.GetString(), &m_fd, bCaseSensitive ? _T("TRUE") : _T("FALSE"), m_hFind);
                    // Keep looking for the next item until we have skipped the . and .. entries
                    while (isDotDir_(m_fd.cFileName))
                    {
                        if (!next())
                        {
                            break;
                        }
                    }
                }
                else
                {
                    m_lError = GetLastError();
                    m_bValid = FALSE;
                    //_TRACE(_T("NativeFindFirstFile(\"%s\", %p, %s) -> INVALID_HANDLE_VALUE (error: %d)"), sSearchPath.GetString(), &m_fd, bCaseSensitive ? _T("TRUE") : _T("FALSE"), m_lError);
                }
            }
        }

        ~CPathFinder()
        {
            if (m_hFind != NULL && m_hFind != INVALID_HANDLE_VALUE)
            {
                //_TRACE(_T("NativeFindClose(%p)"), m_hFind);
                NativeFindClose(m_hFind);
            }
        }

        inline bool next()
        {
            if (!m_bValid)
            {
                m_lError = ERROR_INVALID_PARAMETER;
                return false;
            }
            BOOL bRetVal = NativeFindNextFile(m_hFind, &m_fd);
            if (bRetVal)
            {
                // Keep looking for the next item until we have skipped the . and .. entries
                while (isDotDir_(m_fd.cFileName))
                {
                    bRetVal = NativeFindNextFile(m_hFind, &m_fd);
                    if (!bRetVal)
                    {
                        break;
                    }
                }
            }
            if (!bRetVal)
            {
                memset(&m_fd, 0, sizeof(m_fd));
                m_lError = GetLastError();
            }
            return FALSE != bRetVal;
        }

        inline NTFIND_DATA const& getFindData() const
        {
            return m_fd;
        }

        inline CString getNormalizedPathName() const
        {
            return m_sNormalizedPath;
        }

        inline CString getFullPathName() const
        {
            CString p;
            if (m_bValid)
            {
                p = m_sDirectory;
                p += m_fd.cFileName;
            }
            else
            {
                p = m_sOriginalPath;
            }
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
            if (!path)
                return FALSE;
            if (path[0] == _T('.'))
            {
                if (!path[1]) // directory entry '.'
                    return TRUE;
                if (path[1] == _T('.') && !path[2]) // directory entry '..'
                    return TRUE;
            }
            return FALSE;
        }

        BOOL adjustPaths_()
        {
            // Replace forward slashes by backward slashes
            m_sNormalizedPath.TrimRight(_T("\\/"));
            for (int i = 0; i < m_sNormalizedPath.GetLength(); i++)
            {
                if (m_sNormalizedPath.GetAt(i) == _T('/'))
                {
                    m_sNormalizedPath.SetAt(i, _T('\\'));
                }
            }
            size_t const nOffset = _countof(WIN32_FILE_NAMESPACE_A) - 1;
            if (m_sNormalizedPath.Left(nOffset) != _T(WIN32_FILE_NAMESPACE_A)
                && m_sNormalizedPath.Left(nOffset) != _T(WIN32_DEVICE_NAMESPACE_A))
            {
                CString sFullPathBuf(_T(WIN32_FILE_NAMESPACE_A));
                LPTSTR lpszFullPathBuf = &sFullPathBuf.GetBufferSetLength(0x8000)[nOffset];
                DWORD nBufferLength = sFullPathBuf.GetLength() - nOffset - 1;
                LPTSTR lpszFileName = NULL;
                DWORD dwLength = ::GetFullPathName(m_sNormalizedPath, nBufferLength, lpszFullPathBuf, &lpszFileName);
                if (!dwLength)
                {
                    _TRACE(_T("[ERROR:%d] Failed to get full path from \"%s\" (%s)."), GetLastError(), m_sNormalizedPath.GetString(), formatMessage(NULL, GetLastError()).GetString());
                    m_lError = GetLastError();
                    m_bValid = FALSE;
                    return m_bValid;
                }
                if (dwLength > nBufferLength)
                {
                    _TRACE(_T("[ERROR:%d] Buffer too small to get full path from \"%s\" (%s)."), GetLastError(), m_sNormalizedPath.GetString(), formatMessage(NULL, GetLastError()).GetString());
                    m_lError = GetLastError();
                    m_bValid = FALSE;
                    return m_bValid;
                }
                sFullPathBuf.ReleaseBuffer();
                m_sNormalizedPath = sFullPathBuf;
            }
            DWORD dwAttr = ::GetFileAttributes(m_sNormalizedPath);
            // For all we know it's a file or a path with wildcard characters at
            // this point. But it could also be a filename instead of a wildcard.
            if ((INVALID_FILE_ATTRIBUTES == dwAttr) || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                int iLastSlash = m_sNormalizedPath.ReverseFind(_T('\\'));
                // If there was no slash before the wildcard/filename, something else is fishy ... bail out
                if (-1 == iLastSlash)
                {
                    _TRACE(_T("[ERROR:%d] Could not find last slash in \"%s\" (%s)."), GetLastError(), m_sNormalizedPath.GetString(), formatMessage(NULL, GetLastError()).GetString());
                    m_lError = GetLastError();
                    m_bValid = FALSE;
                    return m_bValid;
                }
                TCHAR* lpszString = m_sNormalizedPath.GetBuffer();
                m_sSearchMask = &lpszString[iLastSlash + 1];
                lpszString[iLastSlash + 1] = 0; // zero-terminate
                m_sDirectory = lpszString;
            }
            if ((INVALID_FILE_ATTRIBUTES != dwAttr) && (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
            {
                m_sDirectory = m_sNormalizedPath;
                if (m_sDirectory.GetAt(m_sDirectory.GetLength() - 1) != _T('\\'))
                {
                    m_sDirectory.AppendChar(_T('\\'));
                }
            }
            m_bValid = TRUE;
            return m_bValid;
        }
    };

    static void showVersion(const CVersionInfo& verinfo)
    {
        _tprintf(
            _T("%s %s\n")
            , verinfo[_T("OriginalFilename")]
            , verinfo[_T("FileVersion")]
        );
#ifdef HG_REV_ID
        _tprintf(
            _T("  Revision: %s\n")
            , verinfo[_T("Mercurial revision")]
        );
#endif
        _tprintf(
            _T("  %s\n\n")
            , verinfo[_T("LegalCopyright")]
        );
    }

    static void showHelp(const CVersionInfo& verinfo)
    {
        showVersion(verinfo);
        _tprintf(
            _T("Syntax:\n")
            _T("  %s [-?|-h|--help|...] [--] [path...]\n")
            , verinfo[_T("OriginalFilename")]
        );
        _tprintf(_T("  -?, -h, --help\n\tShow this help and exit\n"));
        _tprintf(_T("  -V, --version\n\tShow program version and exit\n"));
        _tprintf(_T("  -L, --nologo, --nobanner\n\tDon't show program banner text\n"));
        _tprintf(_T("  -E, --noerror\n\tDon't show errors, only show output from success\n"));
        _tprintf(_T("  -v, --verbose\n\tShow more verbose output\n"));
        _tprintf(_T("  -i, --nocase, --case-insensitive\n\tWork case-insensitive\n"));
        _tprintf(_T("  -a, --all, --show-all\n\tShow each file system entity encountered, not just \"special\" ones\n"));
        _tprintf(_T("  -p, --printname, --print-name\n\tInstead of the substitute name, show print names for reparse points\n"));
        _tprintf(_T("  --\n\tDelimiter for options. Everything after it will be interpreted as paths\n"));

#ifdef TEXT_PORTIONSCOPYRIGHT
        _tprintf(_T("\n") _T(ANSISTRING(TEXT_PORTIONSCOPYRIGHT)) _T("\n"));
#endif
    }

    int traversePath(WCHAR const* szPath, settings_t const& sett)
    {
        if (!szPath)
        {
            return 1; // error
        }
        CPathFinder pathFinder(szPath, sett.casesensitive);
        int iRet = 0;
        do
        {
            static DWORD const FILE_ATTRIBUTE_REPARSE_DIR = (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT);
            if (!pathFinder)
            {
                /*
                                // Smoke tests
                                CString sCheckedPath = pathFinder.getNormalizedPathName();
                                sCheckedPath.TrimRight(_T("\\"));
                                DWORD dwAttr = ::GetFileAttributes(sCheckedPath);
                                if(INVALID_FILE_ATTRIBUTES == dwAttr)
                                {
                                    if(!sett.noerror)
                                    {
                                        _ftprintf(stderr, _T("[ERROR:%d] Invalid file attributes for \"%s\" (%s).\n"), GetLastError(), sCheckedPath.GetString(), formatMessage(NULL, GetLastError()).GetString());
                                    }
                                    return 1;
                                }
                                if(REPARSE_DIR == (REPARSE_DIR & dwAttr))
                                {
                                    return showReparsePoint(szPath, sett);
                                }
                */
                if (!sett.noerror)
                {
                    _ftprintf(stderr, _T("[ERROR:%d] Failed to read contents of \"%s\" (%s).\n"), pathFinder.LastError(), pathFinder.getFullPathName().GetString(), formatMessage(NULL, pathFinder.LastError()).GetString());
                }
                return 1;
            }
            CString path(pathFinder.getFullPathName());
            if (sett.showall)
            {
                _tprintf(_T("%s\n"), path.GetString());
            }
            NTFIND_DATA const& fd = pathFinder.getFindData();
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Don't follow reparse points in any case
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                {
                    // Recurse into subdirectory
                    int err = traversePath(path, sett);
                    if (err)
                    {
                        iRet = err;
                    }
                    return iRet;
                }
            }
            // Not only directories can be reparse points
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                int err = showReparsePoint(path, sett);
                if (err)
                {
                    iRet = err;
                }
            }
        } while (pathFinder.next());
        return iRet;
    }

    enum
    {
        OPT_HELP = 0,
        OPT_VERSION,
        OPT_NOLOGO,
        OPT_NOERR,
        OPT_VERBOSE,
        OPT_NOCASE,
        OPT_SHOWALL,
        OPT_PRINTNAME,
        OPT_OUTPUTFILE,
        OPT_STOP,
    };

    CSimpleOpt::SOption g_Options[] = {
        { OPT_HELP,     _T("-?"),           SO_NONE },
        { OPT_HELP,     _T("-h"),           SO_NONE },
        { OPT_HELP,     _T("-help"),        SO_NONE },
        { OPT_HELP,     _T("--help"),       SO_NONE },
        { OPT_NOLOGO,   _T("-L"),           SO_NONE },
        { OPT_NOLOGO,   _T("--nologo"),     SO_NONE },
        { OPT_NOLOGO,   _T("--nobanner"),   SO_NONE },
        { OPT_NOERR,    _T("-E"),           SO_NONE },
        { OPT_NOERR,    _T("--noerror"),    SO_NONE },
        { OPT_VERBOSE,  _T("-v"),           SO_NONE },
        { OPT_VERBOSE,  _T("--verbose"),    SO_NONE },
        { OPT_VERSION,  _T("-V"),           SO_NONE },
        { OPT_VERSION,  _T("--version"),    SO_NONE },
        { OPT_NOCASE,   _T("-i"),           SO_NONE },
        { OPT_NOCASE,   _T("--nocase"),     SO_NONE },
        { OPT_NOCASE,   _T("--case-insensitive"), SO_NONE },
        { OPT_SHOWALL,  _T("-a"),           SO_NONE },
        { OPT_SHOWALL,  _T("--all"),        SO_NONE },
        { OPT_SHOWALL,  _T("--show-all"),   SO_NONE },
        { OPT_PRINTNAME,_T("-p"),           SO_NONE },
        { OPT_PRINTNAME,_T("--printname"),  SO_NONE },
        { OPT_PRINTNAME,_T("--print-name"), SO_NONE },
        { OPT_OUTPUTFILE,_T("-o"),         SO_REQ_SEP },
        { OPT_OUTPUTFILE,_T("--output"),    SO_REQ_SEP },
        { OPT_STOP,     _T("--"),           SO_NONE },
        SO_END_OF_OPTIONS,
    };

}

int __cdecl _tmain(int argc, _TCHAR *argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);
#endif // _DEBUG
    CVersionInfo verinfo(getInstanceHandle());
    CSimpleOpt args(argc, argv, g_Options, SO_O_EXACT | SO_O_CLUMP | SO_O_SHORTARG | SO_O_ICASE_LONG);

    while (args.Next())
    {
        if (args.LastError() != SO_SUCCESS)
        {
            _ftprintf(
                stderr
                , _T("%s: '%s' (use --help to get command line help)\n")
                , getSimpleOptLastErrorText(args.LastError()), args.OptionText()
            );
            continue;
        }
        switch (args.OptionId())
        {
        case OPT_HELP:
            showHelp(verinfo);
            return 0;
        case OPT_VERSION:
            showVersion(verinfo);
            return 0;
        case OPT_NOLOGO:
            g_settings.logo = false;
            break;
        case OPT_NOERR:
            g_settings.noerror = true;
            break;
        case OPT_VERBOSE:
            g_settings.verbose = true;
            break;
        case OPT_NOCASE:
            g_settings.casesensitive = false;
            break;
        case OPT_SHOWALL:
            g_settings.showall = true;
            break;
        case OPT_PRINTNAME:
            g_settings.printname = true;
            break;
        case OPT_OUTPUTFILE:
            g_outfile.open(args.OptionArg());
            if (!g_outfile)
            {
                _tprintf(_T("Failed to open output file.\n"));
            }
            break;
        case OPT_STOP:
            args.Stop();
            break;
        }
    }

    if (g_settings.logo)
    {
        if (!args.FileCount())
        {
            showHelp(verinfo);
            return 0;
        }
        else
        {
            showVersion(verinfo);
        }
    }

    // allows us to traverse into places that are otherwise off limits
    CSnapEnableAssignedPrivilege privBackup(SE_BACKUP_NAME);

    int retval = 0;

    for (int n = 0; n < args.FileCount(); n++)
    {
#ifdef _DEBUG
        _CrtCheckMemory();
#endif // _DEBUG

        int err = traversePath(args.File(n), g_settings);
        if (err > retval)
        {
            retval = err;
        }
    }

    return retval;
}
