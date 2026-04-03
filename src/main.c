#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "hash.h"
#include "scan.h"
#include "baseline.h"
#include "compare.h"
#include "report.h"

#define BASELINE_DB "data/baseline.db"

/*
 * Prints usage instructions to stderr and exits with a failure code.
 * Called whenever the user passes bad arguments.
 */
static void usage(const char *prog) {
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "  %s --baseline <directory>   Create a new snapshot\n", prog);
    fprintf(stderr, "  %s --check    <directory>   Compare against snapshot\n", prog);
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s --baseline /etc\n", prog);
    fprintf(stderr, "  %s --check    /etc\n", prog);
    fprintf(stderr, "\n");
}

/*
 * Baseline mode.
 *
 * 1. Scan the target directory and collect all file paths.
 * 2. Hash each file.
 * 3. Build a Baseline struct from the paths and hashes.
 * 4. Save it to BASELINE_DB.
 * 5. Print a confirmation via report.
 *
 * Returns 0 on success, 1 on any failure.
 */
static int run_baseline(const char *dirpath) {
    FileList list;
    Baseline db;
    char     hash[MAX_HASH_LEN];
    int      ret = 0;

    memset(&list, 0, sizeof(FileList));
    memset(&db,   0, sizeof(Baseline));

    printf("Scanning %s ...\n", dirpath);

    if (scan_directory(dirpath, &list) != 0) {
        fprintf(stderr, "main: scan failed on %s\n", dirpath);
        return 1;
    }

    printf("Hashing %d files ...\n", list.count);

    for (int i = 0; i < list.count; i++) {
        ret = hash_file(list.paths[i], hash);
        if (ret != 0) {
            fprintf(stderr, "main: could not hash %s, skipping\n",
                    list.paths[i]);
            continue;
        }

        if (db.count >= MAX_FILES) {
            fprintf(stderr, "main: MAX_FILES limit hit during baseline\n");
            return 1;
        }

        safe_strncpy(db.records[db.count].path, list.paths[i], MAX_PATH_LEN);
        safe_strncpy(db.records[db.count].hash, hash,           MAX_HASH_LEN);
        db.count++;
    }

    if (baseline_save(&db, BASELINE_DB) != 0) {
        fprintf(stderr, "main: failed to save baseline to %s\n", BASELINE_DB);
        return 1;
    }

    report_baseline_created(dirpath, db.count);
    return 0;
}

/*
 * Check mode.
 *
 * 1. Load the saved baseline from BASELINE_DB.
 * 2. Scan the target directory fresh.
 * 3. Compare the current state against the baseline.
 * 4. Report the results.
 *
 * Returns 0 if the check completed (even if changes were found).
 * Returns 1 on a hard failure that prevented the check from running.
 *
 * Note: finding changes is NOT a failure condition. The tool did its
 * job. Return 0 and let the report speak for itself.
 */
static int run_check(const char *dirpath) {
    Baseline   db;
    FileList   current;
    ChangeList changes;

    memset(&db,      0, sizeof(Baseline));
    memset(&current, 0, sizeof(FileList));
    memset(&changes, 0, sizeof(ChangeList));

    if (baseline_load(&db, BASELINE_DB) != 0) {
        fprintf(stderr, "main: could not load baseline from %s\n", BASELINE_DB);
        fprintf(stderr, "      run --baseline first to create a snapshot.\n");
        return 1;
    }

    printf("Baseline loaded: %d records\n", db.count);
    printf("Scanning %s ...\n", dirpath);

    if (scan_directory(dirpath, &current) != 0) {
        fprintf(stderr, "main: scan failed on %s\n", dirpath);
        return 1;
    }

    printf("Comparing %d current files against baseline ...\n", current.count);

    if (compare_baseline(&db, &current, dirpath, &changes) != 0) {
        fprintf(stderr, "main: comparison failed\n");
        return 1;
    }

    report_results(&changes, dirpath);
    return 0;
}

/*
 * Entry point.
 *
 * Expects exactly two arguments: a mode flag and a directory path.
 * Dispatches to run_baseline() or run_check() accordingly.
 *
 * argc is the argument count including the program name itself.
 * argv[0] is always the program name.
 * argv[1] is the mode flag.
 * argv[2] is the target directory.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    const char *mode    = argv[1];
    const char *dirpath = argv[2];

    if (strcmp(mode, "--baseline") == 0) {
        return run_baseline(dirpath);
    } else if (strcmp(mode, "--check") == 0) {
        return run_check(dirpath);
    } else {
        fprintf(stderr, "main: unknown mode '%s'\n", mode);
        usage(argv[0]);
        return 1;
    }
}
