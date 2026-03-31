#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "scan.h"
#include "utils.h"

/*
 * Forward declaration so scan_directory() can call itself recursively
 * when it finds a subdirectory.
 */
static int scan_recursive(const char *dirpath, FileList *list);

/*
 * Public entry point. Zeroes out the list, then kicks off the
 * recursive walk from the given root directory.
 *
 * Returns  0 on success.
 * Returns -1 if the root directory could not be opened.
 */
int scan_directory(const char *dirpath, FileList *list) {
    memset(list, 0, sizeof(FileList));
    return scan_recursive(dirpath, list);
}

/*
 * Recursively walks dirpath. For every regular file found, the full
 * path gets appended to list->paths and list->count incremented.
 * Subdirectories get walked by calling this function again on them.
 *
 * "." and ".." are always skipped -- without this check you would
 * recurse infinitely because every directory contains both as entries.
 *
 * Symlinks are intentionally skipped. Following symlinks during a
 * recursive walk can lead to infinite loops (a symlink pointing to a
 * parent directory). Keeping it simple for now.
 */
static int scan_recursive(const char *dirpath, FileList *list) {
    DIR           *dp   = NULL;
    struct dirent *entry = NULL;
    struct stat    info;
    char           fullpath[MAX_PATH_LEN];

    dp = opendir(dirpath);
    if (dp == NULL) {
        fprintf(stderr, "scan: could not open directory: %s\n", dirpath);
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {

        /* Skip the current directory entry and parent directory entry */
        if (strcmp(entry->d_name, ".")  == 0) continue;
        if (strcmp(entry->d_name, "..") == 0) continue;

        /* Build the full path for this entry */
        join_path(fullpath, sizeof(fullpath), dirpath, entry->d_name);

        /* stat() queries the filesystem for metadata about the path.
         * We use it to determine if this entry is a regular file or
         * a directory. lstat() would give us info about the symlink
         * itself rather than what it points to -- we use that to
         * detect and skip symlinks safely. */
        if (lstat(fullpath, &info) != 0) {
            fprintf(stderr, "scan: could not stat: %s\n", fullpath);
            continue;
        }

        if (S_ISREG(info.st_mode)) {
            /* Regular file -- add it to the list */
            if (list->count >= MAX_FILES) {
                fprintf(stderr, "scan: MAX_FILES limit reached (%d)\n", MAX_FILES);
                closedir(dp);
                return -1;
            }
            safe_strncpy(list->paths[list->count], fullpath, MAX_PATH_LEN);
            list->count++;

        } else if (S_ISDIR(info.st_mode)) {
            /* Directory -- recurse into it */
            if (scan_recursive(fullpath, list) != 0) {
                closedir(dp);
                return -1;
            }
        }
        /* anything else (symlink, socket, device file) is silently skipped */
    }

    closedir(dp);
    return 0;
}
