/**
 * @file
 * AirBitz file-sync functions.
 */

#include <git2.h>
#include "ABC_Sync.h"

/**
 * Initializes the underlying git library. Should be called a program start.
 */
tABC_CC ABC_SyncInit(tABC_Error *pError)
{
    return ABC_CC_Ok;
}

/**
 * Shuts down the underlying git library. Should be called when the program
 * exits.
 */
void ABC_SyncTerminate()
{
}

/**
 * Prepares a directory for syncing. This must be called one time after
 * the directory has first been created.
 */
tABC_CC ABC_SyncMakeRepo(const char *szRepoPath,
                         tABC_Error *pError)
{
    return ABC_CC_Ok;
}

/**
 * Synchronizes the directory with the server. New files in the folder will
 * go up to the server, and new files on the server will come down to the
 * directory. If there is a conflict, the newer file will win.
 */
tABC_CC ABC_SyncRepo(const char *szRepoPath,
                     const char *szRepoKey,
                     const char *szServer,
                     tABC_Error *pError)
{
    return ABC_CC_Ok;
}