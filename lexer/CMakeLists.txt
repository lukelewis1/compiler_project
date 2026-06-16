cmake_minimum_required(VERSION 3.28)
project(Lexer C)

set(CMAKE_C_STANDARD 11)

add_executable(lexer lexer.c)

# ── Tests ──────────────────────────────────────────────────────────────────
enable_testing()

# Unit tests (compiles tests/test_unit.c which #includes lexer.c internally)
add_executable(unit_tests tests/test_unit.c)
add_test(NAME UnitTests COMMAND unit_tests)

# I/O tests via shell script — pass the lexer binary path as an argument
add_test(
    NAME IOTests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_io_tests.sh $<TARGET_FILE:lexer>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)