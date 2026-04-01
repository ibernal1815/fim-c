#ifndef BASELINE_H
#define BASELINE_H

#include "utils.h"

typedef struct {
    char path[MAX_PATH_LEN];
    char hash[MAX_HASH_LEN];
} FileRecord;

typedef struct {
    FileRecord records[MAX_FILES];
    int        count;
} Baseline;

int baseline_save(const Baseline *db, const char *filepath);
int baseline_load(Baseline *db, const char *filepath);

#endif
