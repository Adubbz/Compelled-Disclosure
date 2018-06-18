#include "io.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>

#define BUF_SIZE 0x50000

Result copyFile(const char *srcPath, const char *dstPath)
{
    FILE *src;
    FILE *dst;

    if (!(src = fopen(srcPath, "rb")))
    {
        printf("Failed to open file %s\n", srcPath);
        return 1;
    }

    if (!(dst = fopen(dstPath, "wb")))
    {
        printf("Failed to open file %s\n", dstPath);
        fclose(src);
        return 1;
    }

    size_t size;
    u8 *buf = malloc(sizeof(u8) * BUF_SIZE);
    u64 offset = 0;
    
    while ((size = fread(buf, 1, BUF_SIZE, src)) > 0)
    {
        fwrite(buf, 1, size, dst);
        offset += size;
    }

    free(buf);
    fclose(src);
    fclose(dst);

    // Check if the dest path starts with save:/
    if (strncmp(dstPath, "save:/", strlen("save:/")) == 0)
    {
        fsdevCommitDevice("save");
    }

    return 0;
}

Result copyDir(const char *pathIn, const char *pathOut)
{
    Result rc = 0;
    DirEntry *dirEntries;
    // NOTE: We can't have both a dir handle open and a file handle, so we must
    // do this beforehand
    int entryCount = listDir(pathIn, &dirEntries);

    char entryPathIn[PATH_MAX];
    char entryPathOut[PATH_MAX];

    for (int i = 0; i < entryCount; i++)
    {
        DirEntry entry = dirEntries[i];

        memset(entryPathIn, 0, PATH_MAX);
        memset(entryPathOut, 0, PATH_MAX);
        snprintf(entryPathIn, PATH_MAX, "%s%s", pathIn, entry.name);
        snprintf(entryPathOut, PATH_MAX, "%s%s", pathOut, entry.name);

        if (entry.isDir)
        {
            strncat(entryPathIn, "/", 2);
            strncat(entryPathOut, "/", 2);

            if (R_FAILED(rc = createDir(entryPathOut)))
            {
                printf("Failed to create dir %s\n", entryPathOut);
                break;
            }

            if (R_FAILED(rc = copyDir(entryPathIn, entryPathOut)))
            {
                printf("Failed to copy dir %s to %s\n", entryPathIn, entryPathOut);
                break;
            }
        }
        else
        {
            if (R_FAILED(rc = copyFile(entryPathIn, entryPathOut)))
            {
                printf("Failed to copy file %s\n", entryPathIn);
                break;
            }
        }
    }

    free(dirEntries);

    return rc;
}

Result removeDir(const char *path)
{
    Result rc = 0;
    DIR *dir = opendir(path);

    // If dir already doesn't exist, no action needed
    if (dir == NULL)
        return rc;

    closedir(dir);
    DirEntry *dirEntries;
    // NOTE: We can't have both a dir handle open and a file handle, so we must
    // do this beforehand
    int entryCount = listDir(path, &dirEntries);
    char entryPath[PATH_MAX];

    for (int i = 0; i < entryCount; i++)
    {
        DirEntry entry = dirEntries[i];

        memset(entryPath, 0, PATH_MAX);
        snprintf(entryPath, PATH_MAX, "%s%s", path, entry.name);

        if (entry.isDir)
        {
            strncat(entryPath, "/", 2);

            if (R_FAILED(rc = removeDir(entryPath)))
            {
                printf("Failed to remove dir %s\n", entryPath);
                break;
            }
        }
        else
        {
            if (R_FAILED(rc = remove(entryPath)))
            {
                printf("Failed to remove file %s\n", entryPath);
                break;
            }
        }
    }

    free(dirEntries);
    if (R_FAILED(rc = rmdir(path)))
        printf("Failed to remove dir %s\n", path);

    return rc;
}

size_t listDir(const char *path, DirEntry **dirEntriesOut)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
        return 0;

    struct dirent *entry;
    int entryCount = 0;

    while ((entry = readdir(dir)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        entryCount++;
    }

    // rewinddir appears to be broken
    closedir(dir);
    dir = opendir(path);

    if (dir == NULL)
        return 0;

    DirEntry *dir_entries = malloc(sizeof(DirEntry) * entryCount);
    u32 offset = 0;

    while ((entry = readdir(dir)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        DirEntry *dir_ent = dir_entries + offset;

        memset(dir_ent->name, 0, 256);
        strncpy(dir_ent->name, entry->d_name, 256);

        dir_ent->isDir = entry->d_type == DT_DIR;
        offset++;
    }

    closedir(dir);
    *dirEntriesOut = dir_entries;

    return entryCount;
}

Result createDir(char* path) 
{
    DIR *dir = opendir(path);

    if (dir)
    {
        closedir(dir);
        return 0;
    }

    if (mkdir(path, 777) != 0 && errno != EEXIST)
        return -1; 

    return 0;
}