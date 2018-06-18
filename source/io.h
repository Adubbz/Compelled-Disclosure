#pragma once

#include <switch.h>
#include <limits.h>

typedef struct
{
    char name[256];
    u8 isDir;
} DirEntry;

Result copyFile(const char *srcPath, const char *dstPath);
Result copyDir(const char *pathIn, const char *pathOut);
size_t listDir(const char *path, DirEntry **dirEntriesOut);
Result createDir(char* path);
Result removeDir(const char *path);