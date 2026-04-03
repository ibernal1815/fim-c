#!/usr/bin/env bash

# Verifies that fim's SHA-256 output matches the system's sha256sum.
# If these differ, hash.c has a bug.

set -e

TESTFILE=$(mktemp)
echo "fim hash test" > "$TESTFILE"

EXPECTED=$(sha256sum "$TESTFILE" | awk '{print $1}')

make -C "$(dirname "$0")/.." --silent

ACTUAL=$(./fim --baseline "$(dirname "$TESTFILE")" > /dev/null && \
         grep "$TESTFILE" data/baseline.db | cut -d: -f2)

rm -f "$TESTFILE"

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "[PASS] test_hash: hashes match"
    exit 0
else
    echo "[FAIL] test_hash: expected $EXPECTED got $ACTUAL"
    exit 1
fi
