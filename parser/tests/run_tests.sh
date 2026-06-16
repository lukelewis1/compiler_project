#!/bin/bash
# run_tests.sh — COMP3722 Assignment 2 test suite
# Runs all test files through the lexer then parser and reports pass/fail.
# Usage: ./run_tests.sh
# Requires: lexer and parser binaries on PATH or in ../lexer/build and ./build

LEXER=../lexer/build/lexer
PARSER=./build/parser
TEST_DIR=./tests

echo "============================================"
echo " Parser Test Suite"
echo "============================================"
echo ""

pass=0
fail=0

run_test() {
    local file=$1
    local expect_success=$2   # "true" = should accept, "false" = should reject
    local label=$(basename "$file")

    # Run lexer to produce tokens.txt
    $LEXER "$file" > /dev/null 2>&1

    if [ ! -f tokens.txt ]; then
        echo "FAIL: $label (lexer produced no tokens.txt)"
        ((fail++))
        return
    fi

    # Run parser
    output=$($PARSER tokens.txt 2>&1)
    result=$(echo "$output" | grep "^Result:" | awk '{print $2}')

    if [ "$expect_success" = "true" ]; then
        if [ "$result" = "SUCCESS" ]; then
            echo "PASS: $label"
            ((pass++))
        else
            echo "FAIL: $label (expected SUCCESS, got $result)"
            ((fail++))
        fi
    else
        if [ "$result" != "SUCCESS" ]; then
            echo "PASS: $label (correctly rejected)"
            ((pass++))
        else
            echo "FAIL: $label (expected rejection, got SUCCESS)"
            ((fail++))
        fi
    fi

    rm -f tokens.txt
}

echo "--- Valid Input Tests (expect: SUCCESS) ---"
run_test "$TEST_DIR/01_simple_class.c"       true
run_test "$TEST_DIR/02_access_specifiers.c"  true
run_test "$TEST_DIR/03_functions.c"          true
run_test "$TEST_DIR/04_if_else.c"            true
run_test "$TEST_DIR/05_while_loop.c"         true
run_test "$TEST_DIR/06_for_loop.c"           true
run_test "$TEST_DIR/07_new_delete.c"         true
run_test "$TEST_DIR/08_multiple_classes.c"   true
echo ""

echo "--- Invalid Input Tests (expect: FAILED with recovery) ---"
run_test "$TEST_DIR/09_missing_class_name.c"       false
run_test "$TEST_DIR/10_missing_access_specifier.c" false
run_test "$TEST_DIR/11_missing_semicolon.c"        false
run_test "$TEST_DIR/12_missing_brace.c"            false
run_test "$TEST_DIR/13_invalid_panic_recovery.c"   false
run_test "$TEST_DIR/14_stress.c"                   true
run_test "$TEST_DIR/15_nested_malformed.c"         false
echo ""

echo "============================================"
echo " Results: $pass passed, $fail failed"
echo "============================================"