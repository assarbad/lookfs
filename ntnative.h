///////////////////////////////////////////////////////////////////////////////
///
/// Type and function declarations for the NT native API.
///
/// Dual-licensed under MS-PL and MIT license (see below).
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016-2018 Oliver Schneider (assarbad.net)
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

#ifndef __NTNATIVE_H_VER__
#define __NTNATIVE_H_VER__ 2018063019
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

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

#ifndef _Must_inspect_result_
#   define _Must_inspect_result_
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

#if defined(DDKBUILD)
#   include <WinDef.h>
#else
#   pragma push_macro("NTSYSCALLAPI")
#   ifdef NTSYSCALLAPI
#       undef NTSYSCALLAPI
#       define NTSYSCALLAPI
#   endif
#   pragma warning(disable:4201)
#   include <winternl.h>
#   pragma warning(default:4201)
#   pragma pop_macro("NTSYSCALLAPI")
#endif // DDKBUILD
#pragma warning(disable:4005)
#include <ntstatus.h>
#pragma warning(default:4005)

#if defined(DDKBUILD)
#   if defined(__cplusplus)
extern "C"
{
#   endif
    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING;
    typedef UNICODE_STRING *PUNICODE_STRING;
    typedef const UNICODE_STRING *PCUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES;
    typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#   ifndef _NTDEF_
    typedef _Success_(return >= 0) LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;

#   if defined(DDKBUILD)
    typedef struct _STRING {
        USHORT Length;
        USHORT MaximumLength;
#       ifdef MIDL_PASS
        [size_is(MaximumLength), length_is(Length) ]
#       endif // MIDL_PASS
        PCHAR Buffer;
    } STRING;
    typedef STRING *PSTRING;

    typedef STRING ANSI_STRING;
    typedef PSTRING PANSI_STRING;

    typedef STRING OEM_STRING;
    typedef PSTRING POEM_STRING;

    typedef int PROCESSINFOCLASS;
    typedef int THREADINFOCLASS;
    typedef int SYSTEM_INFORMATION_CLASS;
    typedef CONST char *PCSZ;
    typedef PSTRING PCANSI_STRING;
#   endif

#   endif

    typedef struct _IO_STATUS_BLOCK {
        union {
            NTSTATUS Status;
            PVOID Pointer;
        } DUMMYUNIONNAME;

        ULONG_PTR Information;
    } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#   define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#   define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#   define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

    NTSTATUS
    NTAPI
    NtClose(
        _In_ HANDLE Handle
    );

    VOID
    NTAPI
    RtlInitUnicodeString(
        _Out_ PUNICODE_STRING DestinationString,
        _In_opt_ PCWSTR SourceString
    );

#   if defined(__cplusplus)
    }
#   endif
#endif // DDKBUILD

#ifndef InitializeObjectAttributes
#   define InitializeObjectAttributes( p, n, a, r, s ) { \
        (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
        (p)->RootDirectory = r;                             \
        (p)->Attributes = a;                                \
        (p)->ObjectName = n;                                \
        (p)->SecurityDescriptor = s;                        \
        (p)->SecurityQualityOfService = NULL;               \
        }
#endif

#if (_MSC_VER < 1400) || defined(DDKBUILD)
typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectAllInformation,
    ObjectDataInformation
} OBJECT_INFORMATION_CLASS;
#endif

#ifndef RTL_CONSTANT_STRING
#if defined(__cplusplus)
extern "C++"
{
    char _RTL_CONSTANT_STRING_type_check(const char *s);
    char _RTL_CONSTANT_STRING_type_check(const WCHAR *s);
    // __typeof would be desirable here instead of sizeof.
    template <size_t N> class _RTL_CONSTANT_STRING_remove_const_template_class;
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(char)>  { public: typedef  char T; };
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(WCHAR)> { public: typedef WCHAR T; };
#   define _RTL_CONSTANT_STRING_remove_const_macro(s) \
        (const_cast<_RTL_CONSTANT_STRING_remove_const_template_class<sizeof((s)[0])>::T*>(s))
}
#else
    char _RTL_CONSTANT_STRING_type_check(const void *s);
#   define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)
#endif // __cplusplus
#define RTL_CONSTANT_STRING(s) \
{ \
    sizeof( s ) - sizeof( (s)[0] ), \
    sizeof( s ) / sizeof(_RTL_CONSTANT_STRING_type_check(s)), \
    _RTL_CONSTANT_STRING_remove_const_macro(s) \
}
#endif // RTL_CONSTANT_STRING

#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef OBJECT_TYPE_CREATE
#   define OBJECT_TYPE_CREATE (0x0001)
#endif // OBJECT_TYPE_CREATE

#ifndef OBJECT_TYPE_ALL_ACCESS
#   define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)
#endif // OBJECT_TYPE_ALL_ACCESS

#ifndef DIRECTORY_QUERY
#   define DIRECTORY_QUERY                 (0x0001)
#endif // DIRECTORY_QUERY
#ifndef DIRECTORY_TRAVERSE
#   define DIRECTORY_TRAVERSE              (0x0002)
#endif // DIRECTORY_TRAVERSE
#ifndef DIRECTORY_CREATE_OBJECT
#   define DIRECTORY_CREATE_OBJECT         (0x0004)
#endif // DIRECTORY_CREATE_OBJECT
#ifndef DIRECTORY_CREATE_SUBDIRECTORY
#   define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)
#endif // DIRECTORY_CREATE_SUBDIRECTORY

#ifndef DIRECTORY_ALL_ACCESS
#   define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)
#endif // DIRECTORY_ALL_ACCESS

