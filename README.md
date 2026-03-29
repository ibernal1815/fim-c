# fim-c

A file integrity monitor written in C. The tool takes a snapshot of a directory by hashing every file with SHA-256, saves those hashes to a local database, and then lets you compare the current state of that directory against the snapshot at any time. Anything added, removed, or modified gets flagged.

This is the kind of tool that shows up in real blue team environments. Tripwire does the same thing at an enterprise scale. This is a much smaller version built to understand how it works under the hood.

---

## Why I built this

I am working toward a career in detection engineering and digital forensics. File integrity monitoring is one of the core concepts in that space, so building it from scratch in C made sense. It also gave me a reason to get comfortable with things like directory traversal, file I/O, and process-level hashing in C, which I had not done before.

---

## How it works

There are two modes.

**Baseline mode** walks a target directory, hashes every file it finds using SHA-256, and writes the results to `data/baseline.db`. Each line in that file is just `filepath:hash`.

**Check mode** walks the same directory again, hashes everything fresh, and compares the results against the stored baseline. It reports three things: files that were modified, files that are new, and files that are missing. Output goes to the terminal and to `data/fim.log` with a timestamp.

---

## Dependencies

You need GCC and OpenSSL installed.

On Ubuntu or Debian:

```
sudo apt install gcc libssl-dev
```

On macOS with Homebrew:

```
brew install openssl
```

---

## Build

```
make
```

That compiles everything in `src/` and produces a `fim` binary in the project root.

To clean up the build:

```
make clean
```

---

## Usage

Create a baseline of a directory:

```
./fim --baseline /path/to/directory
```

Check the directory against the saved baseline:

```
./fim --check /path/to/directory
```

The log file is written to `data/fim.log`. The baseline database is stored at `data/baseline.db`. Both are excluded from version control.

---

## Running the tests

```
make test
```

The test scripts live in `tests/`. They verify that hashing produces correct output, that the baseline file gets created properly, and that a modified file actually triggers an alert.

---

## Project structure

```
fim-c/
  src/          source files
  include/      header files
  tests/        shell-based test scripts
  docs/         design notes and usage details
  data/         runtime output, gitignored
  Makefile
  README.md
```

---

## Status

This is a learning project. It works but it is not production ready. There is no daemon mode, no config file, and no scheduling built in. Those are things I plan to add.
