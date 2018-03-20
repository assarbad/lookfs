///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
/// Purpose          : Functions to deal with NT privileges.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __PRIV_H_VER__
#define __PRIV_H_VER__ 2018031723
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif /* Check for "#pragma once" support */

#include <Windows.h>
#include <winnt.h>
#include <tchar.h>

EXTERN_C BOOL SetTokenPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
EXTERN_C BOOL SetContextPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
EXTERN_C BOOL SetProcessPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
EXTERN_C BOOL SetThreadPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
EXTERN_C BOOL HasTokenPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes);
EXTERN_C BOOL HasContextTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes);
EXTERN_C BOOL HasProcessTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes);
EXTERN_C BOOL HasThreadTokenPrivilege(LPCTSTR lpszPrivilege, LPDWORD lpdwAttributes);
EXTERN_C BOOL IsContextTokenPrivilegeEnabled(LPCTSTR lpszPrivilege);
EXTERN_C BOOL IsProcessTokenPrivilegeEnabled(LPCTSTR lpszPrivilege);
EXTERN_C BOOL IsThreadTokenPrivilegeEnabled(LPCTSTR lpszPrivilege);

#ifdef __cplusplus
class CSnapEnableAssignedPrivilege
{
    BOOL m_bEnabled, m_bNoErrors;
    LPTSTR m_lpszPrivilege;
public:
    CSnapEnableAssignedPrivilege(LPCTSTR lpszPrivilege, BOOL bNoErrors = TRUE)
        : m_bEnabled(FALSE)
        , m_bNoErrors(bNoErrors)
        , m_lpszPrivilege((lpszPrivilege) ? _tcsdup(lpszPrivilege) : NULL)
    {
        if (HasContextTokenPrivilege(m_lpszPrivilege, NULL))
        {
            m_bEnabled = SetContextPrivilege(m_lpszPrivilege, TRUE);
            if (!m_bEnabled)
            {
                if (!m_bNoErrors)
                {
                    _tprintf(_T("Could not enable %s\n"), lpszPrivilege);
                }
            }
        }
    }

    virtual ~CSnapEnableAssignedPrivilege()
    {
        if (m_bEnabled)
        {
            (void)SetContextPrivilege(m_lpszPrivilege, FALSE);
        }
        free(m_lpszPrivilege);
    }
private:
    CSnapEnableAssignedPrivilege(CSnapEnableAssignedPrivilege&);
    CSnapEnableAssignedPrivilege& operator=(CSnapEnableAssignedPrivilege&);
};
#endif

#endif /* __PRIV_H_VER__ */
