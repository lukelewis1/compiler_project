/*
 * test_unit.c — Unit tests for lexer.c
 *
 * Includes lexer.c directly so it can reach the static helper functions.
 * The lexer's main() is renamed to lexer_main() to avoid a linker conflict.
 *
 * Build:
 *   clang -Wall -Wextra -std=c11 -o unit_tests tests/test_unit.c && ./unit_tests
 *
 * Or via CMake/CTest:
 *   cmake --build cmake-build-debug && ctest --test-dir cmake-build-debug -V
 */

#define main lexer_main
#include "../lexer.c"
#undef main

#include <fcntl.h>   /* open, O_WRONLY */
#include <unistd.h>  /* dup, dup2, close, STDOUT_FILENO */

/* ── Test infrastructure ────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;

#define ASSERT(cond, msg)                                          \
    do {                                                           \
        tests_run++;                                               \
        if (cond) {                                                \
            tests_passed++;                                        \
            printf("  PASS: %s\n", msg);                          \
        } else {                                                   \
            printf("  FAIL: %s  (line %d)\n", msg, __LINE__);     \
        }                                                          \
    } while (0)

/* Reset the global counters that lexical_analyser accumulates into. */
static void reset_globals(void) {
    total_tokens = 0;
    valid_tokens = 0;
    max_token_len = 0;
    memset(token_type_counts, 0, sizeof(token_type_counts));
}

/*
 * Silence stdout for the duration of a lexer run so the lexer's own printf
 * output doesn't pollute the unit-test output.  We redirect fd 1 to
 * /dev/null, run the function, then restore it.
 */
static int saved_stdout = -1;

static void silence_stdout(void) {
    saved_stdout = dup(STDOUT_FILENO);
    int devnull = open("/dev/null", O_WRONLY);
    dup2(devnull, STDOUT_FILENO);
    close(devnull);
}

static void restore_stdout(void) {
    if (saved_stdout >= 0) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        saved_stdout = -1;
    }
}

/*
 * Run the lexer on a string literal and capture the FILE* output into
 * lex_out_buf for assertions to inspect.
 */
static char lex_out_buf[8192];

static void run_lexer(const char *input) {
    reset_globals();

    /* lexical_analyser writes into the buffer via pointer — it needs a
     * writable copy since it scans by advancing a char* pointer. */
    char *buf = malloc(strlen(input) + 1);
    strcpy(buf, input);

    FILE *tmp = tmpfile();
    silence_stdout();
    lexical_analyser(buf, tmp);
    restore_stdout();
    free(buf);

    rewind(tmp);
    size_t n = fread(lex_out_buf, 1, sizeof(lex_out_buf) - 1, tmp);
    lex_out_buf[n] = '\0';
    fclose(tmp);
}

/* ── Helper-function unit tests ─────────────────────────────────────────── */

static void test_is_keyword(void) {
    printf("is_keyword:\n");
    /* All keywords defined in the table */
    ASSERT(is_keyword("int"),     "recognises 'int'");
    ASSERT(is_keyword("char"),    "recognises 'char'");
    ASSERT(is_keyword("if"),      "recognises 'if'");
    ASSERT(is_keyword("else"),    "recognises 'else'");
    ASSERT(is_keyword("while"),   "recognises 'while'");
    ASSERT(is_keyword("for"),     "recognises 'for'");
    ASSERT(is_keyword("do"),      "recognises 'do'");
    ASSERT(is_keyword("return"),  "recognises 'return'");
    /* Negative cases */
    ASSERT(!is_keyword("Int"),     "case-sensitive: 'Int' is not a keyword");
    ASSERT(!is_keyword("integer"), "'integer' is not a keyword");
    ASSERT(!is_keyword("in"),      "prefix 'in' is not a keyword");
    ASSERT(!is_keyword(""),        "empty string is not a keyword");
    ASSERT(!is_keyword("foo"),     "plain identifier is not a keyword");
}

static void test_is_delimiter(void) {
    printf("is_delimiter:\n");
    ASSERT(is_delimiter(';'), "';'");
    ASSERT(is_delimiter(','), "','");
    ASSERT(is_delimiter('('), "'('");
    ASSERT(is_delimiter(')'), "')'");
    ASSERT(is_delimiter('{'), "'{'");
    ASSERT(is_delimiter('}'), "'}'");
    ASSERT(is_delimiter('['), "'['");
    ASSERT(is_delimiter(']'), "']'");
    /* Negative cases */
    ASSERT(!is_delimiter('a'),  "'a' not a delimiter");
    ASSERT(!is_delimiter('1'),  "'1' not a delimiter");
    ASSERT(!is_delimiter('+'),  "'+' not a delimiter");
    ASSERT(!is_delimiter(' '),  "space not a delimiter");
    ASSERT(!is_delimiter('\n'), "newline not a delimiter");
}

