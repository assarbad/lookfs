///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Original filename: ReparsePoint.h
/// Project          : lookfs
///
/// Purpose          : Class to investigate reparse points on Windows 2000 and
///                    later.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __REPARSEPOINT_H_VER__
#define __REPARSEPOINT_H_VER__ 2018032722
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif // Check for "#pragma once" support

#include <tchar.h>
#include <Windows.h>
#include <WinIoCtl.h>

#include "ReparseTags.h"
#include "FileAttributes.h"

#ifndef WIN32_FILE_NAMESPACE
#define WIN32_FILE_NAMESPACE    L"\\\\?\\"
#endif // WIN32_FILE_NAMESPACE
#ifndef WIN32_DEVICE_NAMESPACE
#define WIN32_DEVICE_NAMESPACE  L"\\\\.\\"
#endif // WIN32_DEVICE_NAMESPACE
#ifndef NT_OBJMGR_NAMESPACE
#define NT_OBJMGR_NAMESPACE     L"\\??\\"
#endif // NT_OBJMGR_NAMESPACE

#ifndef WIN32_FILE_NAMESPACE_A
#define WIN32_FILE_NAMESPACE_A      "\\\\?\\"
#endif // WIN32_FILE_NAMESPACE_A
#ifndef WIN32_DEVICE_NAMESPACE_A
#define WIN32_DEVICE_NAMESPACE_A    "\\\\.\\"
#endif // WIN32_DEVICE_NAMESPACE_A
#ifndef NT_OBJMGR_NAMESPACE_A
#define NT_OBJMGR_NAMESPACE_A       L"\\??\\"
#endif // NT_OBJMGR_NAMESPACE_A

#ifndef SYMLINK_FLAG_FULLPATH
/* The substitute name is a full path name. */
#define SYMLINK_FLAG_FULLPATH 0x00000000
#endif // SYMLINK_FLAG_FULLPATH

#ifndef SYMLINK_FLAG_RELATIVE
/* The substitute name is a path name relative to the directory containing the symbolic
link. */
#define SYMLINK_FLAG_RELATIVE 0x00000001
#endif // SYMLINK_FLAG_RELATIVE

namespace
{
    static const ULARGE_INTEGER uliZero = { 0 };
}

