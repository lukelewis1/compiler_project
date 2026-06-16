/*
* Token.h — COMP3722 Assignment 2
 *
 * Defines the Token class representing a single lexical token produced
 * by the A1 lexer. Each token has a lexeme, type, line, and column —
 * matching the output format of tokens.txt.
 *
 * Token objects are not used directly by the Bison parser (which reads
 * token types via yylex()), but provide a structured representation for
 * documentation and potential future use in semantic analysis.
 */

#pragma once
#include <string>

// Token types matching the A1 lexer output categories
enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    INTEGER,
    DELIMITER,
    OPERATOR,
    INVALID
};

/*
 * Token — represents a single lexeme from the token stream.
 * lexeme — the exact string matched from the source file
 * type   — the token category (keyword, identifier, etc.)
 * line   — line number where the token appears (1-indexed)
 * col    — column number where the token appears (1-indexed)
 */
class Token {
public:
    std::string lexeme;
    TokenType   type;
    int         line;
    int         col;

    Token(const std::string &lexeme, TokenType type, int line, int col)
        : lexeme(lexeme), type(type), line(line), col(col) {}

    // Returns the token type as a string for display purposes
    std::string typeString() const {
        switch (type) {
            case TokenType::KEYWORD:    return "KEYWORD";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::INTEGER:    return "INTEGER";
            case TokenType::DELIMITER:  return "DELIMITER";
            case TokenType::OPERATOR:   return "OPERATOR";
            case TokenType::INVALID:    return "INVALID";
            default:                    return "UNKNOWN";
        }
    }
};