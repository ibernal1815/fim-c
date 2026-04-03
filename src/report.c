#include <stdio.h>
#include <string.h>
#include "report.h"
#include "utils.h"

/*
 * ANSI escape codes for terminal color output.
 * These are just strings that terminals interpret as color instructions.
 * \033[ is the escape sequence prefix (ESC + '[').
 * 0m resets back to default.
 * 31m = red, 33m = yellow, 32m = green, 36m = cyan, 1m = bold.
 *
 * If you ever pipe output to a file or a tool that does not understand
 * ANSI codes, the raw escape strings will show up as garbage. A more
 * complete implementation would check isatty(STDOUT_FILENO) and disable
 * colors automatically. For now this is fine.
 */
#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_CYAN   "\033[36m"
#define COLOR_BOLD   "\033[1m"

/*
 * Writes a single log entry to LOG_PATH.
 * Opens in append mode so entries accumulate over time.
 * Each entry is one line: [timestamp] TYPE: path
 *
 * Fails silently if the log file cannot be opened -- we do not want
 * a logging failure to crash the tool or hide real detections from
 * the terminal output.
 */
static void write_log_entry(const char *timestamp, const char *type,
                             const char *path) {
    FILE *fp = fopen(LOG_PATH, "a");
    if (fp == NULL) {
        fprintf(stderr, "report: could not open log file %s\n", LOG_PATH);
        return;
    }
    fprintf(fp, "[%s] %s: %s\n", timestamp, type, path);
    fclose(fp);
}

/*
 * Prints a section header to stdout.
 * Used to visually separate MODIFIED / ADDED / MISSING sections.
 */
static void print_section_header(const char *color, const char *label) {
    printf("\n%s%s%s-- %s --%s\n",
           COLOR_BOLD, color, COLOR_BOLD, label, COLOR_RESET);
}

/*
 * Prints all results to the terminal and logs every change.
 *
 * Output is grouped into three sections: modified, added, missing.
 * Each section only prints if there are changes of that type.
 * If there are no changes at all, a clean status message is printed.
 *
 * The hash display for MODIFIED entries is truncated to 12 characters.
 * Full SHA-256 hashes are 64 characters -- showing the full string on
 * every line makes the output hard to scan quickly. 12 characters is
 * enough to visually confirm they differ.
 */
void report_results(const ChangeList *changes, const char *target_dir) {
    char timestamp[32];
    int  modified_count = 0;
    int  added_count    = 0;
    int  missing_count  = 0;

    get_timestamp(timestamp, sizeof(timestamp));

    /* Print the report header */
    printf("\n%sFIM Report%s\n", COLOR_BOLD, COLOR_RESET);
    printf("%sTarget:%s  %s\n", COLOR_CYAN, COLOR_RESET, target_dir);
    printf("%sTime:%s    %s\n", COLOR_CYAN, COLOR_RESET, timestamp);
    printf("%sChanges:%s %d\n", COLOR_CYAN, COLOR_RESET, changes->count);

    if (changes->count == 0) {
        printf("\n%s[CLEAN]%s No changes detected.\n",
               COLOR_GREEN, COLOR_RESET);
        write_log_entry(timestamp, "CLEAN", target_dir);
        return;
    }

    /* Modified files */
    for (int i = 0; i < changes->count; i++) {
        if (changes->changes[i].type != CHANGE_MODIFIED) continue;
        if (modified_count == 0) {
            print_section_header(COLOR_YELLOW, "MODIFIED");
        }
        const Change *c = &changes->changes[i];
        printf("  %s%s%s\n", COLOR_YELLOW, c->path, COLOR_RESET);
        printf("    old: %.12s...  new: %.12s...\n",
               c->old_hash, c->new_hash);
        write_log_entry(timestamp, "MODIFIED", c->path);
        modified_count++;
    }

    /* Added files */
    for (int i = 0; i < changes->count; i++) {
        if (changes->changes[i].type != CHANGE_ADDED) continue;
        if (added_count == 0) {
            print_section_header(COLOR_RED, "ADDED");
        }
        const Change *c = &changes->changes[i];
        printf("  %s%s%s\n", COLOR_RED, c->path, COLOR_RESET);
        printf("    hash: %.12s...\n", c->new_hash);
        write_log_entry(timestamp, "ADDED", c->path);
        added_count++;
    }

    /* Missing files */
    for (int i = 0; i < changes->count; i++) {
        if (changes->changes[i].type != CHANGE_MISSING) continue;
        if (missing_count == 0) {
            print_section_header(COLOR_RED, "MISSING");
        }
        const Change *c = &changes->changes[i];
        printf("  %s%s%s\n", COLOR_RED, c->path, COLOR_RESET);
        printf("    last hash: %.12s...\n", c->old_hash);
        write_log_entry(timestamp, "MISSING", c->path);
        missing_count++;
    }

    /* Summary line */
    printf("\n%sSummary:%s  %d modified  %d added  %d missing\n",
           COLOR_BOLD, COLOR_RESET,
           modified_count, added_count, missing_count);

    /* Write a summary entry to the log as well */
    char summary[128];
    snprintf(summary, sizeof(summary),
             "%s | %d modified  %d added  %d missing",
             target_dir, modified_count, added_count, missing_count);
    write_log_entry(timestamp, "SUMMARY", summary);
}

/*
 * Confirmation message printed after a baseline is successfully created.
 * Also logs the event so there is a record of when the snapshot was taken.
 */
void report_baseline_created(const char *target_dir, int file_count) {
    char timestamp[32];
    char log_msg[MAX_PATH_LEN + 64];

    get_timestamp(timestamp, sizeof(timestamp));

    printf("\n%s[BASELINE]%s Snapshot created.\n", COLOR_GREEN, COLOR_RESET);
    printf("%sTarget:%s    %s\n", COLOR_CYAN, COLOR_RESET, target_dir);
    printf("%sFiles:%s     %d\n", COLOR_CYAN, COLOR_RESET, file_count);
    printf("%sTime:%s      %s\n", COLOR_CYAN, COLOR_RESET, timestamp);
    printf("%sDatabase:%s  %s\n", COLOR_CYAN, COLOR_RESET, "data/baseline.db");

    snprintf(log_msg, sizeof(log_msg),
             "%s | %d files snapshotted", target_dir, file_count);
    write_log_entry(timestamp, "BASELINE", log_msg);
}
