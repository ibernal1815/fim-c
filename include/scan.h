#ifndef SCAN_H
#define SCAN_H

#include "utils.h"

typedef struct {
    char paths[MAX_FILES][MAX_PATH_LEN];
    int  count;
} FileList;

int scan_directory(const char *dirpath, FileList *list);

#endif
