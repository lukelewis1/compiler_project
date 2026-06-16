# COMP3722 Assignment 2 — Parser
This README is for Luke Lewis's (lewi0454) parser solution for Compiler Theory and Practice COMP3722.
The parser is an SLR(1) pushdown automaton generated with GNU Bison 2.3 and Flex 2.6.4, built with CMake 4.2.3 (C++17, clang).

## Solution Package Structure
The grammar and scanner specifications live at the top level, with the parse-tree
data structures and the driver in C++. Build output goes to `build/` and is not tracked.
```
parser/
├── parser.y              # Bison grammar — SLR(1) rules + parse-tree actions
├── lexer_reader.l        # Flex scanner — reads tokens.txt back into tokens
├── main.cpp              # Entry point — runs parse, prints tree + summary
├── Parser.cpp            # Parser class — wraps yyparse(), owns the parse tree
├── Parser.h
├── ParseTree.h           # ParseNode / ParseTree — build + print the tree
├── Token.h               # Token representation matching tokens.txt format
├── CMakeLists.txt        # Build config (find_package BISON + FLEX REQUIRED)
├── cfg.txt               # Context-free grammar reference
├── FIRST_set.txt         # Computed FIRST sets
├── FOLLOW_set.txt        # Computed FOLLOW sets
└── tests/
    ├── 01_simple_class.c … 15_nested_malformed.c
    └── run_tests.sh      # Lexes + parses every test, reports pass/fail
```

The parser consumes the `tokens.txt` produced by the A1 lexer — it does **not** read `.c`
source directly. [lexer_reader.l](lexer_reader.l) reads each token line back in, Bison's
`yyparse()` in [parser.y](parser.y) drives the shift-reduce loop, and on each reduction the
grammar actions build the parse tree bottom-up via [ParseTree.h](ParseTree.h).

## Building
Bison and Flex are required to build (they regenerate the scanner and tables on every fresh
build); they are not needed to run the compiled binary.
```bash
cmake -S . -B build      # configure (first time only)
cmake --build build      # compile → build/parser
```

## How to use
The parser takes a `tokens.txt` file as its only argument. Produce one with the lexer, then
parse it:
```bash
../lexer/build/lexer tests/01_simple_class.c   # writes tokens.txt
./build/parser tokens.txt
```
On a valid input the parser prints the full parse tree and `Result: SUCCESS`. On a malformed
input it recovers in panic mode, prints a partial tree marking each `[error recovered]` point,
and reports `Result: FAILED` with the error count.

## Running the tests
The suite lexes and parses all 15 test cases and checks each against its expected outcome
(valid inputs must be accepted, malformed inputs must be rejected with recovery):
```bash
bash tests/run_tests.sh
```
Expected result: `15 passed, 0 failed`. Tests `01`–`08` are valid programs expected to succeed,
`09`–`13` and `15` are malformed and expected to be rejected, and `14` is a valid stress test.