class CReparsePoint
{
    // Allow the header to be included again outside of this file (and class)
    // Still this header needs to be included before anyone else who requires VerySimpleBuf.hpp
#   define __VERYSIMPLEBUF_MULTI_INC__
#   include "VerySimpleBuf.hpp"
#   undef __VERYSIMPLEBUF_HPP_VER__
    // Just make it more readable
    typedef CVerySimpleBuf<WCHAR> CWideString, *PWideString;
    typedef CVerySimpleBuf<CHAR>  CAnsiString, *PAnsiString;
    typedef CVerySimpleBuf<TCHAR> CTString, *PTString;
    typedef CVerySimpleBuf<BYTE>  ByteBuf;

#   include <PshPack1.h>
    // The Microsoft version of the reparse data, as opposed to REPARSE_GUID_DATA_BUFFER
    typedef struct _REPARSE_DATA_BUFFER
    {
        ULONG  ReparseTag;
        USHORT  ReparseDataLength;
        USHORT  Reserved;
        union
        {
            struct
            {
                USHORT  SubstituteNameOffset;
                USHORT  SubstituteNameLength;
                USHORT  PrintNameOffset;
                USHORT  PrintNameLength;
                ULONG   Flags;
                WCHAR   PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct
            {
                USHORT  SubstituteNameOffset;
                USHORT  SubstituteNameLength;
                USHORT  PrintNameOffset;
                USHORT  PrintNameLength;
                WCHAR   PathBuffer[1];
            } MountPointReparseBuffer;
            struct
            {
                UCHAR  DataBuffer[1];
            } GenericReparseBuffer;
        };
    } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#   include <PopPack.h>
public:
    CReparsePoint(WCHAR const* Path)
        : m_LastError(ERROR_SUCCESS)
        , m_Path(normalizePath_(Path))
        , m_ReparseGuid(GUID_NULL)
        , m_Attr(::GetFileAttributesW(m_Path.getBuf()))
        , m_OpenFlags(FILE_FLAG_OPEN_REPARSE_POINT | ((m_Attr & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS : 0))
        , m_ReparseTag(0)
#ifdef RP_QUERY_FILE_ID
        , m_FileId(uliZero)
#endif // RP_QUERY_FILE_ID
    {
        cacheValues_();
    }

    inline bool isReparsePoint() const
    {
        return (0 != (m_Attr & FILE_ATTRIBUTE_REPARSE_POINT));
    }

    inline bool isDirectory() const
    {
        return (0 != (m_Attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    inline bool isVirtual() const
    {
        return (0 != (m_Attr & FILE_ATTRIBUTE_VIRTUAL));
    }

    inline bool isSymbolicLink() const // Windows Vista++ symlink
    {
        return (IO_REPARSE_TAG_SYMLINK == m_ReparseTag);
    }

    inline bool isPlaceHolderFile() const // Windows 8.1++ placeholder file (SkyDrive)
    {
        return (IO_REPARSE_TAG_SYMLINK == m_ReparseTag);
    }

    inline bool isJunctionPoint() const
    {
        return (IO_REPARSE_TAG_MOUNT_POINT == m_ReparseTag);
    }

    inline bool isVolumeMountPoint() const // Real *volume* mount point
    {
        WCHAR const VolMtPointPrefix[] = L"\\\\?\\Volume{";
        CVerySimpleBuf<WCHAR> buf(CanonicalSubstName());
        if (buf && (buf.getCountZ() > wcslen(VolMtPointPrefix)))
        {
            buf.getBuf()[wcslen(VolMtPointPrefix)] = 0;
            return isJunctionPoint() && buf && (0 == wcsncmp(buf.getBuf(), VolMtPointPrefix, wcslen(VolMtPointPrefix)));
        }
        return false;
    }

    inline bool isMicrosoftTag() const
    {
        return 0 != IsReparseTagMicrosoft(m_ReparseTag);
    }

    inline bool isNameSurrogate() const
    {
        return 0 != IsReparseTagNameSurrogate(m_ReparseTag);
    }

    WCHAR const* Path() const
    {
        return m_Path.getBuf();
    }

    WCHAR const* CanonicalSubstName() const
    {
        return m_SubstNameCanonical.getBuf();
    }

    WCHAR const* PrintName() const
    {
        return m_PrintName.getBuf();
    }

    WCHAR const* SubstName() const
    {
        return m_SubstName.getBuf();
    }

    inline DWORD ReparseTag() const
    {
        return m_ReparseTag;
    }

    inline GUID const& ReparseGuid() const
    {
        return m_ReparseGuid;
    }

    inline LONG LastError() const
    {
        return m_LastError;
    }

    inline unsigned char* RawReparseData() const
    {
        return m_RawReparseData.getBuf();
    }

    inline size_t RawReparseDataLength() const
    {
        return m_RawReparseData.getCount();
    }

#ifdef RP_QUERY_FILE_ID
    inline ULONGLONG FileIndex() const
    {
        return m_FileId.QuadPart;
    }

    inline DWORD FileIndexLow() const
    {
        return m_FileId.LowPart;
    }

    inline DWORD FileIndexHigh() const
    {
        return m_FileId.HighPart;
    }
#endif // RP_QUERY_FILE_ID

private:
    CVerySimpleBuf<WCHAR> normalizePath_(WCHAR const* Path)
    {
        if (Path)
        {
            WCHAR const Win32Prefix[] = WIN32_FILE_NAMESPACE;
            CVerySimpleBuf<WCHAR> sPath((0 != wcsncmp(Win32Prefix, Path, wcslen(Win32Prefix))) ? Win32Prefix : L"");
            if (sPath.getCountZ()) // Only do anything if the path doesn't have the prefix, yet
            {
                // The following handles the special case where Path == "." ... any other (including "..") seem to be handled fine by GetFullPathName()
                if (1 == wcslen(Path) && 0 == wcsncmp(L".", Path, 1))
                {
                    DWORD dwNeeded = ::GetCurrentDirectoryW(0, NULL);
                    if (dwNeeded)
                    {
                        if (sPath.reAlloc(1 + (size_t)dwNeeded + sPath.getCount()))
                        {
                            if (0 < ::GetCurrentDirectoryW(static_cast<DWORD>(sPath.getCount() - sPath.getCountZ()), sPath.getBuf() + sPath.getCountZ()))
                            {
                                return sPath;
                            }
                        }
                        return Path; // fallback method
                    }
                }
                LPWSTR filePart = 0;
                // dummy call to evaluate required length
                DWORD dwNeeded = ::GetFullPathNameW(Path, 0, sPath.getBuf(), &filePart);
                if (sPath.reAlloc(1 + (size_t)dwNeeded + wcslen(Path)))
                {
                    if (0 < ::GetFullPathNameW(Path, static_cast<DWORD>(sPath.getCount() - sPath.getCountZ()), sPath.getBuf() + sPath.getCountZ(), &filePart))
                    {
                        return sPath;
                    }
                }
            }
        }
        return Path;
    }

    void cacheValues_()
    {
        // WCHAR const Win32Prefix[] = WIN32_UNICODE_PREFIX;
        WCHAR const NativePrefix[] = L"\\??\\";
        HANDLE hFile = ::CreateFile(m_Path.getBuf(), FILE_READ_EA, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, m_OpenFlags, 0);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            memset(&m_ReparseGuid, 0, sizeof(m_ReparseGuid));
            if (isReparsePoint())
            {
#ifdef RP_QUERY_FILE_ID
                BY_HANDLE_FILE_INFORMATION bhfi = { 0 };
                if (::GetFileInformationByHandle(hFile, &bhfi))
                {
                    m_FileId.HighPart = bhfi.nFileIndexHigh;
                    m_FileId.LowPart = bhfi.nFileIndexLow;
                }
                else
                {
                    m_FileId.QuadPart = ((ULONGLONG)-1);
                }
#endif // RP_QUERY_FILE_ID
                CVerySimpleBuf<unsigned char>& buf = m_RawReparseData;
                ::SetLastError(ERROR_SUCCESS);
                DWORD dwReturned = 0;
                if (buf.reAlloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE))
                {
                    if (::DeviceIoControl(
                        hFile,
                        FSCTL_GET_REPARSE_POINT,
                        NULL,
                        0,
                        buf.getBuf(),
                        static_cast<DWORD>(buf.getByteCount()),
                        &dwReturned,
                        0))
                    {
                        PREPARSE_GUID_DATA_BUFFER const repbuf = reinterpret_cast<PREPARSE_GUID_DATA_BUFFER>(buf.getBuf());
                        m_ReparseTag = repbuf->ReparseTag;
                        memcpy(&m_ReparseGuid, &repbuf->ReparseGuid, sizeof(GUID));
                        if (0 != IsReparseTagMicrosoft(m_ReparseTag))
                        {
                            PREPARSE_DATA_BUFFER const msrepbuf = reinterpret_cast<PREPARSE_DATA_BUFFER>(repbuf);
                            if ((IO_REPARSE_TAG_SYMLINK == m_ReparseTag)
                                && (m_PrintName.reAlloc(msrepbuf->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR) + 1))
                                && (m_SubstName.reAlloc(msrepbuf->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR) + 1))
                                )
                            {
                                memcpy(m_PrintName.getBuf(), &msrepbuf->SymbolicLinkReparseBuffer.PathBuffer[msrepbuf->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR)], msrepbuf->SymbolicLinkReparseBuffer.PrintNameLength);
                                memcpy(m_SubstName.getBuf(), &msrepbuf->SymbolicLinkReparseBuffer.PathBuffer[msrepbuf->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], msrepbuf->SymbolicLinkReparseBuffer.SubstituteNameLength);
                            }
                            else if ((IO_REPARSE_TAG_MOUNT_POINT == m_ReparseTag)
                                && (m_PrintName.reAlloc(msrepbuf->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR) + 1))
                                && (m_SubstName.reAlloc(msrepbuf->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR) + 1))
                                )
                            {
                                memcpy(m_PrintName.getBuf(), &msrepbuf->MountPointReparseBuffer.PathBuffer[msrepbuf->MountPointReparseBuffer.PrintNameOffset / sizeof(WCHAR)], msrepbuf->MountPointReparseBuffer.PrintNameLength);
                                memcpy(m_SubstName.getBuf(), &msrepbuf->MountPointReparseBuffer.PathBuffer[msrepbuf->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], msrepbuf->MountPointReparseBuffer.SubstituteNameLength);
                            }
                            m_SubstNameCanonical = m_SubstName;
                            // If the prefix is of the native NT form, change it to the Win32 form instead
                            if (m_SubstNameCanonical.getCountZ() && (0 == wcsncmp(NativePrefix, m_SubstNameCanonical.getBuf(), sizeof(NativePrefix) / sizeof(NativePrefix[0]) - 1)))
                            {
                                m_SubstNameCanonical.getBuf()[1] = L'\\';
                            }
                        }
                    }
                }
            }
#ifdef RP_QUERY_FILE_ID
            else
            {
                BY_HANDLE_FILE_INFORMATION bhfi = { 0 };
                if (::GetFileInformationByHandle(hFile, &bhfi))
                {
                    m_FileId.HighPart = bhfi.nFileIndexHigh;
                    m_FileId.LowPart = bhfi.nFileSizeLow;
                }
                else
                {
                    m_FileId.QuadPart = ((ULONGLONG)-1);
                }
            }
#endif // RP_QUERY_FILE_ID
            ::CloseHandle(hFile);
        }
        else
        {
            m_LastError = GetLastError();
        }
    }

    LONG                    m_LastError;
    CVerySimpleBuf<WCHAR>   m_Path;
    CVerySimpleBuf<WCHAR>   m_SubstName;
    CVerySimpleBuf<WCHAR>   m_PrintName;
    CVerySimpleBuf<WCHAR>   m_SubstNameCanonical;
    GUID                    m_ReparseGuid;
    DWORD const             m_Attr;
    DWORD const             m_OpenFlags;
    DWORD                   m_ReparseTag;
    CVerySimpleBuf<unsigned char> m_RawReparseData;
#ifdef RP_QUERY_FILE_ID
    ULARGE_INTEGER          m_FileId;
#endif // RP_QUERY_FILE_ID
    // Hide these
    CReparsePoint();
    CReparsePoint& operator=(const CReparsePoint&);
};

#endif // __REPARSEPOINT_H_VER__
