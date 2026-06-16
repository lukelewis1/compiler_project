/*
* Parser.h — COMP3722 Assignment 2
 * Encapsulates the SLR(1) PDA — wraps yyparse(), manages the parse tree,
 * and reports syntax errors.
 */

#pragma once
#include <string>
#include <memory>
#include "ParseTree.h"

class Parser {
public:
    // Accepts path to tokens.txt produced by the A1 lexer
    explicit Parser(const std::string &tokensFilePath);

    // Opens tokens.txt, runs yyparse() shift-reduce loop, builds parse tree
    // Returns true if parse succeeded with no syntax errors
    bool parse();

    // Prints parse tree via ParseTree::print() — only valid after parse()
    void printTree() const;

    // Prints SUCCESS/FAILED and syntax error count
    void printSummary() const;

private:
    std::string tokensFilePath;        // path to lexer tokens.txt output
    std::shared_ptr<ParseTree> tree;   // parse tree built during reductions
    bool parseSuccess;                 // true if yyparse() == 0 and no errors
};