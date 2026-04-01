#ifndef COMPARE_H
#define COMPARE_H

#include "baseline.h"
#include "scan.h"

typedef enum {
    CHANGE_MODIFIED,
    CHANGE_ADDED,
    CHANGE_MISSING
} ChangeType;

typedef struct {
    char       path[MAX_PATH_LEN];
    char       old_hash[MAX_HASH_LEN];
    char       new_hash[MAX_HASH_LEN];
    ChangeType type;
} Change;

typedef struct {
    Change changes[MAX_FILES];
    int    count;
} ChangeList;

int compare_baseline(const Baseline *db, const FileList *current,
                     const char *dirpath, ChangeList *out);

#endif
