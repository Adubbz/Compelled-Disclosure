#include "save.h"

#include <stdio.h>
#include "io.h"

static FsFileSystem g_activeFs;

Result openSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;
    
    for (int attempt = 0; attempt < 100; attempt++)
    {
        pmshellTerminateProcessByTitleId(titleId);

        if (R_SUCCEEDED(rc = fsMount_SystemSaveData(&g_activeFs, saveId)))
        {
            break;
        }
    }

    // Took too many attempts
    if (R_FAILED(rc))
    {
        printf("Failed to mount system save data %016lx for tid %016lx. Error code: 0x%08x", saveId, titleId, rc);
        return rc;
    }

    if (fsdevMountDevice("save", g_activeFs) == -1) 
    {
        printf("Failed to mount system save data device.\n");
        return -1;
    }

    return rc;
}

Result backupSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;

    if (R_FAILED(rc = openSystemSavedata(titleId, saveId)))
    {
        printf("Failed to open save data. Error code: 0x%08x\n", rc);
        return rc;
    }

    // Setup folders to copy save data into
    createDir("/switch/");

    if (R_FAILED(rc = createDir("/switch/compelled_disclosure/")))
    {
        printf("Failed to create compelled disclosure folder\n");
        return rc;
    }

    char outPath[FS_MAX_PATH];
    snprintf(outPath, FS_MAX_PATH, "/switch/compelled_disclosure/%016lx/", titleId);

    if (R_FAILED(rc = removeDir(outPath)))
    {
        printf("Failed to remove compelled disclosure folder\n");
        return rc;
    }

    if (R_FAILED(rc = createDir(outPath)))
    {
        printf("Failed to create out dir path %s\n", outPath);
        return rc;
    }

    // Begin copying files
    if (R_FAILED(rc = copyDir("save:/", outPath)))
    {
        printf("Failed to copy save:/ to out path %s\n", outPath);
        return rc;
    }

    return rc;
}

Result restoreSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;

    if (R_FAILED(rc = openSystemSavedata(titleId, saveId)))
    {
        printf("Failed to open save data. Error code: 0x%08x\n", rc);
        return rc;
    }

    char outPath[FS_MAX_PATH];
    snprintf(outPath, FS_MAX_PATH, "/switch/compelled_disclosure/%016lx/", titleId);

    if (R_FAILED(rc = removeDir("save:/")))
    {
        printf("Failed to remove save folder\n");
        return rc;
    }

    // Begin copying files
    if (R_FAILED(rc = copyDir(outPath, "save:/")))
    {
        printf("Failed to copy save:/ to out path %s\n", outPath);
        return rc;
    }

    return rc;
}