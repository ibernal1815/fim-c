#!/usr/bin/env bash

# The real test. Creates a baseline, tampers with files in three ways
# (modify, add, delete), runs --check, and verifies that all three
# change types appear in the log.

set -e

TESTDIR=$(mktemp -d)
echo "original content" > "$TESTDIR/stable.txt"
echo "will be changed"  > "$TESTDIR/target.txt"
echo "will be deleted"  > "$TESTDIR/ghost.txt"

cd "$(dirname "$0")/.."
make --silent
mkdir -p data

# Clear old log and baseline so this test runs clean
rm -f data/baseline.db data/fim.log

./fim --baseline "$TESTDIR" > /dev/null

# Tamper
echo "tampered content"  > "$TESTDIR/target.txt"
echo "brand new file"    > "$TESTDIR/newfile.txt"
rm "$TESTDIR/ghost.txt"

./fim --check "$TESTDIR" > /dev/null

MODIFIED=$(grep "MODIFIED" data/fim.log | wc -l)
ADDED=$(grep "ADDED"    data/fim.log | wc -l)
MISSING=$(grep "MISSING"  data/fim.log | wc -l)

rm -rf "$TESTDIR"

PASS=1

if [ "$MODIFIED" -ge 1 ]; then
    echo "[PASS] test_tamper: MODIFIED detected"
else
    echo "[FAIL] test_tamper: MODIFIED not detected"
    PASS=0
fi

if [ "$ADDED" -ge 1 ]; then
    echo "[PASS] test_tamper: ADDED detected"
else
    echo "[FAIL] test_tamper: ADDED not detected"
    PASS=0
fi

if [ "$MISSING" -ge 1 ]; then
    echo "[PASS] test_tamper: MISSING detected"
else
    echo "[FAIL] test_tamper: MISSING not detected"
    PASS=0
fi

[ "$PASS" -eq 1 ] && exit 0 || exit 1
