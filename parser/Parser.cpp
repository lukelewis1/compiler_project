/*
 * Parser.cpp — COMP3722 Assignment 2
 *
 * Encapsulates the SLR(1) pushdown automaton. Opens the lexer tokens.txt
 * file, runs the Bison-generated yyparse() loop, and wraps the resulting
 * parse tree root in a ParseTree for printing.
 *
 * PDA flow:
 *   yyin  → Flex yylex() reads tokens line by line
 *   yyparse() → consults ACTION/GOTO tables, calls grammar actions
 *   grammar actions → push/pop nodeStack, building parse tree bottom-up
 *   parseRoot → set on accept, wrapped in ParseTree by parse()
 *   errorCount → incremented by yyerror() on each syntax error
 */

#include "Parser.h"
#include "parser.tab.h"
#include <iostream>
#include <cstdio>

// Globals defined in parser.y
extern std::shared_ptr<ParseNode> parseRoot; // parse tree root, set on accept
extern int errorCount;                        // syntax error counter
extern FILE *yyin;                            // Flex input file pointer
extern int yyparse();                         // Bison SLR(1) parse loop

/*
 * Constructor — stores path to tokens.txt for use in parse().
 */
Parser::Parser(const std::string &tokensFilePath)
    : tokensFilePath(tokensFilePath), parseSuccess(false) {}

/*
 * parse() — opens tokens.txt, runs yyparse(), and wraps the parse tree.
 * Returns true on success (yyparse() == 0 and no syntax errors).
 */
bool Parser::parse() {
    ::errorCount = 0;                // reset before each parse
    // Assign tokens.txt to yyin so yylex() reads from it
    yyin = fopen(tokensFilePath.c_str(), "r");
    if (!yyin) {
        std::cerr << "Error: cannot open '" << tokensFilePath << "'\n";
        return false;
    }

    // Run the SLR(1) parse loop — returns 0 on success, 1 on unrecovered error
    int result = yyparse();
    fclose(yyin);

    // Success requires both yyparse() returning 0 and no panic mode errors
    parseSuccess = (result == 0 && ::errorCount == 0);

    // Wrap the global parse tree root for printing
    if (parseRoot) {
        tree = std::make_shared<ParseTree>(parseRoot);
    }

    return parseSuccess;
}

/*
 * printTree() — prints the parse tree using box-drawing characters.
 * Only prints on successful parse.
 */
void Parser::printTree() const {
    if (!tree) {
        std::cout << "No parse tree available — parse failed.\n";
        return;
    }
    // On a recovered parse the root is still built, just with
    // [error recovered] nodes in place of the malformed members.
    std::cout << (parseSuccess ? "\n===== Parse Tree =====\n"
                               : "\n===== Parse Tree (partial — errors recovered) =====\n");
    tree->print();
    std::cout << "======================\n";
}

/*
 * printSummary() — prints parse result and error count to stdout.
 */
void Parser::printSummary() const {
    std::cout << "\n===== Parse Summary =====\n";
    if (parseSuccess) {
        std::cout << "Result:  SUCCESS\n";
        std::cout << "Errors:  0\n";
    } else {
        std::cout << "Result:  FAILED\n";
        std::cout << "Errors:  " << ::errorCount << "\n";
    }
    std::cout << "=========================\n";
}