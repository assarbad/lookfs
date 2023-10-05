#ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#    define WINVER 0x0500
#endif
#include <Windows.h>

// Fake the SAL1 annotations where they don't exist.
#if !defined(__in_bcount) && !defined(_In_reads_bytes_)
#    define __success(x)
#    define __field_range(x, y)
#    define __field_nullterminated
#    define __in
#    define __in_z
#    define __in_bcount(x)
#    define __in_opt
#    define __inout
#    define __inout_opt
#    define __out
#    define __out_bcount(x)
#    define __out_opt
#    define __out_bcount_opt(x)
#    define __reserved
#endif

// Fake the SAL2 annotations where they don't exist.
#if defined(__in_bcount) && !defined(_In_reads_bytes_)
#    define _Success_(x)              __success(x)
#    define _Field_range_(x, y)       __field_range(x, y)
#    define _Field_z_                 __field_nullterminated
#    define _In_                      __in
#    define _In_z_                    __in_z
#    define _In_reads_bytes_(x)       __in_bcount(x)
#    define _In_opt_                  __in_opt
#    define _Inout_                   __inout
#    define _Inout_opt_               __inout_opt
#    define _Out_                     __out
#    define _Out_writes_bytes_(x)     __out_bcount(x)
#    define _Out_opt_                 __out_opt
#    define _Out_writes_bytes_opt_(x) __out_bcount_opt(x)
#    define _Reserved_                __reserved
#endif

#ifndef _Must_inspect_result_
#    define _Must_inspect_result_
#endif

#ifndef _Ret_maybenull_
#    define _Ret_maybenull_
#endif

#ifndef _Ret_writes_bytes_maybenull_
#    define _Ret_writes_bytes_maybenull_(Size)
#endif

#ifndef _Post_writable_byte_size_
#    define _Post_writable_byte_size_(Size)
#endif

#ifndef _Post_invalid_
#    define _Post_invalid_
#endif

#ifndef _Notnull_
#    define _Notnull_
#endif

#ifndef _Pre_
#    define _Pre_
#endif

#ifndef _When_
#    define _When_(x, y)
#endif

#ifndef _In_range_
#    define _In_range_(x, y)
#endif

#ifndef _Out_range_
#    define _Out_range_(x, y)
#endif

#ifndef _Frees_ptr_opt_
#    define _Frees_ptr_opt_
#endif

#ifndef _Frees_ptr_
#    define _Frees_ptr_
#endif

#ifndef _Inout_updates_opt_
#    define _Inout_updates_opt_(x)
#endif

#ifndef _Inout_updates_
#    define _Inout_updates_(x)
#endif

#ifndef _Out_writes_bytes_to_opt_
#    define _Out_writes_bytes_to_opt_(x, y)
#endif

#ifndef _In_reads_opt_
#    define _In_reads_opt_(x)
#endif

#ifndef _Strict_type_match_
#    define _Strict_type_match_
#endif

#ifndef _Outptr_
#    define _Outptr_
#endif

#ifndef _Outptr_result_maybenull_
#    define _Outptr_result_maybenull_
#endif

#ifndef _Writable_elements_
#    define _Writable_elements_(x)
#endif

