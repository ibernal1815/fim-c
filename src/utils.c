#include <stdio.h>
#include <string.h>
#include <time.h>
#include "utils.h"

/*
 * Writes a formatted timestamp into buf.
 * Format: "2025-03-31 14:22:05"
 * Uses localtime() so it reflects the system timezone.
 */
void get_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", t);
}

/*
 * Builds a full file path from a directory and a filename.
 * Handles whether or not dir already has a trailing slash.
 * Example: join_path(out, size, "/etc/fim", "passwd")
 *          -> out = "/etc/fim/passwd"
 */
void join_path(char *out, size_t out_len, const char *dir, const char *file) {
    size_t dir_len = strlen(dir);

    if (dir_len > 0 && dir[dir_len - 1] == '/') {
        snprintf(out, out_len, "%s%s", dir, file);
    } else {
        snprintf(out, out_len, "%s/%s", dir, file);
    }
}

/*
 * A safer strncpy wrapper. strncpy from the standard library does NOT
 * guarantee null-termination if src is longer than n. This always does.
 */
void safe_strncpy(char *dst, const char *src, size_t n) {
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}
