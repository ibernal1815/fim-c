#!/usr/bin/env bash

# Verifies that running --baseline creates data/baseline.db
# and that it contains at least one entry per file in the test directory.

set -e

TESTDIR=$(mktemp -d)
echo "alpha"   > "$TESTDIR/a.txt"
echo "beta"    > "$TESTDIR/b.txt"
echo "gamma"   > "$TESTDIR/c.txt"

cd "$(dirname "$0")/.."
make --silent
mkdir -p data

./fim --baseline "$TESTDIR" > /dev/null

if [ ! -f data/baseline.db ]; then
    echo "[FAIL] test_baseline: baseline.db was not created"
    rm -rf "$TESTDIR"
    exit 1
fi

COUNT=$(wc -l < data/baseline.db)

rm -rf "$TESTDIR"

if [ "$COUNT" -ge 3 ]; then
    echo "[PASS] test_baseline: baseline.db contains $COUNT records"
    exit 0
else
    echo "[FAIL] test_baseline: expected at least 3 records, got $COUNT"
    exit 1
fi
