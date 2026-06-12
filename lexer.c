/*
 * Created by Luke Lewis on 19/5/2026.
 * lexer.c — COMP3722 Assignment 1
 *
 * DFA States (mapped directly to the diagram supplied):
 *   STATE_START      — initial / reset state
 *   STATE_IDENTIFIER — consuming an identifier or keyword
 *   STATE_INTEGER    — consuming an integer literal
 *   STATE_ERROR      — panic-mode error recovery
 *
 * Input symbol classes (σ):
 *   α  — letter or underscore  [a-zA-Z_]
 *   λ  — digit                 [0-9]
 *   ω  — delimiter             ; , ( ) { } [ ]
 *   β  — whitespace            space, \t, \r, \n
 *   γ  — (not used as a distinct class here; folds into ω/β)
 *   μ  — everything else (unrecognised) → ERROR transition
 */


#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constant for max token length
#define MAX_LEXEME 256

// Token types
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_DELIMITER,
    TOKEN_OPERATOR,
    TOKEN_INVALID
} TokenType;

static const char *token_types[] = {
    "KEYWORD", "IDENTIFIER", "INTEGER", "DELIMITER", "OPERATOR", "INVALID"
};

// Token record
typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME];
    int line;
    int col;
} Token;

// DFA States
typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_INTEGER,
    STATE_ERROR
} State;

// Keyword table
static const char *keywords[] = {
    "int", "char", "if", "else", "while", "for", "do", "return", NULL
};

// Global variables for tracking token statistics
int total_tokens = 0;
int valid_tokens = 0;
int token_type_counts[6] = {0};
int max_token_len = 0;

/**
 * Lexeme checked against keywords array and returns the result accordingly
 * @param lexeme
 * @return
 */
