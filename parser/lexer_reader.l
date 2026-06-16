%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yycolumn = 1;

static char* extract_lexeme(const char *line) {
    const char *start = strstr(line, "Lexeme: ");
    if (!start) return NULL;
    start += 8;
    const char *end = strstr(start, ", Token Type:");
    if (!end) return NULL;
    int len = (int)(end - start);
    char *val = (char*)malloc(len + 1);
    strncpy(val, start, len);
    val[len] = '\0';
    return val;
}

static char* extract_type(const char *line) {
    const char *start = strstr(line, "Token Type: ");
    if (!start) return NULL;
    start += 12;
    const char *end = strstr(start, ", Line:");
    if (!end) return NULL;
    int len = (int)(end - start);
    char *val = (char*)malloc(len + 1);
    strncpy(val, start, len);
    val[len] = '\0';
    return val;
}

static int extract_line_num(const char *line) {
    const char *p = strstr(line, "Line: ");
    if (!p) return 0;
    return atoi(p + 6);
}

static int extract_col(const char *line) {
    const char *p = strstr(line, "Column: ");
    if (!p) return 1;
    return atoi(p + 8);
}
%}

%option noyywrap

%%

^"Lexeme: ".+\n {
    char *lexeme = extract_lexeme(yytext);
    char *type   = extract_type(yytext);
    if (!lexeme || !type) { free(lexeme); free(type); }
    else {
        yylineno = extract_line_num(yytext);
        yycolumn = extract_col(yytext);

        if (strcmp(type, "KEYWORD") == 0) {
            int tok = -1;
            if      (strcmp(lexeme, "class")     == 0) tok = KW_CLASS;
            else if (strcmp(lexeme, "public")    == 0) tok = KW_PUBLIC;
            else if (strcmp(lexeme, "private")   == 0) tok = KW_PRIVATE;
            else if (strcmp(lexeme, "protected") == 0) tok = KW_PROTECTED;
            else if (strcmp(lexeme, "new")       == 0) tok = KW_NEW;
            else if (strcmp(lexeme, "delete")    == 0) tok = KW_DELETE;
            else if (strcmp(lexeme, "int")       == 0) tok = KW_INT;
            else if (strcmp(lexeme, "char")      == 0) tok = KW_CHAR;
            else if (strcmp(lexeme, "if")        == 0) tok = KW_IF;
            else if (strcmp(lexeme, "else")      == 0) tok = KW_ELSE;
            else if (strcmp(lexeme, "while")     == 0) tok = KW_WHILE;
            else if (strcmp(lexeme, "for")       == 0) tok = KW_FOR;
            else if (strcmp(lexeme, "return")    == 0) tok = KW_RETURN;
            free(lexeme); free(type);
            if (tok != -1) return tok;
        }
        else if (strcmp(type, "IDENTIFIER") == 0) {
            yylval.strval = lexeme;
            free(type);
            return IDENTIFIER;
        }
        else if (strcmp(type, "INTEGER") == 0) {
            yylval.strval = lexeme;
            free(type);
            return INTEGER_LITERAL;
        }
        else if (strcmp(type, "DELIMITER") == 0) {
            int tok = -1;
            if      (strcmp(lexeme, ";") == 0) tok = SEMICOLON;
            else if (strcmp(lexeme, ",") == 0) tok = COMMA;
            else if (strcmp(lexeme, ":") == 0) tok = COLON;
            else if (strcmp(lexeme, "(") == 0) tok = LPAREN;
            else if (strcmp(lexeme, ")") == 0) tok = RPAREN;
            else if (strcmp(lexeme, "{") == 0) tok = LBRACE;
            else if (strcmp(lexeme, "}") == 0) tok = RBRACE;
            else if (strcmp(lexeme, "[") == 0) tok = LBRACKET;
            else if (strcmp(lexeme, "]") == 0) tok = RBRACKET;
            free(lexeme); free(type);
            if (tok != -1) return tok;
        }
        else if (strcmp(type, "OPERATOR") == 0) {
            int tok = -1;
            if      (strcmp(lexeme, "+") == 0) tok = PLUS;
            else if (strcmp(lexeme, "-") == 0) tok = MINUS;
            else if (strcmp(lexeme, "*") == 0) tok = STAR;
            else if (strcmp(lexeme, "/") == 0) tok = SLASH;
            else if (strcmp(lexeme, "=") == 0) tok = EQUALS;
            else if (strcmp(lexeme, "<") == 0) tok = LESSTHAN;
            else if (strcmp(lexeme, ">") == 0) tok = GREATERTHAN;
            else if (strcmp(lexeme, ".") == 0) tok = DOT;
            free(lexeme); free(type);
            if (tok != -1) return tok;
        }
        else {
            /* INVALID or unknown — skip */
            free(lexeme); free(type);
        }
    }
}

.|\n   { /* skip summary block and blank lines */ }

%%
