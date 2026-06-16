/*
 * ParseTree.h — COMP3722 Assignment 2
 *
 * Defines ParseNode and ParseTree for representing and printing the
 * parse tree constructed during SLR(1) reductions.
 *
 * Tree construction (in parser.y):
 *   On each reduction, child nodes are popped from the global nodeStack,
 *   attached to a new parent ParseNode labelled with the rule name,
 *   and the parent is pushed back. Terminal nodes use the lexeme as label.
 *   Error recovery nodes use "[error recovered]" as label.
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

/*
 * ParseNode — one node in the parse tree.
 * label    — grammar rule name (e.g. "ClassDeclaration") or terminal lexeme
 * children — RHS symbols of the grammar rule that produced this node
 */
class ParseNode {
public:
    std::string label;
    std::vector<std::shared_ptr<ParseNode>> children;

    explicit ParseNode(const std::string &label) : label(label) {}

    // Appends a child node — called during grammar rule reductions in parser.y
    void addChild(std::shared_ptr<ParseNode> child) {
        children.push_back(child);
    }
};

/*
 * ParseTree — owns the root node and handles recursive printing.
 * Prints the tree using box-drawing characters to
 * visualise the parse hierarchy from Program down to terminals.
 */
class ParseTree {
public:
    std::shared_ptr<ParseNode> root;

    explicit ParseTree(std::shared_ptr<ParseNode> root) : root(root) {}

    // Prints the full parse tree to stdout
    void print() const {
        if (root) printNode(root, "", true);
    }

private:
    /*
     * printNode() — recursively prints a node and its children.
     * prefix  — indentation string built up through recursion
     * isLast  — true if this is the last child of its parent,
     *           determines └── vs ├── and whether to extend with │
     */
    void printNode(const std::shared_ptr<ParseNode> &node,
                   const std::string &prefix,
                   bool isLast) const {
        std::cout << prefix
                  << (isLast ? "└── " : "├── ")
                  << node->label << "\n";

        // Extend prefix — blank for last child, vertical bar for others
        const std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < node->children.size(); ++i) {
            printNode(node->children[i], childPrefix,
                      i == node->children.size() - 1);
        }
    }
};