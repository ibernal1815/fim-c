#include <stdio.h>
#include <string.h>
#include "baseline.h"
#include "utils.h"

/*
 * Writes a Baseline to a plain text file at filepath.
 * Each line is formatted as:   path:hash\n
 *
 * The file is created if it does not exist and overwritten if it does.
 * Returns  0 on success.
 * Returns -1 if the file could not be opened or a write error occurred.
 */
int baseline_save(const Baseline *db, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        fprintf(stderr, "baseline_save: could not open %s for writing\n", filepath);
        return -1;
    }

    for (int i = 0; i < db->count; i++) {
        if (fprintf(fp, "%s:%s\n", db->records[i].path, db->records[i].hash) < 0) {
            fprintf(stderr, "baseline_save: write error at record %d\n", i);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

/*
 * Reads a baseline file back into a Baseline struct.
 * Parses each line expecting the format:   path:hash
 *
 * The tricky part is the separator. A file path can contain colons on
 * some systems, so we search for the LAST colon in the line rather than
 * the first. The hash is always the final 64 characters before the newline,
 * so the last colon is always the real delimiter.
 *
 * Returns  0 on success.
 * Returns -1 if the file could not be opened or a line could not be parsed.
 */
int baseline_load(Baseline *db, const char *filepath) {
    FILE *fp = NULL;
    char  line[MAX_LINE_LEN];
    char *sep  = NULL;
    int   line_num = 0;

    memset(db, 0, sizeof(Baseline));

    fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "baseline_load: could not open %s\n", filepath);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;

        /* Strip the trailing newline fgets leaves in */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        /* Skip blank lines */
        if (len == 0) continue;

        /* Find the last colon -- everything before it is the path,
         * everything after is the hash */
        sep = strrchr(line, ':');
        if (sep == NULL) {
            fprintf(stderr, "baseline_load: malformed line %d, skipping\n", line_num);
            continue;
        }

        if (db->count >= MAX_FILES) {
            fprintf(stderr, "baseline_load: MAX_FILES limit reached\n");
            fclose(fp);
            return -1;
        }

        /* Split the line at the colon */
        *sep = '\0';
        safe_strncpy(db->records[db->count].path, line,      MAX_PATH_LEN);
        safe_strncpy(db->records[db->count].hash, sep + 1,   MAX_HASH_LEN);
        db->count++;
    }

    if (ferror(fp)) {
        fprintf(stderr, "baseline_load: read error in %s\n", filepath);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}