typedef struct
{
    int x;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef struct IO_STATUS_BLOCK* PIO_STATUS_BLOCK;
typedef struct ANSI_STRING* PANSI_STRING;
typedef struct ANSI_STRING const* PCANSI_STRING;
typedef struct UNICODE_STRING* PUNICODE_STRING;
typedef struct UNICODE_STRING const* PCUNICODE_STRING;
typedef struct OEM_STRING* POEM_STRING;
typedef struct OEM_STRING const* PCOEM_STRING;
typedef struct STRING* PSTRING;
typedef void* PKEY_VALUE_ENTRY;
typedef LPCSTR PCSZ;
typedef BOOLEAN* PBOOLEAN;
typedef FARPROC PIO_APC_ROUTINE;
typedef struct RTL_RELATIVE_NAME* PRTL_RELATIVE_NAME;
typedef struct GENERATE_NAME_CONTEXT* PGENERATE_NAME_CONTEXT;
typedef int NT_FILE_INFORMATION_CLASS;       // enum
typedef int RTL_PATH_TYPE;                   // enum
typedef int FILE_INFORMATION_CLASS;          // enum
typedef int THREADINFOCLASS;                 // enum
typedef int PROCESSINFOCLASS;                // enum
typedef int OBJECT_INFORMATION_CLASS;        // enum
typedef int SYSTEM_INFORMATION_CLASS;        // enum
typedef int EVENT_INFORMATION_CLASS;         // enum
typedef int IO_COMPLETION_INFORMATION_CLASS; // enum
typedef int MUTANT_INFORMATION_CLASS;        // enum
typedef int SEMAPHORE_INFORMATION_CLASS;     // enum
typedef int SECTION_INFORMATION_CLASS;       // enum
typedef int TIMER_INFORMATION_CLASS;         // enum
typedef int KEY_INFORMATION_CLASS;           // enum
typedef int KEY_VALUE_INFORMATION_CLASS;     // enum
typedef int KEY_INFORMATION_CLASS;           // enum
typedef int KEY_VALUE_INFORMATION_CLASS;     // enum
typedef int KEY_SET_INFORMATION_CLASS;       // enum
typedef ULONG SECTION_INHERIT;

NTSTATUS
NTAPI
NtAdjustGroupsToken(_In_ HANDLE TokenHandle,
                    _In_ BOOLEAN ResetToDefault,
                    _In_opt_ PTOKEN_GROUPS NewState,
                    _In_range_(>=, sizeof(TOKEN_GROUPS)) ULONG BufferLength,
                    _Out_writes_bytes_to_opt_(BufferLength, *ReturnLength) PTOKEN_GROUPS PreviousState,
                    _Out_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtAdjustPrivilegesToken(_In_ HANDLE TokenHandle,
                        _In_ BOOLEAN DisableAllPrivileges,
                        _In_opt_ PTOKEN_PRIVILEGES NewState,
                        _In_ ULONG BufferLength,
                        _Out_writes_bytes_to_opt_(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
                        _Out_ _When_(PreviousState == NULL, _Out_opt_) PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtClose(_In_ HANDLE Handle)
{
    return 0;
}

NTSTATUS
NTAPI
NtCreateFile(_Out_ PHANDLE FileHandle,
             _In_ ACCESS_MASK DesiredAccess,
             _In_ POBJECT_ATTRIBUTES ObjectAttributes,
             _Out_ PIO_STATUS_BLOCK IoStatusBlock,
             _In_opt_ PLARGE_INTEGER AllocationSize,
             _In_ ULONG FileAttributes,
             _In_ ULONG ShareAccess,
             _In_ ULONG CreateDisposition,
             _In_ ULONG CreateOptions,
             _In_opt_ PVOID EaBuffer,
             _In_ ULONG EaLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtCreateIoCompletion(_Out_ PHANDLE IoCompletionHandle, _In_ ACCESS_MASK DesiredAccess, _Inout_opt_ POBJECT_ATTRIBUTES ObjectAttributes, _In_opt_ ULONG Count)
{
    return 0;
}

NTSTATUS
NTAPI
NtCreateKey(_Out_ PHANDLE KeyHandle,
            _In_ ACCESS_MASK DesiredAccess,
            _In_ POBJECT_ATTRIBUTES ObjectAttributes,
            _Reserved_ ULONG TitleIndex,
            _In_opt_ PUNICODE_STRING Class,
            _In_ ULONG CreateOptions,
            _Out_opt_ PULONG Disposition)
{
    return 0;
}

NTSTATUS
NTAPI
NtCreateSection(_Out_ PHANDLE SectionHandle,
                _In_ ACCESS_MASK DesiredAccess,
                _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
                _In_opt_ PLARGE_INTEGER MaximumSize,
                _In_ ULONG SectionPageProtection,
                _In_ ULONG AllocationAttributes,
                _In_opt_ HANDLE FileHandle)
{
    return 0;
}

NTSTATUS
NTAPI
NtDeleteValueKey(_In_ HANDLE KeyHandle, _In_ PUNICODE_STRING ValueName)
{
    return 0;
}

NTSTATUS
NTAPI
NtDeviceIoControlFile(_In_ HANDLE FileHandle,
                      _In_opt_ HANDLE Event,
                      _In_opt_ PIO_APC_ROUTINE ApcRoutine,
                      _In_opt_ PVOID ApcContext,
                      _Out_ PIO_STATUS_BLOCK IoStatusBlock,
                      _In_ ULONG IoControlCode,
                      _In_opt_ PVOID InputBuffer,
                      _In_ ULONG InputBufferLength,
                      _Out_opt_ PVOID OutputBuffer,
                      _In_ ULONG OutputBufferLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtDuplicateToken(_In_ HANDLE ExistingTokenHandle,
                 _In_ ACCESS_MASK DesiredAccess,
                 _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
                 _In_ BOOLEAN EffectiveOnly,
                 _In_ TOKEN_TYPE TokenType,
                 _Out_ PHANDLE NewTokenHandle)
{
    return 0;
}

NTSTATUS
NTAPI
NtEnumerateKey(_In_ HANDLE KeyHandle,
               _In_ ULONG Index,
               _In_ KEY_INFORMATION_CLASS KeyInformationClass,
               _Out_writes_bytes_opt_(Length) PVOID KeyInformation,
               _In_ ULONG Length,
               _Out_ PULONG ResultLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtEnumerateValueKey(_In_ HANDLE KeyHandle,
                    _In_ ULONG Index,
                    _In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                    _Out_writes_bytes_opt_(Length) PVOID KeyValueInformation,
                    _In_ ULONG Length,
                    _Out_ PULONG ResultLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtMapViewOfSection(_In_ HANDLE SectionHandle,
                   _In_ HANDLE ProcessHandle,
                   _Inout_ PVOID* BaseAddress,
                   _In_ ULONG_PTR ZeroBits,
                   _In_ SIZE_T CommitSize,
                   _Inout_opt_ PLARGE_INTEGER SectionOffset,
                   _Inout_ PSIZE_T ViewSize,
                   _In_ SECTION_INHERIT InheritDisposition,
                   _In_ ULONG AllocationType,
                   _In_ ULONG Win32Protect)
{
    return 0;
}

NTSTATUS
NTAPI
NtNotifyChangeMultipleKeys(_In_ HANDLE MasterKeyHandle,
                           _In_opt_ ULONG Count,
                           _In_reads_opt_(Count) OBJECT_ATTRIBUTES SubordinateObjects[],
                           _In_opt_ HANDLE Event,
                           _In_opt_ PIO_APC_ROUTINE ApcRoutine,
                           _In_opt_ PVOID ApcContext,
                           _Out_ PIO_STATUS_BLOCK IoStatusBlock,
                           _In_ ULONG CompletionFilter,
                           _In_ BOOLEAN WatchTree,
                           _Out_writes_bytes_opt_(BufferSize) PVOID Buffer,
                           _In_ ULONG BufferSize,
                           _In_ BOOLEAN Asynchronous)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenDirectoryObject(_Out_ PHANDLE DirectoryHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenEvent(_Out_ PHANDLE EventHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenEventPair(_Out_ PHANDLE EventPairHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenFile(_Out_ PHANDLE FileHandle,
           _In_ ACCESS_MASK DesiredAccess,
           _In_ POBJECT_ATTRIBUTES ObjectAttributes,
           _Out_ PIO_STATUS_BLOCK IoStatusBlock,
           _In_ ULONG ShareAccess,
           _In_ ULONG OpenOptions)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenIoCompletion(_Out_ PHANDLE IoCompletionHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenKey(_Out_ PHANDLE KeyHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenMutant(_Out_ PHANDLE MutantHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenProcessToken(_In_ HANDLE ProcessHandle, _In_ ACCESS_MASK DesiredAccess, _Out_ PHANDLE TokenHandle)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenSection(_Out_ PHANDLE SectionHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenSemaphore(_Out_ PHANDLE SemaphoreHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(_Out_ PHANDLE LinkHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenThreadToken(_In_ HANDLE ThreadHandle, _In_ ACCESS_MASK DesiredAccess, _In_ BOOLEAN OpenAsSelf, _Out_ PHANDLE TokenHandle)
{
    return 0;
}

NTSTATUS
NTAPI
NtOpenTimer(_Out_ PHANDLE TimerHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes)
{
    return 0;
}

NTSTATUS
NTAPI
NtPrivilegeCheck(_In_ HANDLE ClientToken, _Inout_ PPRIVILEGE_SET RequiredPrivileges, _Out_ PBOOLEAN Result)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryDirectoryFile(_In_ HANDLE FileHandle,
                     _In_opt_ HANDLE Event,
                     _In_opt_ PIO_APC_ROUTINE ApcRoutine,
                     _In_opt_ PVOID ApcContext,
                     _Out_ PIO_STATUS_BLOCK IoStatusBlock,
                     _Out_writes_bytes_(Length) PVOID FileInformation,
                     _In_ ULONG Length,
                     _In_ FILE_INFORMATION_CLASS FileInformationClass,
                     _In_ BOOLEAN ReturnSingleEntry,
                     _In_opt_ PUNICODE_STRING FileName,
                     _In_ BOOLEAN RestartScan)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryDirectoryObject(_In_ HANDLE DirectoryHandle,
                       _Out_writes_bytes_(Length) PVOID Buffer,
                       _In_ ULONG Length,
                       _In_ BOOLEAN ReturnSingleEntry,
                       _In_ BOOLEAN RestartScan,
                       _Inout_ PULONG Context,
                       _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryEvent(_In_ HANDLE EventHandle,
             _In_ EVENT_INFORMATION_CLASS EventInformationClass,
             _Out_writes_bytes_(EventInformationLength) PVOID EventInformation,
             _In_ ULONG EventInformationLength,
             _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryInformationFile(_In_ HANDLE FileHandle,
                       _Out_ PIO_STATUS_BLOCK IoStatusBlock,
                       _Out_writes_bytes_(Length) PVOID FileInformation,
                       _In_ ULONG Length,
                       _In_ NT_FILE_INFORMATION_CLASS FileInformationClass)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryInformationProcess(_In_ HANDLE ProcessHandle,
                          _In_ PROCESSINFOCLASS ProcessInformationClass,
                          _Out_ PVOID ProcessInformation,
                          _In_ ULONG ProcessInformationLength,
                          _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryInformationThread(_In_ HANDLE ThreadHandle,
                         _In_ THREADINFOCLASS ThreadInformationClass,
                         _Out_ PVOID ThreadInformation,
                         _In_ ULONG ThreadInformationLength,
                         _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryInformationToken(_In_ HANDLE TokenHandle,
                        _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
                        _Out_writes_bytes_to_opt_(TokenInformationLength, *ReturnLength) PVOID TokenInformation,
                        _In_ ULONG TokenInformationLength,
                        _Out_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryIoCompletion(_In_ HANDLE IoCompletionHandle,
                    _In_ IO_COMPLETION_INFORMATION_CLASS InformationClass,
                    _Out_ PVOID IoCompletionInformation,
                    _In_ ULONG InformationBufferLength,
                    _Out_opt_ PULONG RequiredLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryKey(_In_ HANDLE KeyHandle,
           _In_ KEY_INFORMATION_CLASS KeyInformationClass,
           _Out_writes_bytes_opt_(Length) PVOID KeyInformation,
           _In_ ULONG Length,
           _Out_ PULONG ResultLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryMultipleValueKey(_In_ HANDLE KeyHandle,
                        _Inout_updates_(EntryCount) PKEY_VALUE_ENTRY ValueEntries,
                        _In_ ULONG EntryCount,
                        _Out_writes_bytes_(*BufferLength) PVOID ValueBuffer,
                        _Inout_ PULONG BufferLength,
                        _Out_opt_ PULONG RequiredBufferLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryMutant(_In_ HANDLE MutantHandle,
              _In_ MUTANT_INFORMATION_CLASS MutantInformationClass,
              _Out_writes_bytes_(MutantInformationLength) PVOID MutantInformation,
              _In_ ULONG MutantInformationLength,
              _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryObject(_In_opt_ HANDLE Handle,
              _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
              _Out_writes_bytes_(ObjectInformationLength) PVOID ObjectInformation,
              _In_ ULONG ObjectInformationLength,
              _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryOpenSubKeys(_In_ POBJECT_ATTRIBUTES TargetKey, _Out_ PULONG HandleCount)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySection(_In_ HANDLE SectionHandle,
               _In_ SECTION_INFORMATION_CLASS SectionInformationClass,
               _Out_ PVOID SectionInformation,
               _In_ ULONG SectionInformationLength,
               _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySecurityObject(_In_ HANDLE Handle,
                      _In_ SECURITY_INFORMATION SecurityInformation,
                      _Out_writes_bytes_opt_(Length) PSECURITY_DESCRIPTOR SecurityDescriptor,
                      _In_ ULONG Length,
                      _Out_ PULONG LengthNeeded)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySemaphore(_In_ HANDLE SemaphoreHandle,
                 _In_ SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
                 _Out_writes_bytes_(SemaphoreInformationLength) PVOID SemaphoreInformation,
                 _In_ ULONG SemaphoreInformationLength,
                 _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(_In_ HANDLE LinkHandle, _Inout_ PUNICODE_STRING LinkTarget, _Out_opt_ PULONG ReturnedLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySystemInformation(_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
                         _Out_ PVOID SystemInformation,
                         _In_ ULONG SystemInformationLength,
                         _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQuerySystemTime(_Out_ PLARGE_INTEGER SystemTime)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryTimer(_In_ HANDLE TimerHandle,
             _In_ TIMER_INFORMATION_CLASS TimerInformationClass,
             _Out_ PVOID TimerInformation,
             _In_ ULONG TimerInformationLength,
             _Out_opt_ PULONG ReturnLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtQueryValueKey(_In_ HANDLE KeyHandle,
                _In_ PUNICODE_STRING ValueName,
                _In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                _Out_writes_bytes_opt_(Length) PVOID KeyValueInformation,
                _In_ ULONG Length,
                _Out_ PULONG ResultLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtRemoveIoCompletion(
    _In_ HANDLE IoCompletionHandle, _Out_ PVOID* KeyContext, _Out_ PVOID* ApcContext, _Out_ PIO_STATUS_BLOCK IoStatus, _In_opt_ PLARGE_INTEGER Timeout)
{
    return 0;
}

NTSTATUS
NTAPI
NtRenameKey(_In_ HANDLE KeyHandle, _In_ PUNICODE_STRING NewName)
{
    return 0;
}

NTSTATUS
NTAPI
NtSetInformationKey(_In_ HANDLE KeyHandle,
                    _In_ _Strict_type_match_ KEY_SET_INFORMATION_CLASS KeySetInformationClass,
                    _In_reads_bytes_(KeySetInformationLength) PVOID KeySetInformation,
                    _In_ ULONG KeySetInformationLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtSetInformationToken(_In_ HANDLE TokenHandle,
                      _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
                      _In_reads_bytes_(TokenInformationLength) PVOID TokenInformation,
                      _In_ ULONG TokenInformationLength)
{
    return 0;
}

NTSTATUS
NTAPI
NtSetIoCompletion(_In_ HANDLE IoCompletionHandle, _In_ PVOID KeyContext, _In_opt_ PVOID ApcContext, _In_ NTSTATUS IoStatus, _In_ ULONG_PTR IoStatusInformation)
{
    return 0;
}

NTSTATUS
NTAPI
NtSetSecurityObject(_In_ HANDLE Handle, _In_ SECURITY_INFORMATION SecurityInformation, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    return 0;
}

NTSTATUS
NTAPI
NtSetValueKey(_In_ HANDLE KeyHandle,
              _In_ PUNICODE_STRING ValueName,
              _In_opt_ ULONG TitleIndex,
              _In_ ULONG Type,
              _In_reads_bytes_(DataSize) PVOID Data,
              _In_ ULONG DataSize)
{
    return 0;
}

NTSTATUS
NTAPI
NtUnmapViewOfSection(_In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress)
{
    return 0;
}

NTSTATUS
NTAPI
NtWaitForSingleObject(_In_ HANDLE Handle, _In_ BOOLEAN Alertable, _In_opt_ PLARGE_INTEGER Timeout)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAddAccessAllowedAce(_Inout_ PACL Acl, _In_ ULONG AceRevision, _In_ ACCESS_MASK AccessMask, _In_ PSID Sid)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAddAccessAllowedAceEx(_Inout_ PACL Acl, _In_ ULONG AceRevision, _In_ ULONG AceFlags, _In_ ACCESS_MASK AccessMask, _In_ PSID Sid)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAddAce(_Inout_ PACL Acl, _In_ ULONG AceRevision, _In_ ULONG StartingAceIndex, _In_reads_bytes_(AceListLength) PVOID AceList, _In_ ULONG AceListLength)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAllocateAndInitializeSid(_In_ PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
                            _In_ UCHAR SubAuthorityCount,
                            _In_ ULONG SubAuthority0,
                            _In_ ULONG SubAuthority1,
                            _In_ ULONG SubAuthority2,
                            _In_ ULONG SubAuthority3,
                            _In_ ULONG SubAuthority4,
                            _In_ ULONG SubAuthority5,
                            _In_ ULONG SubAuthority6,
                            _In_ ULONG SubAuthority7,
                            _Outptr_ PSID* Sid)
{
    return 0;
}

_Ret_maybenull_ _Post_writable_byte_size_(Size)
PVOID
NTAPI
RtlAllocateHeap(_In_ PVOID HeapHandle, _In_ ULONG Flags, _In_ SIZE_T Size)
{
    return NULL;
}

NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(PUNICODE_STRING DestinationString, PCANSI_STRING SourceString, BOOLEAN AllocateDestinationString)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAppendUnicodeStringToString(_In_ PUNICODE_STRING Destination, _In_ PCUNICODE_STRING Source)
{
    return 0;
}

NTSTATUS
NTAPI
RtlAppendUnicodeToString(_In_ PUNICODE_STRING Destination, _In_opt_ PCWSTR Source)
{
    return 0;
}

NTSTATUS
NTAPI
RtlCharToInteger(PCSZ String, ULONG Base, PULONG Value)
{
    return 0;
}

_Must_inspect_result_ LONG NTAPI RtlCompareUnicodeString(_In_ PCUNICODE_STRING String1, _In_ PCUNICODE_STRING String2, _In_ BOOLEAN CaseInSensitive)
{
    return 0;
}

NTSTATUS
NTAPI
RtlConvertSidToUnicodeString(PUNICODE_STRING UnicodeString, PSID Sid, BOOLEAN AllocateDestinationString)
{
    return 0;
}

VOID NTAPI RtlCopyLuid(_Out_ PLUID DestinationLuid, _In_ PLUID SourceLuid)
{
}

NTSTATUS
NTAPI
RtlCopySid(_In_ ULONG DestinationSidLength, _Out_writes_bytes_(DestinationSidLength) PSID DestinationSid, _In_ PSID SourceSid)
{
    return 0;
}

VOID NTAPI RtlCopyUnicodeString(_In_ PUNICODE_STRING DestinationString, _In_ PCUNICODE_STRING SourceString)
{
}

NTSTATUS
NTAPI
RtlCreateAcl(_Out_writes_bytes_(AclLength) PACL Acl, _In_ ULONG AclLength, _In_ ULONG AclRevision)
{
    return 0;
}

NTSTATUS
NTAPI
RtlCreateSecurityDescriptorRelative(_Out_ PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor, _In_ ULONG Revision)
{
    return 0;
}

BOOLEAN
NTAPI
RtlCreateUnicodeString(_Out_ PUNICODE_STRING DestinationString, _In_opt_ PCWSTR SourceString)
{
    return FALSE;
}

NTSTATUS
NTAPI
RtlDeleteAce(_Inout_ PACL Acl, _In_ ULONG AceIndex)
{
    return 0;
}

NTSTATUS
NTAPI
RtlDeleteCriticalSection(_In_ PRTL_CRITICAL_SECTION CriticalSection)
{
    return 0;
}

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(_In_ PCWSTR Path)
{
    return 0;
}

_Success_(return != 0) BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(_In_ PCWSTR DosFileName,
                                                                  _Out_ PUNICODE_STRING NtFileName,
                                                                  _Out_opt_ PWSTR* FilePart,
                                                                  _Out_opt_ PRTL_RELATIVE_NAME RelativeName)
{
    return FALSE;
}

NTSTATUS
NTAPI
RtlDosPathNameToNtPathName_U_WithStatus(_In_ PCWSTR DosFileName,
                                        _Out_ PUNICODE_STRING NtFileName,
                                        _Out_opt_ PWSTR* FilePart,
                                        _Out_opt_ PRTL_RELATIVE_NAME RelativeName)
{
    return 0;
}

_Success_(return != 0) BOOLEAN NTAPI RtlDosPathNameToRelativeNtPathName_U(_In_ PCWSTR DosFileName,
                                                                          _Out_ PUNICODE_STRING NtFileName,
                                                                          _Out_opt_ PWSTR* FilePath,
                                                                          _Out_opt_ PRTL_RELATIVE_NAME RelativeName)
{
    return FALSE;
}

NTSTATUS
NTAPI
RtlDosPathNameToRelativeNtPathName_U_WithStatus(_In_ PCWSTR DosFileName,
                                                _Out_ PUNICODE_STRING NtFileName,
                                                _Out_opt_ PWSTR* FilePath,
                                                _Out_opt_ PRTL_RELATIVE_NAME RelativeName)
{
    return 0;
}

WCHAR
NTAPI
RtlDowncaseUnicodeChar(_In_ WCHAR SourceCharacter)
{
    return 0;
}

NTSTATUS
NTAPI
RtlDowncaseUnicodeString(PUNICODE_STRING DestinationString, _In_ PCUNICODE_STRING SourceString, _In_ BOOLEAN AllocateDestinationString)
{
    return 0;
}

NTSTATUS
NTAPI
RtlEnterCriticalSection(_In_ PRTL_CRITICAL_SECTION CriticalSection)
{
    return 0;
}

BOOLEAN
NTAPI
RtlEqualPrefixSid(_In_ PSID Sid1, _In_ PSID Sid2)
{
    return 0;
}

BOOLEAN
NTAPI
RtlEqualSid(_In_ PSID Sid1, _In_ PSID Sid2)
{
    return 0;
}

_Must_inspect_result_ BOOLEAN NTAPI RtlEqualUnicodeString(_In_ PCUNICODE_STRING String1, _In_ PCUNICODE_STRING String2, _In_ BOOLEAN CaseInSensitive)
{
    return 0;
}

VOID NTAPI RtlFreeAnsiString(PANSI_STRING AnsiString)
{
}

_Success_(return != 0) BOOLEAN NTAPI RtlFreeHeap(_In_ PVOID HeapHandle, _In_opt_ ULONG Flags, _Frees_ptr_opt_ PVOID BaseAddress)
{
    return 0;
}

VOID NTAPI RtlFreeOemString(POEM_STRING OemString)
{
}

PVOID
NTAPI
RtlFreeSid(_In_ _Post_invalid_ PSID Sid)
{
    return NULL;
}

VOID NTAPI RtlFreeUnicodeString(PUNICODE_STRING UnicodeString)
{
}

NTSTATUS /* Returns VOID in pre-Vista */
    NTAPI
    RtlGenerate8dot3Name(_In_ PCUNICODE_STRING Name,
                         _In_ BOOLEAN AllowExtendedCharacters,
                         _Inout_ PGENERATE_NAME_CONTEXT Context,
                         _Inout_ PUNICODE_STRING Name8dot3)
{
    return 0;
}

NTSTATUS
NTAPI
RtlGetAce(_In_ PACL Acl, _In_ ULONG AceIndex, _Outptr_ PVOID* Ace)
{
    return 0;
}

NTSTATUS
NTAPI
RtlGetDaclSecurityDescriptor(_In_ PSECURITY_DESCRIPTOR SecurityDescriptor,
                             _Out_ PBOOLEAN DaclPresent,
                             _Outptr_result_maybenull_ PACL* Dacl,
                             _Pre_ _Writable_elements_(1) _When_(!(*DaclPresent), _Post_invalid_) _When_((*DaclPresent), _Post_valid_) PBOOLEAN DaclDefaulted)
{
    return 0;
}

ULONG
NTAPI
RtlGetFullPathName_U(_In_ PWSTR FileName, _In_ ULONG BufferLength, _Out_writes_bytes_(BufferLength) PWSTR Buffer, _Out_opt_ PWSTR* FilePart)
{
    return 0;
}

NTSTATUS
NTAPI
RtlGetGroupSecurityDescriptor(_In_ PSECURITY_DESCRIPTOR SecurityDescriptor,
                              _Outptr_result_maybenull_ PSID* Group,
                              _Pre_ _Notnull_ _Pre_ _Writable_elements_(1) _When_(*Group == NULL, _Post_invalid_) _When_(*Group != NULL, _Post_valid_)
                                  PBOOLEAN GroupDefaulted)
{
    return 0;
}

NTSTATUS
NTAPI
RtlGetOwnerSecurityDescriptor(_In_ PSECURITY_DESCRIPTOR SecurityDescriptor,
                              _Outptr_result_maybenull_ PSID* Owner,
                              _When_(*Owner == NULL, _Post_invalid_) _When_(*Owner != NULL, _Post_valid_) _Pre_ _Notnull_ _Pre_ _Writable_elements_(1)
                                  PBOOLEAN OwnerDefaulted)
{
    return 0;
}

NTSTATUS
NTAPI
RtlGetVersion(LPOSVERSIONINFOEXW lpVersionInformation)
{
    return 0;
}

PSID_IDENTIFIER_AUTHORITY
NTAPI
RtlIdentifierAuthoritySid(_In_ PSID Sid)
{
    return NULL;
}

PVOID
NTAPI
RtlImageDirectoryEntryToData(_In_ PVOID Base, _In_ BOOLEAN MappedAsImage, _In_ USHORT DirectoryEntry, _Out_ PULONG Size)
{
    return NULL;
}

PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(_In_ PVOID Base)
{
    return NULL;
}

PVOID
NTAPI
RtlImageRvaToVa(_In_ PIMAGE_NT_HEADERS NtHeaders, _In_ PVOID Base, _In_ ULONG Rva, _Inout_opt_ PIMAGE_SECTION_HEADER* LastRvaSection)
{
    return NULL;
}

VOID NTAPI RtlInitAnsiString(PANSI_STRING DestinationString, PCSZ SourceString)
{
}

NTSTATUS
NTAPI
RtlInitializeCriticalSection(_In_ PRTL_CRITICAL_SECTION CriticalSection)
{
    return 0;
}

NTSTATUS
NTAPI
RtlInitializeSid(_Out_ PSID Sid, _In_ PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, _In_ UCHAR SubAuthorityCount)
{
    return 0;
}

VOID NTAPI RtlInitString(PSTRING DestinationString, PCSZ SourceString)
{
}

VOID NTAPI RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString)
{
}

BOOLEAN
NTAPI
RtlIsNameLegalDOS8Dot3(_In_ PUNICODE_STRING Name, _Inout_opt_ POEM_STRING OemName, _Inout_opt_ PBOOLEAN NameContainsSpaces)
{
    return 0;
}

NTSTATUS
NTAPI
RtlLeaveCriticalSection(_In_ PRTL_CRITICAL_SECTION CriticalSection)
{
    return 0;
}

ULONG
NTAPI
RtlLengthRequiredSid(_In_ ULONG SubAuthorityCount)
{
    return 0;
}

ULONG
NTAPI
RtlLengthSid(_In_ PSID Sid)
{
    return 0;
}

NTSTATUS
NTAPI
RtlLocalTimeToSystemTime(_In_ PLARGE_INTEGER LocalTime, _Out_ PLARGE_INTEGER SystemTime)
{
    return 0;
}

ULONG
NTAPI
RtlNtStatusToDosError(NTSTATUS Status)
{
    return 0;
}

_Must_inspect_result_ BOOLEAN NTAPI RtlPrefixUnicodeString(_In_ PCUNICODE_STRING String1, _In_ PCUNICODE_STRING String2, _In_ BOOLEAN CaseInSensitive)
{
    return 0;
}

NTSTATUS
NTAPI
RtlSetGroupSecurityDescriptor(_Inout_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID Group, _In_ BOOLEAN GroupDefaulted)
{
    return 0;
}

NTSTATUS
NTAPI
RtlSetOwnerSecurityDescriptor(_Inout_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID Owner, _In_ BOOLEAN OwnerDefaulted)
{
    return 0;
}

PUCHAR
NTAPI
RtlSubAuthorityCountSid(_In_ PSID Sid)
{
    return 0;
}

PULONG
NTAPI
RtlSubAuthoritySid(_In_ PSID Sid, _In_ ULONG SubAuthority)
{
    return 0;
}

BOOLEAN
NTAPI
RtlTimeToSecondsSince1970(PLARGE_INTEGER Time, PULONG ElapsedSeconds)
{
    return FALSE;
}

NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(PANSI_STRING DestinationString, PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString)
{
    return 0;
}

NTSTATUS
NTAPI
RtlUnicodeStringToOemString(POEM_STRING DestinationString, PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString)
{
    return 0;
}

NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(_Out_ PULONG BytesInMultiByteString, _In_reads_bytes_(BytesInUnicodeString) PWCH UnicodeString, _In_ ULONG BytesInUnicodeString)
{
    return 0;
}

ULONG
NTAPI
RtlUniform(PULONG Seed)
{
    return 0;
}

WCHAR
NTAPI
RtlUpcaseUnicodeChar(_In_ WCHAR SourceCharacter)
{
    return 0;
}

NTSTATUS
NTAPI
RtlUpcaseUnicodeString(_Inout_ PUNICODE_STRING DestinationString, _In_ PCUNICODE_STRING SourceString, _In_ BOOLEAN AllocateDestinationString)
{
    return 0;
}

NTSTATUS
NTAPI
RtlValidateUnicodeString(_In_ _Reserved_ ULONG Flags, _In_ PCUNICODE_STRING String)
{
    return 0;
}

BOOLEAN
NTAPI
RtlValidSid(_In_ PSID Sid)
{
    return FALSE;
}
