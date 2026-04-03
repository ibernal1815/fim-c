#!/usr/bin/env bash

# Creates a file with known content, baselines a directory containing it,
# then checks that the stored hash matches sha256sum's output.

set -e

TESTDIR=$(mktemp -d)
TESTFILE="$TESTDIR/hashtest.txt"
echo "fim hash test string" > "$TESTFILE"

cd "$(dirname "$0")/.."
make --silent
mkdir -p data

./fim --baseline "$TESTDIR" > /dev/null

EXPECTED=$(sha256sum "$TESTFILE" | awk '{print $1}')
ACTUAL=$(grep "hashtest.txt" data/baseline.db | rev | cut -d: -f1 | rev)

rm -rf "$TESTDIR"

if [ "$EXPECTED" = "$ACTUAL" ]; then
    echo "[PASS] test_hash"
    exit 0
else
    echo "[FAIL] test_hash: expected=$EXPECTED got=$ACTUAL"
    exit 1
fi