#ifndef SYMBOLIC_LINK_QUERY
#   define SYMBOLIC_LINK_QUERY (0x0001)
#endif // SYMBOLIC_LINK_QUERY

#ifndef SYMBOLIC_LINK_ALL_ACCESS
#   define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)
#endif // SYMBOLIC_LINK_ALL_ACCESS

#ifndef _NTDEF_
    typedef _Success_(return >= 0) LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;
#   define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#   define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#   define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#   define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

//
// Section Access Rights.
//


#define SECTION_QUERY                0x0001
#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0008
#define SECTION_EXTEND_SIZE          0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020 // not included in SECTION_ALL_ACCESS

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG Attributes;
    ACCESS_MASK DesiredAccess;
    ULONG HandleCount;
    ULONG ReferenceCount;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG Reserved[3];
    ULONG NameInformationLength;
    ULONG TypeInformationLength;
    ULONG SecurityDescriptorLength;
    LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

typedef enum _ATOM_INFORMATION_CLASS
{
    AtomBasicInformation,
    AtomTableInformation,
} ATOM_INFORMATION_CLASS;

typedef enum _EVENT_INFORMATION_CLASS
{
    EventBasicInformation
} EVENT_INFORMATION_CLASS;

typedef enum _IO_COMPLETION_INFORMATION_CLASS {
    IoCompletionBasicInformation
} IO_COMPLETION_INFORMATION_CLASS, *PIO_COMPLETION_INFORMATION_CLASS;

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

#if !defined(_WDMDDK_) && !defined(_WDM_INCLUDED_) && !defined(_DDK_DRIVER_) && !defined(WINAPI_FAMILY_PARTITION) && !defined(WINAPI_PARTITION_DESKTOP) && !defined(WINAPI_PARTITION_SYSTEM)
/* newer Windows Kits have this one, so we check for WINAPI_FAMILY_PARTITION and friends */
typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyWow64FlagsInformation,
    KeyControlFlagsInformation,
    KeySetVirtualizationInformation,
    KeySetDebugInformation,
    KeySetHandleTagsInformation,
    MaxKeySetInfoClass  // MaxKeySetInfoClass should always be the last enum
} KEY_SET_INFORMATION_CLASS;
#endif

typedef enum _MUTANT_INFORMATION_CLASS
{
    MutantBasicInformation,
    MutantOwnerInformation
} MUTANT_INFORMATION_CLASS;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS;

typedef enum _SEMAPHORE_INFORMATION_CLASS
{
    SemaphoreBasicInformation
} SEMAPHORE_INFORMATION_CLASS;

typedef enum _TIMER_INFORMATION_CLASS
{
    TimerBasicInformation
} TIMER_INFORMATION_CLASS;

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE, *PEVENT_TYPE;

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

typedef struct _IO_COMPLETION_BASIC_INFORMATION {
    ULONG Depth;
} IO_COMPLETION_BASIC_INFORMATION, *PIO_COMPLETION_BASIC_INFORMATION;

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG TitleIndex;
    ULONG NameLength;
    WCHAR Name[1]; // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
    //          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

#if (_MSC_VER  < 1500) || defined(DDKBUILD)
typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;
#endif

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

#pragma warning(disable:4201 4214)
typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    ULONG MaximumStackSize;
    ULONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        } minmaj;
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN ImageFlags;
    ULONG ComPlusNativeReady: 1;
    ULONG ComPlusILOnly: 1;
    ULONG ImageDynamicallyRelocated: 1;
    ULONG Reserved: 5;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;
#pragma warning(default:4201 4214)

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    ULONG CurrentCount;
    ULONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

//
//  The context structure is used when generating 8.3 names.  The caller must
//  always zero out the structure before starting a new generation sequence
//

typedef struct _GENERATE_NAME_CONTEXT {

    //
    //  The structure is divided into two strings.  The Name, and extension.
    //  Each part contains the value that was last inserted in the name.
    //  The length values are in terms of wchars and not bytes.  We also
    //  store the last index value used in the generation collision algorithm.
    //

    USHORT Checksum;
    BOOLEAN ChecksumInserted;

    _Field_range_(<=, 8) UCHAR NameLength;        // not including extension
    WCHAR NameBuffer[8];                          // e.g., "ntoskrnl"

    _Field_range_(<=, 4) ULONG ExtensionLength;   // including dot
    WCHAR ExtensionBuffer[4];                     // e.g., ".exe"

    ULONG LastIndexValue;

} GENERATE_NAME_CONTEXT;
typedef GENERATE_NAME_CONTEXT *PGENERATE_NAME_CONTEXT;

#ifndef PIO_APC_ROUTINE_DEFINED
#pragma warning(push) /* disable code analyzer warnings for ATL & WTL libraries */
#pragma warning(disable:28301) /* warning C28301 : No annotations for first declaration of ... */
typedef VOID (NTAPI *PIO_APC_ROUTINE) (_In_ PVOID ApcContext, _In_ PIO_STATUS_BLOCK IoStatusBlock, _In_ ULONG Reserved);
#define PIO_APC_ROUTINE_DEFINED
#pragma warning(pop) /* restore code analyzer warnings*/
#endif // PIO_APC_ROUTINE_DEFINED