static void test_is_operator(void) {
    printf("is_operator:\n");
    ASSERT(is_operator('+'), "'+'");
    ASSERT(is_operator('-'), "'-'");
    ASSERT(is_operator('*'), "'*'");
    ASSERT(is_operator('/'), "'/'");
    ASSERT(is_operator('>'), "'>'");
    ASSERT(is_operator('<'), "'<'");
    ASSERT(is_operator('='), "'='");
    ASSERT(is_operator('!'), "'!'");
    /* Negative cases */
    ASSERT(!is_operator('a'),  "'a' not an operator");
    ASSERT(!is_operator('('),  "'(' not an operator");
    ASSERT(!is_operator(' '),  "space not an operator");
    ASSERT(!is_operator('\n'), "newline not an operator");
}

static void test_is_whitespace(void) {
    printf("is_whitespace:\n");
    ASSERT(is_whitespace(' '),  "space");
    ASSERT(is_whitespace('\t'), "tab");
    ASSERT(is_whitespace('\r'), "carriage return");
    ASSERT(is_whitespace('\n'), "newline");
    /* Negative cases */
    ASSERT(!is_whitespace('a'), "'a' not whitespace");
    ASSERT(!is_whitespace(';'), "';' not whitespace");
    ASSERT(!is_whitespace('+'), "'+' not whitespace");
    ASSERT(!is_whitespace('0'), "'0' not whitespace");
}

/* ── lexical_analyser integration tests ─────────────────────────────────── */

static void test_lexer_keywords(void) {
    printf("lexical_analyser — keywords:\n");

    run_lexer("int");
    ASSERT(strstr(lex_out_buf, "Lexeme: int, Token Type: KEYWORD"),  "emits KEYWORD for 'int'");
    ASSERT(total_tokens == 1,                                         "exactly 1 token");
    ASSERT(token_type_counts[TOKEN_KEYWORD] == 1,                     "keyword count is 1");

    run_lexer("return");
    ASSERT(strstr(lex_out_buf, "Lexeme: return, Token Type: KEYWORD"), "emits KEYWORD for 'return'");

    /* All eight keywords in one pass */
    run_lexer("int char if else while for do return");
    ASSERT(token_type_counts[TOKEN_KEYWORD] == 8, "all 8 keywords recognised");
    ASSERT(total_tokens == 8,                     "8 tokens total");
}

static void test_lexer_identifiers(void) {
    printf("lexical_analyser — identifiers:\n");

    run_lexer("foo");
    ASSERT(strstr(lex_out_buf, "Lexeme: foo, Token Type: IDENTIFIER"), "'foo' is IDENTIFIER");
    ASSERT(total_tokens == 1, "1 token");

    run_lexer("_under");
    ASSERT(strstr(lex_out_buf, "Lexeme: _under, Token Type: IDENTIFIER"), "underscore-prefix identifier");

    run_lexer("camel99");
    ASSERT(strstr(lex_out_buf, "Lexeme: camel99, Token Type: IDENTIFIER"), "alphanumeric identifier");

    /* 'integer' looks like a keyword prefix but is not in the table */
    run_lexer("integer");
    ASSERT(strstr(lex_out_buf, "Lexeme: integer, Token Type: IDENTIFIER"), "'integer' is IDENTIFIER not KEYWORD");

    /* Digit after alpha is fine inside an identifier */
    run_lexer("int3");
    ASSERT(strstr(lex_out_buf, "Lexeme: int3, Token Type: IDENTIFIER"), "'int3' is IDENTIFIER (digit inside)");
}

static void test_lexer_integers(void) {
    printf("lexical_analyser — integers:\n");

    run_lexer("42");
    ASSERT(strstr(lex_out_buf, "Lexeme: 42, Token Type: INTEGER"), "'42' is INTEGER");
    ASSERT(total_tokens == 1, "1 token");

    run_lexer("0");
    ASSERT(strstr(lex_out_buf, "Lexeme: 0, Token Type: INTEGER"), "zero is INTEGER");

    run_lexer("0 123 42 9999");
    ASSERT(token_type_counts[TOKEN_INTEGER] == 4, "4 integer tokens");
}

static void test_lexer_operators(void) {
    printf("lexical_analyser — operators:\n");

    run_lexer("+");
    ASSERT(strstr(lex_out_buf, "Lexeme: +, Token Type: OPERATOR"), "'+' is OPERATOR");

    /* Two operators with no separator — each emitted immediately */
    run_lexer("!=");
    ASSERT(strstr(lex_out_buf, "Lexeme: !, Token Type: OPERATOR"), "'!' is OPERATOR");
    ASSERT(strstr(lex_out_buf, "Lexeme: =, Token Type: OPERATOR"), "'=' is OPERATOR");
    ASSERT(total_tokens == 2, "2 operator tokens");

    run_lexer("+ - * / > < = !");
    ASSERT(token_type_counts[TOKEN_OPERATOR] == 8, "all 8 operators recognised");
}

