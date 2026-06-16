# COMP3722 Assignment 1 — Lexical Analyser

Luke Lewis | May 2026

## Requirements

- C11 compiler: `clang` (default) or `gcc`
- `make`
- POSIX shell (for I/O tests)

## Build

```sh
make
```

Produces `./lexer` in the project root. To use `gcc` instead of `clang`:

```sh
CC=gcc make
```

## Run

```sh
./lexer <source_file>
```

Output is printed to stdout and also written to `tokens.txt` in the current directory.

**Example:**

```sh
./lexer tests/inputs/07_mixed.txt
```

## Test

Run all tests (unit + I/O):

```sh
make test
```

Run individually:

```sh
make test-unit   # 94 assertions via test_unit.c
make test-io     # 11 golden-file comparisons via tests/run_io_tests.sh
```

## Clean

```sh
make clean
```

Removes `lexer`, `unit_tests`, and `tokens.txt`.

## Executable size

The binary is built with `-Os -flto` and stripped, targeting the <50 KB hardware constraint.  
Current size (macOS): ~34 KB.
