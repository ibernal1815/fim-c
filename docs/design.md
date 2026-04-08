# Design notes

## Overview

fim-c is a file integrity monitor. It hashes every file in a target
directory using SHA-256, saves those hashes to a flat-file database,
and detects changes when run again later. The goal was to build
something functionally similar to Tripwire but small enough to read
and understand in one sitting.

## Architecture

The project is split into six modules plus main.

utils    --  shared buffer sizes, string helpers, timestamp generation.
             No dependencies on other modules. Everything else includes this.

hash     --  SHA-256 hashing via OpenSSL EVP. Takes a file path,
             returns a 64-character hex string. Reads files in 8KB
             chunks so it works on files of any size.

scan     --  recursive directory walker using opendir/readdir/lstat.
             Skips symlinks to avoid infinite loops. Collects all
             regular file paths into a FileList struct.

baseline --  writes and reads the snapshot database. Format is one
             line per file: path:hash. Uses strrchr to split on the
             last colon so paths containing colons are handled safely.

compare  --  two-pass diff. Pass one walks the current scan and checks
             each file against the baseline (modified or added). Pass
             two walks the baseline and checks for anything no longer
             present (missing).

report   --  formats and delivers output. Prints ANSI color-coded
             sections to stdout. Appends timestamped entries to
             data/fim.log in append mode.

main     --  parses argc/argv, dispatches to run_baseline() or
             run_check(). No logic beyond argument handling.

## Data flow

Baseline mode:
  scan_directory -> hash_file (per file) -> baseline_save -> report

Check mode:
  baseline_load -> scan_directory -> compare_baseline -> report_results

## Database format

data/baseline.db is a plain text file. One record per line:

  /absolute/path/to/file.txt:a3f1d2...64hexchars

The delimiter is the last colon on the line. This handles file paths
that contain colons, which are valid on Linux.

## Known limitations

- MAX_FILES is hardcoded at 8192. Directories with more files than
  this will be truncated.

- No daemon mode. The tool runs once and exits. Scheduling is left
  to cron or a systemd timer.

- No config file. The baseline path and log path are compile-time
  constants.

- Linear search in compare.c is O(n^2) in the worst case. Acceptable
  for thousands of files, not for millions.

- ANSI color codes are always printed. A future improvement would
  check isatty(STDOUT_FILENO) and disable colors when stdout is
  not a terminal.

## Possible extensions

- Daemon mode with inotify for real-time detection instead of polling.
- Config file for specifying excluded paths or file patterns.
- Email or syslog alerting on detection.
- Sorted baseline with binary search for better comparison performance.
- Recursive exclusion patterns (e.g. skip all .git directories).
