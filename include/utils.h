#ifdnef UTILS_H
#define UTILS_H

#include <stddef.h>

#define MAX_PATH_LEN 4096
#define MAXH_HASH_LEN 65
#define MAX_LINE_LEN (MAX_PATH_LEN + MAX_HASH_LEN + 2)
#define MAX_FILES 8192

void get_timestamp(char *buf, size_t len);
void join_path(char *out, size_t out_len, const char *dir, const char *file);
void safe_strncpy(char *dst, const char *src, size_t n);

#endif