static bool is_keyword(char* lexeme) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(lexeme, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Checks character is valid delimiter
 * @param c
 * @return
 */
static bool is_delimiter(char c) {
    return (c == ',' || c == ';' || c == '('
            || c == ')' || c == '[' || c == ']'
            || c == '{' || c == '}');
}

/**
 * Checks character is valid operator
 * @param c
 * @return
 */
static bool is_operator(char c) {
    return (c == '+' || c == '-' || c == '*'
            || c == '/' || c == '>' || c == '<'
            || c == '=' || c == '!');
}


/**
 * Checks character is whitespace
 * @param c
 * @return
 */
static bool is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

/**
 *
 * @param lexeme the matches string from the source file
 * @param type the token type to emit
 * @param row line number where token appears
 * @param col column number where token appears
 * @param out  file pointer to write output to
 */
static void print_token(const Token *t, FILE* out) {
    total_tokens++;
    valid_tokens += t->type == TOKEN_INVALID ? 0 : 1;
    token_type_counts[t->type]++;

    printf("Lexeme: %s, Token Type: %s, Line: %d, Column: %d\n", t->lexeme, token_types[t->type], t->line, t->col);
    fprintf(out, "Lexeme: %s, Token Type: %s, Line: %d, Column: %d\n", t->lexeme, token_types[t->type], t->line, t->col);
}

/**
 * flush_token — finalises and emits a token when the DFA leaves a non-START state.
 * @param out – file pointer for token output
 * @param token_start – pointer to first char of token in source buffer
 * @param current – pointer to terminating char (not included in token)
 * @param line – line number where token begins (1-indexed)
 * @param col – column number where token begins (1-indexed)
 * @param state – DFA state being exited, determines token type
 */
static void flush_token(FILE *out, char *token_start, char *current, int line, int col, State state) {
    // guard for when no token is being built
    if (token_start == NULL) return;
    // produces number of character in token
    int len = (int)(current - token_start);
    // prevents invalid length
    if (len <= 0) return;

    // track the longest token seen for summary
    if (len > max_token_len) max_token_len = len;

    //enforce buffer limit – clamps to MAX_LEXEME - 1 to leave room for null terminator
    if (len > MAX_LEXEME - 1) {
        fprintf(out, "[WARN] Token at line %d col %d exceeds %d chars, truncating\n",
            line, col, MAX_LEXEME - 1);
        fprintf(stderr, "[WARN] Token at line %d col %d exceeds %d chars, truncating\n",
            line, col, MAX_LEXEME - 1);
        len = MAX_LEXEME - 1;
    }

    // Builds the Token struct
    Token t;
    memcpy(t.lexeme, token_start, len);
    t.lexeme[len] = 0;
    t.line = line;
    t.col = col;

    /* Determine token type from the state being exited.
     * IDENTIFIER covers both keywords and identifiers since the DFA cannot distinguish them without the full lexeme first
     */
    switch (state) {
    case STATE_IDENTIFIER:
            t.type = is_keyword(t.lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
            break;
    case STATE_INTEGER:
            t.type = TOKEN_INTEGER;
            break;
    case STATE_ERROR:
            t.type = TOKEN_INVALID;
            break;
    default:
            return; // STATE_START has nothing to flush
    }

    print_token(&t, out);
}

/**
 * Prints overall lexer statistics after parsing is complete
 * @param out – file pointer to write summary to
 * @param line_count – total number of lines scanned
 * @param seconds_elapsed – time taken to parse, measured in parse() with clock_t
 */
static void print_summary(FILE *out, int line_count, double seconds_elapsed) {
    fprintf(out, "\n");
    printf("\n");

    fprintf(out, "============================= Summary Statistics =============================\n");
    printf("============================= Summary Statistics =============================\n");

    /* Total token counts — invalid tokens are those produced by STATE_ERROR */
    fprintf(out, "Total Tokens: %d | Valid: %d | Invalid: %d\n",
            total_tokens, valid_tokens, token_type_counts[TOKEN_INVALID]);
    printf("Total Tokens: %d | Valid: %d | Invalid: %d\n",
           total_tokens, valid_tokens, token_type_counts[TOKEN_INVALID]);

    /* Breakdown by token type — indices match the TokenType enum order */
    fprintf(out, "Keywords: %d\n", token_type_counts[TOKEN_KEYWORD]);
    printf("Keywords: %d\n", token_type_counts[TOKEN_KEYWORD]);

    fprintf(out, "Identifiers: %d\n", token_type_counts[TOKEN_IDENTIFIER]);
    printf("Identifiers: %d\n", token_type_counts[TOKEN_IDENTIFIER]);

    fprintf(out, "Integers: %d\n", token_type_counts[TOKEN_INTEGER]);
    printf("Integers: %d\n", token_type_counts[TOKEN_INTEGER]);

    fprintf(out, "Operators: %d\n", token_type_counts[TOKEN_OPERATOR]);
    printf("Operators: %d\n", token_type_counts[TOKEN_OPERATOR]);

    fprintf(out, "Delimiters: %d\n", token_type_counts[TOKEN_DELIMITER]);
    printf("Delimiters: %d\n", token_type_counts[TOKEN_DELIMITER]);

    fprintf(out, "Lines Scanned: %d\n", line_count);
    printf("Lines Scanned: %d\n", line_count);

    /* Longest token is tracked in flush_token via the max_token_len global */
    fprintf(out, "Longest Token: %d characters\n", max_token_len);
    printf("Longest Token: %d characters\n", max_token_len);

    fprintf(out, "Time Elapsed: %.6f seconds\n", seconds_elapsed);
    printf("Time Elapsed: %.6f seconds\n", seconds_elapsed);

    fprintf(out, "==============================================================================\n");
    printf("==============================================================================\n");
}

/**
 * main DFA driver, walks the source buffer and emits tokens.
 * @param buffer – null-terminated source file contents
 * @param out – file pointer for token output
 */
void lexical_analyser(char *buffer, FILE *out) {
    clock_t start = clock();

    char *current = buffer; // pointer to current character being examined
    char *token_start = NULL; // pointer to first char of token being built
    int line = 1; // current line number, 1-indexed
    int col = 1; // current column number, 1-indexed
    int token_col = 1; // column where current token started
    State state = STATE_START;

    while (*current != '\0') {
        char c = *current;

        // DFA state transitions
        switch (state) {
            case STATE_START:
                token_col = col;
                if (isalpha((unsigned char)c) || c == '_') {
                    token_start = current;
                    state = STATE_IDENTIFIER;
                } else if (isdigit((unsigned char)c)) {
                    token_start = current;
                    state = STATE_INTEGER;
                } else if (is_delimiter(c)) {
                    Token t;
                    t.lexeme[0] = c; t.lexeme[1] = '\0';
                    t.type = TOKEN_DELIMITER;
                    t.line = line; t.col = token_col;
                    if (1 > max_token_len) max_token_len = 1;
                    print_token(&t, out);
                } else if (is_operator(c)) {
                    Token t;
                    t.lexeme[0] = c; t.lexeme[1] = '\0';
                    t.type = TOKEN_OPERATOR;
                    t.line = line; t.col = token_col;
                    if (1 > max_token_len) max_token_len = 1;
                    print_token(&t, out);
                } else if (is_whitespace(c)) {
                    if (c == '\n') {
                        line++;
                        col = 0;
                    }
                } else {
                    token_start = current;
                    state = STATE_ERROR;
                }
                break;

            case STATE_IDENTIFIER:
                if (isalnum((unsigned char)c) || c == '_') {
                    break;
                }

                flush_token(out, token_start, current, line, token_col, STATE_IDENTIFIER);
                token_start = NULL;
                state = STATE_START;
                continue;

            case STATE_INTEGER:
                if (isdigit((unsigned char)c)) {
                    break;
                }
                if (isalpha((unsigned char)c) || c == '_') {
                    state = STATE_ERROR;
                    break;
                }

                flush_token(out, token_start, current, line, token_col, STATE_INTEGER);
                token_start = NULL;
                state = STATE_START;
                continue;

            case STATE_ERROR:
                if (is_whitespace(c) || is_delimiter(c)) {
                    flush_token(out, token_start, current, line, token_col, STATE_ERROR);
                    token_start = NULL;
                    state = STATE_START;
                    continue;
                }
                break;
        }

        current++;
        col++;
    }

    // handles the case where the source file ends without a trailing whitespace or delimiter.
    flush_token(out, token_start, current, line, token_col, state);

    // Compute time elapsed since lexical_analyser() was called
    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;

    print_summary(out, line, seconds);
}

// main function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open source file for reading
    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        fprintf(stderr, "Error: cannot open '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* Determine file size using pointer arithmetic on the file stream,
     * then rewind to the beginning before reading */
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    rewind(in);

    // Allocate heap buffer — +1 for null terminator
    char *buffer = malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        fclose(in);
        return EXIT_FAILURE;
    }

    /* Read entire file into buffer and null terminate using pointer
     * arithmetic — buffer + bytes_read points to one past the last
     * byte read, which is where the null terminator belongs */
    size_t bytes_read = fread(buffer, 1, (size_t)file_size, in);
    *(buffer + bytes_read) = '\0';
    fclose(in);

    // Open output file for writing tokens
    FILE *out = fopen("tokens.txt", "w");
    if (out == NULL) {
        fprintf(stderr, "Error: cannot open tokens.txt for writing\n");
        free(buffer);
        return EXIT_FAILURE;
    }

    // Run the lexical analyser
    lexical_analyser(buffer, out);

    // Free all resources — whoever allocates/opens is responsible for cleanup
    free(buffer);
    fclose(out);

    return EXIT_SUCCESS;

}