typedef enum _NT_FILE_INFORMATION_CLASS
{
    FileInformationDirectory = 1,
    FileInformationFullDirectory, //  2
    FileInformationBothDirectory, //  3
    FileInformationBasic, //  4
    FileInformationStandard, //  5
    FileInformationInternal, //  6
    FileInformationEa, //  7
    FileInformationAccess, //  8
    FileInformationName, //  9
    FileInformationRename, //  10
    FileInformationLink, //  11
    FileInformationNames, //  12
    FileInformationDisposition, //  13
    FileInformationPosition, //  14
    FileInformationFullEa, //  15
    FileInformationMode, //  16
    FileInformationAlignment, //  17
    FileInformationAll, //  18
    FileInformationAllocation, //  19
    FileInformationEndOfFile, //  20
    FileInformationAlternateName, //  21
    FileInformationStream, //  22
    FileInformationPipe, //  23
    FileInformationPipeLocal, //  24
    FileInformationPipeRemote, //  25
    FileInformationMailslotQuery, //  26
    FileInformationMailslotSet, //  27
    FileInformationCompression, //  28
    FileInformationObjectId, //  29
    FileInformationCompletion, //  30
    FileInformationMoveCluster, //  31
    FileInformationQuota, //  32
    FileInformationReparsePoint, //  33
    FileInformationNetworkOpen, //  34
    FileInformationAttributeTag, //  35
    FileInformationTracking, //  36
    FileInformationIdBothDirectory, //  37
    FileInformationIdFullDirectory, //  38
    FileInformationValidDataLength, //  39
    FileInformationShortName, //  40
    FileInformationIoCompletionNotification, //  41
    FileInformationIoStatusBlockRange, //  42
    FileInformationIoPriorityHint, //  43
    FileInformationSfioReserve, //  44
    FileInformationSfioVolume, //  45
    FileInformationHardLink, //  46
    FileInformationProcessIdsUsingFile, //  47
    FileInformationNormalizedName, //  48
    FileInformationNetworkPhysicalName, //  49
    FileInformationIdGlobalTxDirectory, //  50
    FileInformationIsRemoteDevice, //  51
    FileInformationAttributeCache, //  52
    FileInformationMaximum,
} NT_FILE_INFORMATION_CLASS, *PNT_FILE_INFORMATION_CLASS;

typedef struct _FILE_STREAM_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_DIRECTORY_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

/* xref: https://googleprojectzero.blogspot.de/2016/02/the-definitive-guide-on-win32-to-nt.html */
typedef struct _RTL_RELATIVE_NAME
{
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    void* CurDirRef;
} RTL_RELATIVE_NAME, *PRTL_RELATIVE_NAME;

typedef enum
{
    RtlPathTypeUnknown,
    RtlPathTypeUncAbsolute,
    RtlPathTypeDriveAbsolute,
    RtlPathTypeDriveRelative,
    RtlPathTypeRooted,
    RtlPathTypeRelative,
    RtlPathTypeLocalDevice,
    RtlPathTypeRootLocalDevice,
} RTL_PATH_TYPE;

#if !defined(DYNAMIC_NTNATIVE) || (!DYNAMIC_NTNATIVE)
NTSTATUS
NTAPI
RtlGetVersion(
    LPOSVERSIONINFOEXW lpVersionInformation
);

_Ret_maybenull_
_Success_(return != NULL)
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    _In_ PVOID Base
);

_Ret_maybenull_
_Success_(return != NULL)
PVOID
NTAPI
RtlImageDirectoryEntryToData(
    _In_ PVOID Base,
    _In_ BOOLEAN MappedAsImage,
    _In_ USHORT DirectoryEntry,
    _Out_ PULONG Size
);

_Ret_maybenull_
_Success_(return != NULL)
PVOID
NTAPI
RtlImageRvaToVa(
    _In_ PIMAGE_NT_HEADERS NtHeaders,
    _In_ PVOID Base,
    _In_ ULONG Rva,
    _Inout_opt_ PIMAGE_SECTION_HEADER *LastRvaSection
);

NTSTATUS
NTAPI
NtQueryEvent(
    _In_ HANDLE EventHandle,
    _In_ EVENT_INFORMATION_CLASS EventInformationClass,
    _Out_writes_bytes_(EventInformationLength) PVOID EventInformation,
    _In_ ULONG EventInformationLength,
    _Out_opt_ PULONG ReturnLength
);

NTSTATUS
NTAPI
NtQueryIoCompletion(
    _In_ HANDLE IoCompletionHandle,
    _In_ IO_COMPLETION_INFORMATION_CLASS InformationClass,
    _Out_ PVOID IoCompletionInformation,
    _In_ ULONG InformationBufferLength,
    _Out_opt_ PULONG RequiredLength
);

NTSTATUS
NTAPI
NtQueryMutant(
    _In_ HANDLE MutantHandle,
    _In_ MUTANT_INFORMATION_CLASS MutantInformationClass,
    _Out_writes_bytes_(MutantInformationLength) PVOID MutantInformation,
    _In_ ULONG MutantInformationLength,
    _Out_opt_ PULONG ReturnLength
);

NTSTATUS
NTAPI
NtQuerySemaphore(
    _In_ HANDLE SemaphoreHandle,
    _In_ SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    _Out_writes_bytes_(SemaphoreInformationLength) PVOID SemaphoreInformation,
    _In_ ULONG SemaphoreInformationLength,
    _Out_opt_ PULONG ReturnLength
);

NTSTATUS
NTAPI
NtQuerySection(
    _In_ HANDLE SectionHandle,
    _In_ SECTION_INFORMATION_CLASS SectionInformationClass,
    _Out_ PVOID SectionInformation,
    _In_ ULONG SectionInformationLength,
    _Out_opt_ PULONG ReturnLength
 );

