# Usage

## Building

    make

Requires gcc and libssl-dev. See README for install instructions.

## Creating a baseline

    ./fim --baseline /path/to/directory

Scans every file in the directory recursively, hashes each one with
SHA-256, and saves the results to data/baseline.db. A confirmation
is printed to the terminal and the event is logged to data/fim.log.

Run this on a known-good state of the directory before you expect
any changes. On a server you might run this right after initial setup
before any services start.

## Checking against the baseline

    ./fim --check /path/to/directory

Scans the directory fresh, compares against data/baseline.db, and
reports three categories of changes:

    MODIFIED   file exists in both but the hash changed
    ADDED      file exists now but was not in the baseline
    MISSING    file was in the baseline but no longer exists

Output is color-coded in the terminal. All changes are also written
to data/fim.log with a timestamp.

If no changes are detected, a CLEAN status is printed and logged.

## Reading the log

data/fim.log accumulates entries across every run. Each line is:

    [YYYY-MM-DD HH:MM:SS] TYPE: detail

Example:

    [2025-03-31 14:22:05] BASELINE: /etc/fim-test | 42 files snapshotted
    [2025-03-31 14:30:11] MODIFIED: /etc/fim-test/passwd
    [2025-03-31 14:30:11] MISSING:  /etc/fim-test/hosts
    [2025-03-31 14:30:11] SUMMARY:  /etc/fim-test | 1 modified  0 added  1 missing

## Running the tests

    make test

Runs all three test scripts in tests/. Each prints PASS or FAIL.
test_tamper.sh is the most important one -- it creates a full
baseline, tampers with three files, and verifies all three change
types are detected and logged correctly.