static void test_lexer_delimiters(void) {
    printf("lexical_analyser — delimiters:\n");

    run_lexer(";");
    ASSERT(strstr(lex_out_buf, "Lexeme: ;, Token Type: DELIMITER"), "';' is DELIMITER");

    run_lexer("()");
    ASSERT(strstr(lex_out_buf, "Lexeme: (, Token Type: DELIMITER"), "'(' is DELIMITER");
    ASSERT(strstr(lex_out_buf, "Lexeme: ), Token Type: DELIMITER"), "')' is DELIMITER");
    ASSERT(total_tokens == 2, "2 delimiter tokens");

    run_lexer("; , ( ) { } [ ]");
    ASSERT(token_type_counts[TOKEN_DELIMITER] == 8, "all 8 delimiters recognised");
}

static void test_lexer_invalid(void) {
    printf("lexical_analyser — invalid:\n");

    run_lexer("@");
    ASSERT(strstr(lex_out_buf, "Token Type: INVALID"), "'@' is INVALID");
    ASSERT(token_type_counts[TOKEN_INVALID] == 1,  "invalid count is 1");
    ASSERT(valid_tokens == 0,                       "no valid tokens");

    /* Digit immediately followed by alpha letters → error state → INVALID */
    run_lexer("3abc");
    ASSERT(strstr(lex_out_buf, "Lexeme: 3abc, Token Type: INVALID"), "'3abc' is one INVALID token");

    /* Multiple invalid tokens separated by spaces */
    run_lexer("@ # $");
    ASSERT(token_type_counts[TOKEN_INVALID] == 3, "3 separate INVALID tokens");
}

static void test_lexer_mixed(void) {
    printf("lexical_analyser — mixed input:\n");

    run_lexer("int x = 5;");
    ASSERT(strstr(lex_out_buf, "Lexeme: int, Token Type: KEYWORD"),   "keyword 'int'");
    ASSERT(strstr(lex_out_buf, "Lexeme: x, Token Type: IDENTIFIER"),  "identifier 'x'");
    ASSERT(strstr(lex_out_buf, "Lexeme: =, Token Type: OPERATOR"),    "operator '='");
    ASSERT(strstr(lex_out_buf, "Lexeme: 5, Token Type: INTEGER"),     "integer '5'");
    ASSERT(strstr(lex_out_buf, "Lexeme: ;, Token Type: DELIMITER"),   "delimiter ';'");
    ASSERT(total_tokens == 5, "5 tokens total");
}

static void test_lexer_empty(void) {
    printf("lexical_analyser — empty input:\n");

    run_lexer("");
    ASSERT(total_tokens == 0, "empty input: 0 tokens");
    ASSERT(valid_tokens == 0, "empty input: 0 valid tokens");
}

static void test_lexer_multiline(void) {
    printf("lexical_analyser — multiline:\n");

    run_lexer("int x;\nint y;");
    ASSERT(token_type_counts[TOKEN_KEYWORD]    == 2, "2 keywords");
    ASSERT(token_type_counts[TOKEN_IDENTIFIER] == 2, "2 identifiers");
    ASSERT(token_type_counts[TOKEN_DELIMITER]  == 2, "2 delimiters");
    ASSERT(total_tokens == 6,                        "6 tokens total");
    /* The second statement starts on line 2 */
    ASSERT(strstr(lex_out_buf, "Line: 2") != NULL,   "line 2 is tracked");
}

static void test_lexer_statistics(void) {
    printf("lexical_analyser — statistics:\n");

    run_lexer("foo bar baz");
    ASSERT(valid_tokens == 3,                        "valid count == total for clean input");
    ASSERT(token_type_counts[TOKEN_INVALID] == 0,    "no invalid tokens");
    ASSERT(max_token_len == 3,                        "longest token is 3 chars");

    run_lexer("longerword");
    ASSERT(max_token_len == 10,                       "longest token updated to 10");
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Lexer Unit Tests ===\n\n");

    test_is_keyword();
    test_is_delimiter();
    test_is_operator();
    test_is_whitespace();
    printf("\n");
    test_lexer_keywords();
    test_lexer_identifiers();
    test_lexer_integers();
    test_lexer_operators();
    test_lexer_delimiters();
    test_lexer_invalid();
    test_lexer_mixed();
    test_lexer_empty();
    test_lexer_multiline();
    test_lexer_statistics();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
