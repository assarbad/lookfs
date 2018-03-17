///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Purpose          : Functions to deal with NT privileges.
///
///////////////////////////////////////////////////////////////////////////////
#include "priv.h"

// A struct that should fit all possible privileges ... twice over
typedef struct _ALL_TOKEN_PRIVILEGES
{
    DWORD PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[2*MaxTokenInfoClass + 1];
} ALL_TOKEN_PRIVILEGES;

static HANDLE GetProcessToken_(DWORD dwAdditionalAccess)
{
    HANDLE hToken = NULL;

    if (!OpenProcessToken(GetCurrentProcess(), dwAdditionalAccess | TOKEN_QUERY, &hToken))
    {
        return NULL;
    }
    return hToken;
}

static HANDLE GetThreadToken_(DWORD dwAdditionalAccess)
{
    HANDLE hToken = NULL;

    if (!OpenThreadToken(GetCurrentThread(), dwAdditionalAccess | TOKEN_QUERY, TRUE, &hToken))
    {
        return NULL;
    }
    return hToken;
}

BOOL SetTokenPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
    {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tp.Privileges[0].Attributes = 0;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
    {
        return FALSE;
    }
    return TRUE;
}

BOOL SetContextPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    HANDLE hToken = GetThreadToken_(TOKEN_ADJUST_PRIVILEGES);

    if (!hToken)
    {
        hToken = GetProcessToken_(TOKEN_ADJUST_PRIVILEGES);
        if (!hToken)
        {
            return FALSE;
        }
    }

    if (!SetTokenPrivilege(hToken, lpszPrivilege, bEnablePrivilege))
    {
        (void)CloseHandle(hToken);
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL SetProcessPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    HANDLE hToken = GetProcessToken_(TOKEN_ADJUST_PRIVILEGES);

    if (!hToken)
    {
        return FALSE;
    }

    if (!SetTokenPrivilege(hToken, lpszPrivilege, bEnablePrivilege))
    {
        (void)CloseHandle(hToken);
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL SetThreadPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    HANDLE hToken = GetThreadToken_(TOKEN_ADJUST_PRIVILEGES);

    if (!hToken)
    {
        return FALSE;
    }

    if (!SetTokenPrivilege(hToken, lpszPrivilege, bEnablePrivilege))
    {
        (void)CloseHandle(hToken);
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL HasTokenPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes)
{
    struct {
        union
        {
            TOKEN_PRIVILEGES tp;
            ALL_TOKEN_PRIVILEGES atp;
#pragma warning(suppress : 4201)
        };
    } tp;
    DWORD dwLength = 0;

    RtlZeroMemory(&tp, sizeof(tp));
    SetLastError(ERROR_INVALID_DATA);

    if (GetTokenInformation(hToken, TokenPrivileges, (LPVOID)&tp, sizeof(tp), &dwLength))
    {
        DWORD i, dwPrivNameLength = MAX_PATH;
        LPTSTR lpszPrivName = calloc(dwPrivNameLength, sizeof(TCHAR));

        if (lpszPrivName)
        {
            for (i = 0; i < tp.atp.PrivilegeCount; i++)
            {
                dwLength = dwPrivNameLength;
                if (!LookupPrivilegeName(NULL, &tp.atp.Privileges[i].Luid, lpszPrivName, &dwLength))
                {
                    if (dwLength > dwPrivNameLength)
                    {
                        free(lpszPrivName);
                        dwPrivNameLength = dwLength + 1;
                        lpszPrivName = calloc(dwPrivNameLength, sizeof(TCHAR));
                        if (!lpszPrivName)
                        {
                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                            return FALSE;
                        }
                    }
                    else
                    {
                        free(lpszPrivName);
                        // Last error should be set already
                        return FALSE;
                    }
                }
                dwLength = dwPrivNameLength; // no error, this value will have been changed by the previous call
                if (!LookupPrivilegeName(NULL, &tp.atp.Privileges[i].Luid, lpszPrivName, &dwLength))
                {
                    free(lpszPrivName);
                    // Last error should be set already
                    return FALSE;
                }
                if (0 == _tcscmp(lpszPrivilege, lpszPrivName))
                {
                    free(lpszPrivName);
                    if (lpdwAttributes)
                    {
                        *lpdwAttributes = tp.atp.Privileges[i].Attributes;
                    }
                    SetLastError(ERROR_SUCCESS);
                    return TRUE;
                }
            }
            free(lpszPrivName);
            lpszPrivName = NULL;
            SetLastError(ERROR_NO_MATCH);
            return FALSE;
        }
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return FALSE;
}

BOOL HasContextTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes)
{
    HANDLE hToken = GetThreadToken_(0);

    if (!hToken)
    {
        hToken = GetProcessToken_(0);
        if (!hToken)
        {
            return FALSE;
        }
    }

    if (!HasTokenPrivilege(hToken, lpszPrivilege, lpdwAttributes))
    {
        DWORD dwError = GetLastError(); // save
        (void)CloseHandle(hToken);
        SetLastError(dwError); // restore
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL IsContextTokenPrivilegeEnabled(LPCTSTR lpszPrivilege)
{
    DWORD dwAttributes = 0;
    if (HasContextTokenPrivilege(lpszPrivilege, &dwAttributes))
    {
        if (SE_PRIVILEGE_REMOVED & dwAttributes)
        {
            return FALSE;
        }
        if ((SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT) & dwAttributes)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL HasProcessTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes)
{
    HANDLE hToken = GetProcessToken_(0);

    if (!hToken)
    {
        return FALSE;
    }

    if (!HasTokenPrivilege(hToken, lpszPrivilege, lpdwAttributes))
    {
        DWORD dwError = GetLastError(); // save
        (void)CloseHandle(hToken);
        SetLastError(dwError); // restore
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL IsProcessTokenPrivilegeEnabled(LPCTSTR lpszPrivilege)
{
    DWORD dwAttributes = 0;
    if (HasProcessTokenPrivilege(lpszPrivilege, &dwAttributes))
    {
        if (SE_PRIVILEGE_REMOVED & dwAttributes)
        {
            return FALSE;
        }
        if ((SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT) & dwAttributes)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL HasThreadTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes)
{
    HANDLE hToken = GetThreadToken_(0);

    if (!hToken)
    {
        return FALSE;
    }

    if (!HasTokenPrivilege(hToken, lpszPrivilege, lpdwAttributes))
    {
        DWORD dwError = GetLastError(); // save
        (void)CloseHandle(hToken);
        SetLastError(dwError); // restore
        return FALSE;
    }

    (void)CloseHandle(hToken);
    return TRUE;
}

BOOL IsThreadTokenPrivilegeEnabled(LPCTSTR lpszPrivilege)
{
    DWORD dwAttributes = 0;
    if (HasThreadTokenPrivilege(lpszPrivilege, &dwAttributes))
    {
        if (SE_PRIVILEGE_REMOVED & dwAttributes)
        {
            return FALSE;
        }
        if ((SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT) & dwAttributes)
        {
            return TRUE;
        }
    }
    return FALSE;
}
