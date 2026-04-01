#include <stdio.h>
#include <string.h>
#include "compare.h"
#include "hash.h"
#include "utils.h"

/*
 * Finds a record in the baseline by exact path match.
 * Returns a pointer to the matching FileRecord, or NULL if not found.
 *
 * This is a linear search -- O(n). Fine for thousands of files.
 * If you ever needed to handle millions of files you would want a
 * hash table or sorted array with binary search instead.
 */
static const FileRecord *find_in_baseline(const Baseline *db,
                                           const char *path) {
    for (int i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].path, path) == 0) {
            return &db->records[i];
        }
    }
    return NULL;
}

/*
 * Checks whether a path from the baseline still exists in the current
 * scanned file list.
 * Returns 1 if found, 0 if not.
 */
static int exists_in_filelist(const FileList *current, const char *path) {
    for (int i = 0; i < current->count; i++) {
        if (strcmp(current->paths[i], path) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * Appends a Change to the ChangeList.
 * Returns  0 on success.
 * Returns -1 if the list is full.
 */
static int add_change(ChangeList *out, const char *path,
                      const char *old_hash, const char *new_hash,
                      ChangeType type) {
    if (out->count >= MAX_FILES) {
        fprintf(stderr, "compare: ChangeList full, cannot add more changes\n");
        return -1;
    }

    Change *c = &out->changes[out->count];
    safe_strncpy(c->path, path, MAX_PATH_LEN);

    if (old_hash != NULL) {
        safe_strncpy(c->old_hash, old_hash, MAX_HASH_LEN);
    } else {
        c->old_hash[0] = '\0';
    }

    if (new_hash != NULL) {
        safe_strncpy(c->new_hash, new_hash, MAX_HASH_LEN);
    } else {
        c->new_hash[0] = '\0';
    }

    c->type = type;
    out->count++;
    return 0;
}

/*
 * Core comparison function. Two passes:
 *
 * Pass 1 -- walk every file in the current scan.
 *   If it exists in the baseline, hash it fresh and compare.
 *     Match:    nothing to report.
 *     Mismatch: record as CHANGE_MODIFIED.
 *   If it does NOT exist in the baseline, record as CHANGE_ADDED.
 *
 * Pass 2 -- walk every record in the baseline.
 *   If the path no longer exists in the current scan,
 *   record it as CHANGE_MISSING.
 *
 * dirpath is unused in the current implementation but kept in the
 * signature for future use (e.g. scoping the search, config filtering).
 *
 * Returns  0 on success.
 * Returns -1 if a hash operation fails badly enough to abort.
 */
int compare_baseline(const Baseline *db, const FileList *current,
                     const char *dirpath, ChangeList *out) {
    char              current_hash[MAX_HASH_LEN];
    const FileRecord *record = NULL;
    int               ret    = 0;

    (void)dirpath;

    memset(out, 0, sizeof(ChangeList));

    /* Pass 1: check every file that exists right now */
    for (int i = 0; i < current->count; i++) {
        const char *path = current->paths[i];

        ret = hash_file(path, current_hash);
        if (ret != 0) {
            /* Could not hash this file -- permissions issue most likely.
             * Print a warning and keep going rather than aborting the
             * whole comparison. */
            fprintf(stderr, "compare: could not hash %s, skipping\n", path);
            continue;
        }

        record = find_in_baseline(db, path);

        if (record == NULL) {
            /* File exists now but was not in the baseline -- it was added */
            if (add_change(out, path, NULL, current_hash, CHANGE_ADDED) != 0) {
                return -1;
            }
        } else if (strcmp(record->hash, current_hash) != 0) {
            /* File was in the baseline but the hash changed -- it was modified */
            if (add_change(out, path, record->hash, current_hash,
                           CHANGE_MODIFIED) != 0) {
                return -1;
            }
        }
        /* If hashes match, the file is clean -- nothing to record */
    }

    /* Pass 2: check for files that were in the baseline but are now gone */
    for (int i = 0; i < db->count; i++) {
        const char *path = db->records[i].path;

        if (!exists_in_filelist(current, path)) {
            if (add_change(out, path, db->records[i].hash, NULL,
                           CHANGE_MISSING) != 0) {
                return -1;
            }
        }
    }

    return 0;
}
