#!/bin/bash
# run_io_tests.sh — Input/output tests for the lexer
#
# Usage:
#   ./tests/run_io_tests.sh [path/to/lexer]
#
# Defaults to looking for 'lexer' in the project root or cmake-build-debug.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
INPUTS_DIR="$SCRIPT_DIR/inputs"
EXPECTED_DIR="$SCRIPT_DIR/expected"

# Resolve lexer binary — must be an absolute path so it survives cd into tmpdir
if [ $# -ge 1 ]; then
    LEXER="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
elif [ -x "$PROJECT_DIR/lexer" ]; then
    LEXER="$PROJECT_DIR/lexer"
elif [ -x "$PROJECT_DIR/cmake-build-debug/lexer" ]; then
    LEXER="$PROJECT_DIR/cmake-build-debug/lexer"
else
    echo "ERROR: cannot find lexer binary. Build first, or pass path as argument."
    echo "  Usage: $0 [path/to/lexer]"
    exit 1
fi

echo "Using lexer: $LEXER"
echo ""

PASSED=0
FAILED=0

run_test() {
    local name="$1"
    local input="$INPUTS_DIR/$name"
    local expected="$EXPECTED_DIR/$name"

    if [ ! -f "$input" ]; then
        echo "SKIP: $name (no input file)"
        return
    fi
    if [ ! -f "$expected" ]; then
        echo "SKIP: $name (no expected file)"
        return
    fi

    # Run lexer in a temp dir so tokens.txt lands there, not in the project root
    local tmpdir
    tmpdir=$(mktemp -d)
    cp "$input" "$tmpdir/input.txt"

    (cd "$tmpdir" && "$LEXER" input.txt > /dev/null 2>&1)

    # Strip the time elapsed line before comparing
    local actual expected_stripped
    actual=$(grep -v "^Time Elapsed:" "$tmpdir/tokens.txt")
    expected_stripped=$(grep -v "^Time Elapsed:" "$expected")

    rm -rf "$tmpdir"

    if [ "$actual" = "$expected_stripped" ]; then
        echo "PASS: $name"
        PASSED=$((PASSED + 1))
    else
        echo "FAIL: $name"
        echo "  --- expected ---"
        echo "$expected_stripped" | sed 's/^/    /'
        echo "  --- actual ---"
        echo "$actual" | sed 's/^/    /'
        FAILED=$((FAILED + 1))
    fi
}

run_test "01_keywords.txt"
run_test "02_identifiers.txt"
run_test "03_integers.txt"
run_test "04_operators.txt"
run_test "05_delimiters.txt"
run_test "06_invalid.txt"
run_test "07_mixed.txt"
run_test "08_empty.txt"
run_test "09_malformed.txt"
run_test "10_stress.txt"
run_test "11_boundary.txt"

echo ""
echo "=== I/O Tests: $PASSED passed, $FAILED failed ==="

[ "$FAILED" -eq 0 ]
