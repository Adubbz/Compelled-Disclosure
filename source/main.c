#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <switch.h>

#include "save.h"

void loopInput()
{
    printf("\nPress B to exit.\n");

    while (appletMainLoop())
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_B) 
           break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
}

#define NS_TITLE_ID 0x010000000000001F
#define VMDB_SAVE_ID 0x8000000000000045

int main(int argc, char **argv)
{
    Result rc;

    gfxInitDefault();
    consoleInit(NULL);

    if (R_FAILED(rc = pmshellInitialize()))
    {
        printf("Failed to initialize pm:shell. Error code: 0x%08x\n", rc);
        goto loop_input;
    }

    printf("Backing up vmdb save data...\n");
    backupSystemSavedata(NS_TITLE_ID, VMDB_SAVE_ID);
    fsdevUnmountDevice("save");

    loop_input:
    loopInput();

    pmshellExit();
    gfxExit();
    return 0;
}

