///////////////////////////////////////////////////////////////////////////////
///
/// Written by Oliver Schneider (assarbad.net) - PUBLIC DOMAIN/CC0
///
///////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <tchar.h>
#include "ntfindfile.h"
#include "priv.h"

int _tmain(int argc, _TCHAR** argv)
{
    BOOL bBackupPrivEnabled = FALSE;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    __try
    {
        if (PrivHasContextTokenPrivilege(SE_BACKUP_NAME, NULL))
        {
            bBackupPrivEnabled = PrivSetContextPrivilege(SE_BACKUP_NAME, TRUE);
        }
    }
    __finally
    {
        if (bBackupPrivEnabled)
        {
            (void)PrivSetContextPrivilege(SE_BACKUP_NAME, FALSE);
        }
    }
    return 0;
}
