/*
* main.cpp — COMP3722 Assignment 2
 *
 * Entry point for the SLR(1) parser. Accepts a tokens.txt file path
 * as a command-line argument, runs the parser, and prints the parse
 * tree and summary to stdout.
 *
 * Usage: ./parser <tokens_file>
 * Example: ./parser tokens.txt
 */

#include <iostream>
#include <string>
#include "Parser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <tokens_file>\n";
        return 1;
    }

    std::string tokensFile = argv[1];

    // Construct the parser — encapsulates the SLR(1) PDA and parse tree
    Parser parser(tokensFile);

    std::cout << "Parsing: " << tokensFile << "\n";

    // Run the shift-reduce parse loop via yyparse()
    bool success = parser.parse();

    // Print the parse tree built during reductions, then the result summary
    parser.printTree();
    parser.printSummary();

    // Return 0 on success so the shell can check the exit code
    return success ? 0 : 1;
}