NTSTATUS
NTAPI
NtQueryTimer(
    _In_ HANDLE TimerHandle,
    _In_ TIMER_INFORMATION_CLASS TimerInformationClass,
    _Out_ PVOID TimerInformation,
    _In_ ULONG TimerInformationLength,
    _Out_opt_ PULONG ReturnLength
);

NTSTATUS
NTAPI
NtOpenDirectoryObject(
    _Out_ PHANDLE DirectoryHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtQueryDirectoryObject(
    _In_ HANDLE DirectoryHandle,
    _Out_writes_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_ BOOLEAN ReturnSingleEntry,
    _In_ BOOLEAN RestartScan,
    _Inout_ PULONG Context,
    _Out_opt_ PULONG ReturnLength
);

NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    _Out_ PHANDLE LinkHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(
    _In_ HANDLE LinkHandle,
    _Inout_ PUNICODE_STRING LinkTarget,
    _Out_opt_ PULONG ReturnedLength
);

NTSTATUS
NTAPI
NtOpenEvent(
    _Out_ PHANDLE EventHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenMutant(
    _Out_ PHANDLE MutantHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenSection(
    _Out_ PHANDLE SectionHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenTimer(
    _Out_ PHANDLE TimerHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenSemaphore(
    _Out_ PHANDLE SemaphoreHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenEventPair(
    _Out_ PHANDLE EventPairHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenIoCompletion(
    _Out_ PHANDLE IoCompletionHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtCreateKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _Reserved_ ULONG TitleIndex,
    _In_opt_ PUNICODE_STRING Class,
    _In_ ULONG CreateOptions,
    _Out_opt_ PULONG Disposition
);

NTSTATUS
NTAPI
NtEnumerateKey(
    _In_ HANDLE KeyHandle,
    _In_ ULONG Index,
    _In_ KEY_INFORMATION_CLASS KeyInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyInformation,
    _In_ ULONG Length,
    _Out_ PULONG ResultLength
);

NTSTATUS
NTAPI
NtEnumerateValueKey(
    _In_ HANDLE KeyHandle,
    _In_ ULONG Index,
    _In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyValueInformation,
    _In_ ULONG Length,
    _Out_ PULONG ResultLength
);

NTSTATUS
NTAPI
NtQueryKey(
    _In_ HANDLE KeyHandle,
    _In_ KEY_INFORMATION_CLASS KeyInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyInformation,
    _In_ ULONG Length,
    _Out_ PULONG ResultLength
);

NTSTATUS
NTAPI
NtQueryValueKey(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING ValueName,
    _In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyValueInformation,
    _In_ ULONG Length,
    _Out_ PULONG ResultLength
);

NTSTATUS
NTAPI
NtSetValueKey(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING ValueName,
    _In_opt_ ULONG TitleIndex,
    _In_ ULONG Type,
    _In_reads_bytes_(DataSize) PVOID Data,
    _In_ ULONG DataSize
);

#pragma warning(suppress:28252)
NTSTATUS
NTAPI
NtRenameKey(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING NewName
);

NTSTATUS
NTAPI
RtlValidateUnicodeString(
    _In_ _Reserved_ ULONG Flags,
    _In_ PCUNICODE_STRING String
);

NTSTATUS
NTAPI
RtlDowncaseUnicodeString(
         PUNICODE_STRING DestinationString,
    _In_ PCUNICODE_STRING SourceString,
    _In_ BOOLEAN AllocateDestinationString
);

NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    _Inout_ PUNICODE_STRING  DestinationString,
    _In_ PCUNICODE_STRING SourceString,
    _In_ BOOLEAN AllocateDestinationString
);

WCHAR
NTAPI
RtlDowncaseUnicodeChar(
    _In_ WCHAR SourceCharacter
);

WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    _In_ WCHAR SourceCharacter
);

_Must_inspect_result_
LONG
NTAPI
RtlCompareUnicodeString(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
);

_Must_inspect_result_
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
);

VOID
NTAPI
RtlCopyUnicodeString(
    _In_ PUNICODE_STRING DestinationString,
    _In_ PCUNICODE_STRING SourceString
);

NTSTATUS
NTAPI
RtlAppendUnicodeStringToString(
    _In_ PUNICODE_STRING Destination,
    _In_ PCUNICODE_STRING Source
);

NTSTATUS
NTAPI
RtlAppendUnicodeToString(
    _In_ PUNICODE_STRING Destination,
    _In_opt_ PCWSTR Source
);

_Must_inspect_result_
BOOLEAN
NTAPI
RtlPrefixUnicodeString(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
);

NTSTATUS /* VOID in pre-Vista */
NTAPI
RtlGenerate8dot3Name(
    _In_ PCUNICODE_STRING Name,
    _In_ BOOLEAN AllowExtendedCharacters,
    _Inout_ PGENERATE_NAME_CONTEXT Context,
    _Inout_ PUNICODE_STRING Name8dot3
);

NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    _In_ PVOID VolumeDeviceObject,
    _Out_ PUNICODE_STRING DosName
);

NTSTATUS
NTAPI
NtCreateSection(
    _Out_ PHANDLE SectionHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_ PLARGE_INTEGER MaximumSize,
    _In_ ULONG SectionPageProtection,
    _In_ ULONG AllocationAttributes,
    _In_opt_ HANDLE FileHandle
);

NTSTATUS
NTAPI
NtMapViewOfSection(
    _In_ HANDLE SectionHandle,
    _In_ HANDLE ProcessHandle,
    _Inout_ PVOID *BaseAddress,
    _In_ ULONG_PTR ZeroBits,
    _In_ SIZE_T CommitSize,
    _Inout_opt_ PLARGE_INTEGER SectionOffset,
    _Inout_ PSIZE_T ViewSize,
    _In_ SECTION_INHERIT InheritDisposition,
    _In_ ULONG AllocationType,
    _In_ ULONG Win32Protect
);

NTSTATUS
NTAPI
NtUnmapViewOfSection(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress
);

NTSTATUS
NTAPI
NtQueryDirectoryFile(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ NT_FILE_INFORMATION_CLASS FileInformationClass,
    _In_ BOOLEAN ReturnSingleEntry,
    _In_opt_ PUNICODE_STRING FileName,
    _In_ BOOLEAN RestartScan
);

NTSTATUS
NTAPI
NtQueryInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ NT_FILE_INFORMATION_CLASS FileInformationClass
);

_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
NTAPI
RtlAllocateHeap(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags,
    _In_ SIZE_T Size
);

_Success_(return != 0)
BOOLEAN
NTAPI
RtlFreeHeap(
    _In_ PVOID HeapHandle,
    _In_opt_ ULONG Flags,
    _Frees_ptr_opt_ PVOID BaseAddress
);

NTSTATUS
NTAPI
RtlInitializeCriticalSection(
    _In_ PRTL_CRITICAL_SECTION CriticalSection
);

NTSTATUS
NTAPI
RtlDeleteCriticalSection(
    _In_ PRTL_CRITICAL_SECTION CriticalSection
);

NTSTATUS
NTAPI
RtlEnterCriticalSection(
    _In_ PRTL_CRITICAL_SECTION CriticalSection
);

NTSTATUS
NTAPI
RtlLeaveCriticalSection(
    _In_ PRTL_CRITICAL_SECTION CriticalSection
);

_When_(Status < 0, _Out_range_(> , 0))
_When_(Status >= 0, _Out_range_(== , 0))
ULONG
NTAPI
RtlNtStatusToDosError(
    NTSTATUS Status
);

_Success_(return != 0)
BOOLEAN
NTAPI
RtlDosPathNameToNtPathName_U(
    _In_ PCWSTR DosFileName,
    _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR *FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME RelativeName
);

NTSTATUS
NTAPI
RtlDosPathNameToNtPathName_U_WithStatus(
    _In_ PCWSTR DosFileName,
    _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR *FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME RelativeName
);

/* xref: https://googleprojectzero.blogspot.de/2016/02/the-definitive-guide-on-win32-to-nt.html */
_Success_(return != 0)
BOOLEAN
NTAPI
RtlDosPathNameToRelativeNtPathName_U(
    _In_ PCWSTR DosFileName,
    _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR* FilePath,
    _Out_opt_ PRTL_RELATIVE_NAME RelativeName
);

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(
    _In_ PCWSTR Path
);

ULONG
NTAPI
RtlGetFullPathName_U(
    _In_ PWSTR FileName,
    _In_ ULONG BufferLength,
    _Out_writes_bytes_(BufferLength) PWSTR Buffer,
    _Out_opt_ PWSTR *FilePart
);

#if defined(DDKBUILD)
NTSTATUS
NTAPI
NtQueryObject(
   _In_opt_ HANDLE Handle,
   _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
   _Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
   _In_ ULONG ObjectInformationLength,
   _Out_opt_ PULONG ReturnLength
   );

NTSTATUS
NTAPI
NtOpenFile(
    _Out_ PHANDLE FileHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG ShareAccess,
    _In_ ULONG OpenOptions
);
#endif // DDKBUILD
#endif // DYNAMIC_NTNATIVE

typedef NTSTATUS (NTAPI *RtlGetVersion_t)(LPOSVERSIONINFOEXW);
typedef PIMAGE_NT_HEADERS (NTAPI *RtlImageNtHeader_t)(_In_ PVOID);
typedef PVOID (NTAPI *RtlImageDirectoryEntryToData_t)(_In_ PVOID, _In_ BOOLEAN, _In_ USHORT, _Out_ PULONG);
typedef PVOID (NTAPI *RtlImageRvaToVa_t)(_In_ PIMAGE_NT_HEADERS, _In_ PVOID, _In_ ULONG, _Inout_opt_ PIMAGE_SECTION_HEADER *);
typedef NTSTATUS (NTAPI *NtQueryEvent_t)(_In_ HANDLE, _In_ EVENT_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryIoCompletion_t)(_In_ HANDLE, _In_ IO_COMPLETION_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryMutant_t)(_In_ HANDLE, _In_ MUTANT_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQuerySemaphore_t)(_In_ HANDLE, _In_ SEMAPHORE_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQuerySection_t)(_In_ HANDLE, _In_ SECTION_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryTimer_t)(_In_ HANDLE, _In_ TIMER_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtOpenDirectoryObject_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtQueryDirectoryObject_t)(_In_ HANDLE, _Out_ PVOID, _In_ ULONG, _In_ BOOLEAN, _In_ BOOLEAN, _Inout_ PULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtOpenSymbolicLinkObject_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtQuerySymbolicLinkObject_t)(_In_ HANDLE, _Inout_ PUNICODE_STRING, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtOpenEvent_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenMutant_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenSection_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenTimer_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenSemaphore_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenEventPair_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenIoCompletion_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenKey_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtCreateKey_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES, _Reserved_ ULONG, _In_opt_ PUNICODE_STRING, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtEnumerateKey_t)(_In_ HANDLE, _In_ ULONG, _In_ KEY_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtEnumerateValueKey_t)(_In_ HANDLE, _In_ ULONG, _In_ KEY_VALUE_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryKey_t)(_In_ HANDLE, _In_ KEY_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryValueKey_t)(_In_ HANDLE, _In_ PUNICODE_STRING, _In_ KEY_VALUE_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);
typedef NTSTATUS (NTAPI *NtSetValueKey_t)(_In_ HANDLE, _In_ PUNICODE_STRING, _In_opt_ ULONG, _In_ ULONG, _In_ PVOID, _In_ ULONG);
typedef NTSTATUS (NTAPI *NtRenameKey_t)(_In_ HANDLE, _In_ PUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlValidateUnicodeString_t)(_In_ _Reserved_ ULONG, _In_ PCUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlDowncaseUnicodeString_t)(PUNICODE_STRING, _In_ PCUNICODE_STRING, _In_ BOOLEAN);
typedef NTSTATUS (NTAPI *RtlUpcaseUnicodeString_t)(_Inout_ PUNICODE_STRING, _In_ PCUNICODE_STRING, _In_ BOOLEAN);
typedef WCHAR (NTAPI *RtlDowncaseUnicodeChar_t)(_In_ WCHAR);
typedef WCHAR (NTAPI *RtlUpcaseUnicodeChar_t)(_In_ WCHAR);
typedef LONG (NTAPI *RtlCompareUnicodeString_t)(_In_ PCUNICODE_STRING, _In_ PCUNICODE_STRING, _In_ BOOLEAN);
typedef BOOLEAN (NTAPI *RtlEqualUnicodeString_t)(_In_ PCUNICODE_STRING, _In_ PCUNICODE_STRING, _In_ BOOLEAN);
typedef VOID (NTAPI *RtlCopyUnicodeString_t)(_In_ PUNICODE_STRING, _In_ PCUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlAppendUnicodeStringToString_t)(_In_ PUNICODE_STRING, _In_ PCUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlAppendUnicodeToString_t)(_In_ PUNICODE_STRING, _In_opt_ PCWSTR);
typedef BOOLEAN (NTAPI *RtlPrefixUnicodeString_t)(_In_ PCUNICODE_STRING, _In_ PCUNICODE_STRING, _In_ BOOLEAN);
typedef NTSTATUS (NTAPI *RtlGenerate8dot3Name_t)(_In_ PCUNICODE_STRING, _In_ BOOLEAN, _Inout_ PGENERATE_NAME_CONTEXT, _Inout_ PUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlVolumeDeviceToDosName_t)(_In_ PVOID, _Out_ PUNICODE_STRING);
typedef NTSTATUS (NTAPI *NtCreateSection_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_opt_ POBJECT_ATTRIBUTES, _In_opt_ PLARGE_INTEGER, _In_ ULONG, _In_ ULONG, _In_opt_ HANDLE);
typedef NTSTATUS (NTAPI *NtMapViewOfSection_t)(_In_ HANDLE, _In_ HANDLE, _Inout_ PVOID *, _In_ ULONG_PTR, _In_ SIZE_T, _Inout_opt_ PLARGE_INTEGER, _Inout_ PSIZE_T, _In_ SECTION_INHERIT, _In_ ULONG, _In_ ULONG);
typedef NTSTATUS (NTAPI *NtUnmapViewOfSection_t)(_In_ HANDLE, _In_opt_ PVOID);
typedef NTSTATUS (NTAPI *NtQueryDirectoryFile_t)(_In_ HANDLE, _In_opt_ HANDLE, _In_opt_ PIO_APC_ROUTINE, _In_opt_ PVOID, _Out_ PIO_STATUS_BLOCK, _Out_ PVOID, _In_ ULONG, _In_ NT_FILE_INFORMATION_CLASS, _In_ BOOLEAN, _In_opt_ PUNICODE_STRING, _In_ BOOLEAN);
typedef NTSTATUS (NTAPI *NtQueryInformationFile_t)(_In_ HANDLE, _Out_ PIO_STATUS_BLOCK, _Out_ PVOID, _In_ ULONG, _In_ NT_FILE_INFORMATION_CLASS);
typedef PVOID (NTAPI *RtlAllocateHeap_t)(_In_ PVOID, _In_ ULONG, _In_ SIZE_T);
typedef BOOLEAN (NTAPI *RtlFreeHeap_t)(_In_ PVOID, _In_opt_ ULONG, _Frees_ptr_opt_ PVOID);
typedef NTSTATUS (NTAPI *RtlInitializeCriticalSection_t)(_In_ PRTL_CRITICAL_SECTION);
typedef NTSTATUS (NTAPI *RtlDeleteCriticalSection_t)(_In_ PRTL_CRITICAL_SECTION);
typedef NTSTATUS (NTAPI *RtlEnterCriticalSection_t)(_In_ PRTL_CRITICAL_SECTION);
typedef NTSTATUS (NTAPI *RtlLeaveCriticalSection_t)(_In_ PRTL_CRITICAL_SECTION);
typedef ULONG (NTAPI *RtlNtStatusToDosError_t)(NTSTATUS);
typedef BOOLEAN (NTAPI *RtlDosPathNameToNtPathName_U_t)(_In_ PCWSTR, _Out_ PUNICODE_STRING, _Out_opt_ PWSTR *, _Out_opt_ PRTL_RELATIVE_NAME);
typedef NTSTATUS (NTAPI *RtlDosPathNameToNtPathName_U_WithStatus_t)(_In_ PCWSTR, _Out_ PUNICODE_STRING, _Out_opt_ PWSTR *, _Out_opt_ PRTL_RELATIVE_NAME);
typedef BOOLEAN (NTAPI *RtlDosPathNameToRelativeNtPathName_U_t)(_In_ PCWSTR, _Out_ PUNICODE_STRING, _Out_opt_ PWSTR*, _Out_opt_ PRTL_RELATIVE_NAME);
typedef RTL_PATH_TYPE (NTAPI *RtlDetermineDosPathNameType_U_t)(_In_ PCWSTR);
typedef ULONG (NTAPI *RtlGetFullPathName_U_t)(_In_ PWSTR, _In_ ULONG, _Out_ PWSTR, _Out_opt_ PWSTR*);
/* Those from wintrnl.h */
typedef NTSTATUS (NTAPI *NtClose_t)(_In_ HANDLE);
typedef NTSTATUS (NTAPI *NtCreateFile_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES, _Out_ PIO_STATUS_BLOCK, _In_opt_ PLARGE_INTEGER AllocationSize, _In_ ULONG FileAttributes, _In_ ULONG ShareAccess, _In_ ULONG CreateDisposition, _In_ ULONG CreateOptions, _In_opt_ PVOID EaBuffer, _In_ ULONG EaLength);
typedef NTSTATUS (NTAPI *NtOpenFile_t)(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _In_ ULONG ShareAccess, _In_ ULONG);
typedef NTSTATUS (NTAPI *NtRenameKey_t)(_In_ HANDLE, _In_ PUNICODE_STRING);
typedef NTSTATUS (NTAPI *NtNotifyChangeMultipleKeys_t)(_In_ HANDLE, _In_opt_ ULONG, _In_opt_ OBJECT_ATTRIBUTES[], _In_opt_ HANDLE, _In_opt_ PIO_APC_ROUTINE, _In_opt_ PVOID, _Out_ PIO_STATUS_BLOCK, _In_ ULONG, _In_ BOOLEAN, _Out_writes_bytes_opt_(BufferSize) PVOID, _In_ ULONG, _In_ BOOLEAN);
typedef NTSTATUS (NTAPI *NtQueryMultipleValueKey_t)(_In_ HANDLE, _Inout_ PKEY_VALUE_ENTRY, _In_ ULONG, _Out_ PVOID, _Inout_ PULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtSetInformationKey_t)(_In_ HANDLE, _In_ KEY_SET_INFORMATION_CLASS, _In_ PVOID, _In_ ULONG);
typedef NTSTATUS (NTAPI *NtDeviceIoControlFile_t)(_In_ HANDLE, _In_opt_ HANDLE, _In_opt_ PIO_APC_ROUTINE, _In_opt_ PVOID, _Out_ PIO_STATUS_BLOCK, _In_ ULONG, _In_ PVOID, _In_ ULONG, _Out_opt_ PVOID, _In_ ULONG);
typedef NTSTATUS (NTAPI *NtWaitForSingleObject_t)(_In_ HANDLE, _In_ BOOLEAN, _In_opt_ PLARGE_INTEGER);
typedef BOOLEAN (NTAPI *RtlIsNameLegalDOS8Dot3_t)(_In_ PUNICODE_STRING, _Inout_opt_ POEM_STRING, _Inout_opt_ PBOOLEAN);
typedef ULONG (NTAPI *RtlNtStatusToDosError_t)(NTSTATUS);
typedef NTSTATUS (NTAPI *NtQueryInformationProcess_t)(_In_ HANDLE, _In_ PROCESSINFOCLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryInformationThread_t)(_In_ HANDLE, _In_ THREADINFOCLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQueryObject_t)(_In_opt_ HANDLE, _In_ OBJECT_INFORMATION_CLASS, _Out_opt_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQuerySystemInformation_t)(_In_ SYSTEM_INFORMATION_CLASS, _Out_ PVOID, _In_ ULONG, _Out_opt_ PULONG);
typedef NTSTATUS (NTAPI *NtQuerySystemTime_t)(_Out_ PLARGE_INTEGER);
typedef NTSTATUS (NTAPI *RtlLocalTimeToSystemTime_t)(_In_ PLARGE_INTEGER, _Out_ PLARGE_INTEGER);
typedef BOOLEAN (NTAPI *RtlTimeToSecondsSince1970_t)(_In_ PLARGE_INTEGER, _Out_ PULONG);
typedef VOID (NTAPI *RtlFreeAnsiString_t)(PANSI_STRING);
typedef VOID (NTAPI *RtlFreeUnicodeString_t)(PUNICODE_STRING);
typedef VOID (NTAPI *RtlFreeOemString_t)(POEM_STRING);
typedef VOID (NTAPI *RtlInitString_t)(PSTRING, PCSZ);
typedef VOID (NTAPI *RtlInitAnsiString_t)(PANSI_STRING, PCSZ);
typedef VOID (NTAPI *RtlInitUnicodeString_t)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS (NTAPI *RtlAnsiStringToUnicodeString_t)(PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
typedef NTSTATUS (NTAPI *RtlUnicodeStringToAnsiString_t)(PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
typedef NTSTATUS (NTAPI *RtlUnicodeStringToOemString_t)(POEM_STRING, PCUNICODE_STRING, BOOLEAN);
typedef NTSTATUS (NTAPI *RtlUnicodeToMultiByteSize_t)(_Out_ PULONG, _In_ PWCH, _In_ ULONG);
typedef NTSTATUS (NTAPI *RtlCharToInteger_t)(PCSZ, ULONG, PULONG);
typedef NTSTATUS (NTAPI *RtlConvertSidToUnicodeString_t)(PUNICODE_STRING, PSID, BOOLEAN);
typedef ULONG (NTAPI *RtlUniform_t)(PULONG);

/*
  Use this to declare a variable of the name of a native function and of its
  proper type.
    Example     : NTNATIVE_FUNC(RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion;
*/
#define NTNATIVE_FUNC(x) x##_t x
/*
  Use to retrieve a function's pointer by passing a handle. Uses GetProcAddress()
  to do the job.
    Example     : NTNATIVE_GETPROCADDR(hNtDll, RtlGetVersion);
    Expands to  : GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTNATIVE_GETPROCADDR(mod, x) GetProcAddress(mod, #x)
/*
  Use to coerce the type returned from GetProcAddress()
    Example     : NTNATIVE_GETFUNC(hNtDll, RtlGetVersion)
    Expands to  : (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion")
*/
#define NTNATIVE_GETFUNC(mod, x) (x##_t)NTNATIVE_GETPROCADDR(mod, x)
/*
  Use to declare a variable of the same name as the function and assign it the
  function's pointer by passing a module handle. Uses GetProcAddress() to do the
  job.
    Example     : NTNATIVE_DEFFUNC(hNtDll, RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTNATIVE_DEFFUNC(mod, x) NTNATIVE_FUNC(x) = NTNATIVE_GETFUNC(mod, x)
/*
  Use to declare a variable of the same name as the function and assign it the
  function's pointer. Uses GetProcAddress() to do the job.
    Example     : NTDLL_DEFFUNC(RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTDLL_DEFFUNC(x) NTNATIVE_DEFFUNC(hNtDll, x)

#define LOCAL_NTNATIVE_FUNC(x) \
    static NTNATIVE_FUNC(x) = 0; \
    do { \
    static HMODULE hNtDll = NULL; \
    if (!hNtDll) { hNtDll = GetModuleHandle(L"ntdll.dll"); } \
    if (!hNtDll) { return STATUS_NOT_IMPLEMENTED; } \
    if (!x) { x = NTNATIVE_GETFUNC(hNtDll, x); } \
    if (!x) { return STATUS_NOT_IMPLEMENTED; } \
    } while(0)

#define ZwClose NtClose
#define ZwCreateFile NtCreateFile
#define ZwOpenFile NtOpenFile
#define ZwDeviceIoControlFile NtDeviceIoControlFile
#define ZwWaitForSingleObject NtWaitForSingleObject
#define ZwQueryInformationProcess NtQueryInformationProcess
#define ZwQueryInformationThread NtQueryInformationThread
#define ZwQueryObject NtQueryObject
#define ZwQuerySystemInformation NtQuerySystemInformation
#define ZwQuerySystemTime NtQuerySystemTime
#define ZwQueryEvent NtQueryEvent
#define ZwQueryIoCompletion NtQueryIoCompletion
#define ZwQueryMutant NtQueryMutant
#define ZwQuerySemaphore NtQuerySemaphore
#define ZwQuerySection NtQuerySection
#define ZwQueryTimer NtQueryTimer
#define ZwOpenDirectoryObject NtOpenDirectoryObject
#define ZwQueryDirectoryObject NtQueryDirectoryObject
#define ZwOpenSymbolicLinkObject NtOpenSymbolicLinkObject
#define ZwQuerySymbolicLinkObject NtQuerySymbolicLinkObject
#define ZwOpenEvent NtOpenEvent
#define ZwOpenMutant NtOpenMutant
#define ZwOpenSection NtOpenSection
#define ZwOpenSemaphore NtOpenSemaphore
#define ZwOpenTimer NtOpenTimer
#define ZwOpenEventPair NtOpenEventPair
#define ZwOpenIoCompletion NtOpenIoCompletion
#define ZwOpenKey NtOpenKey
#define ZwCreateKey NtCreateKey
#define ZwEnumerateKey NtEnumerateKey
#define ZwEnumerateValueKey NtEnumerateValueKey
#define ZwQueryValueKey NtQueryValueKey
#define ZwSetValueKey NtSetValueKey
#define ZwRenameKey NtRenameKey
#define ZwCreateSection NtCreateSection
#define ZwMapViewOfSection NtMapViewOfSection
#define ZwUnmapViewOfSection NtUnmapViewOfSection
#define ZwQueryDirectoryFile NtQueryDirectoryFile
#define ZwQueryInformationFile NtQueryInformationFile

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
namespace NT {
#endif

#ifndef WIN32_FILE_NAMESPACE
#define WIN32_FILE_NAMESPACE        L"\\\\?\\"
#endif // WIN32_FILE_NAMESPACE
#ifndef WIN32_DEVICE_NAMESPACE
#define WIN32_DEVICE_NAMESPACE      L"\\\\.\\"
#endif // WIN32_DEVICE_NAMESPACE
#ifndef NT_OBJMGR_NAMESPACE
#define NT_OBJMGR_NAMESPACE         L"\\??\\"
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

static UNICODE_STRING const sWin32FileNsPfx = RTL_CONSTANT_STRING(WIN32_FILE_NAMESPACE);
static UNICODE_STRING const sWin32DeviceNsPfx = RTL_CONSTANT_STRING(WIN32_DEVICE_NAMESPACE);
static UNICODE_STRING const sNtObjMgrNsPfx = RTL_CONSTANT_STRING(NT_OBJMGR_NAMESPACE);

#if defined(__cplusplus)
}
#endif


#endif // __NTNATIVE_H_VER__
