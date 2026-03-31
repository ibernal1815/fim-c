#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "hash.h"

/*
 * Hashes a file at filepath using SHA-256 via OpenSSL's EVP interface.
 * out_hash must be at least MAX_HASH_LEN bytes (65 bytes: 64 hex chars + null).
 *
 * Returns  0 on success.
 * Returns -1 on any failure (file not found, read error, OpenSSL error).
 *
 * EVP is OpenSSL's high-level API. It replaced the older SHA256() direct
 * call because it supports algorithm agility -- you can swap SHA-256 for
 * SHA-512 or anything else by changing one line.
 */
int hash_file(const char *filepath, char *out_hash) {
    FILE *fp = NULL;
    EVP_MD_CTX *ctx = NULL;
    unsigned char buffer[8192];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    size_t bytes_read = 0;
    int ret = -1;

    /* Open the file in binary mode. Always binary -- text mode on Windows
     * will mangle line endings and produce wrong hashes. */
    fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "hash_file: could not open %s\n", filepath);
        return -1;
    }

    /* Create and initialize an EVP context. Think of this as the
     * "state machine" that accumulates the hash as you feed it data. */
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "hash_file: failed to create EVP context\n");
        goto cleanup;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        fprintf(stderr, "hash_file: DigestInit failed\n");
        goto cleanup;
    }

    /* Feed the file into the hash context in 8KB chunks.
     * You never load the whole file into memory -- this works on files
     * of any size because you process it piece by piece. */
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (EVP_DigestUpdate(ctx, buffer, bytes_read) != 1) {
            fprintf(stderr, "hash_file: DigestUpdate failed\n");
            goto cleanup;
        }
    }

    if (ferror(fp)) {
        fprintf(stderr, "hash_file: read error on %s\n", filepath);
        goto cleanup;
    }

    /* Finalize -- this produces the raw binary digest (32 bytes for SHA-256) */
    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
        fprintf(stderr, "hash_file: DigestFinal failed\n");
        goto cleanup;
    }

    /* Convert raw bytes to a lowercase hex string.
     * Each byte becomes 2 hex characters, so 32 bytes -> 64 chars. */
    for (unsigned int i = 0; i < digest_len; i++) {
        sprintf(out_hash + (i * 2), "%02x", digest[i]);
    }
    out_hash[digest_len * 2] = '\0';

    ret = 0;

cleanup:
    if (ctx != NULL) EVP_MD_CTX_free(ctx);
    if (fp  != NULL) fclose(fp);
    return ret;
}
