#ifndef REPORT_H
#define REPORT_H

#include "compare.h"

#define LOG_PATH "data/fim.log"

void report_results(const ChangeList *changes, const char *target_dir);
void report_baseline_created(const char *target_dir, int file_count);

#endif
