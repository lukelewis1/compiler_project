%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "ParseTree.h"

extern int  yylex();
extern int  yylineno;
extern int yycolumn;
extern FILE *yyin;

void yyerror(const char *s);

// Global parse tree root
std::shared_ptr<ParseNode> parseRoot;
int errorCount = 0;

// Node stack — used to pass nodes up through grammar rules
static std::vector<std::shared_ptr<ParseNode>> nodeStack;

static std::shared_ptr<ParseNode> makeNode(const std::string &label) {
    return std::make_shared<ParseNode>(label);
}

static void pushNode(std::shared_ptr<ParseNode> n) {
    nodeStack.push_back(n);
}

static std::shared_ptr<ParseNode> popNode() {
    if (nodeStack.empty()) return makeNode("?");
    auto n = nodeStack.back();
    nodeStack.pop_back();
    return n;
}
%}

%union {
    char *strval;
}

%token KW_CLASS KW_PUBLIC KW_PRIVATE KW_PROTECTED KW_NEW KW_DELETE
%token KW_INT KW_CHAR KW_IF KW_ELSE KW_WHILE KW_FOR KW_RETURN
%token <strval> IDENTIFIER
%token <strval> INTEGER_LITERAL
%token SEMICOLON COMMA COLON LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token PLUS MINUS STAR SLASH EQUALS LESSTHAN GREATERTHAN DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE
%left PLUS MINUS LESSTHAN GREATERTHAN
%left STAR SLASH

%%

Program:
    ClassDeclarationList {
        auto n = makeNode("Program");
        n->addChild(popNode());
        parseRoot = n;
    }
    ;

ClassDeclarationList:
    ClassDeclaration {
        auto c = popNode();
        auto n = makeNode("ClassDeclarationList");
        n->addChild(c);
        pushNode(n);
    }
    | ClassDeclaration ClassDeclarationList {
        auto list = popNode();
        auto c    = popNode();
        auto n    = makeNode("ClassDeclarationList");
        n->addChild(c);
        n->addChild(list);
        pushNode(n);
    }
    ;

ClassDeclaration:
    KW_CLASS IDENTIFIER LBRACE AccessSpecifierSectionList RBRACE SEMICOLON {
        auto sections = popNode();
        auto n = makeNode("ClassDeclaration");
        n->addChild(makeNode("class"));
        n->addChild(makeNode(std::string($2)));
        n->addChild(sections);
        pushNode(n);
        free($2);
    }
    ;

AccessSpecifierSectionList:
    AccessSpecifierSection {
        auto s = popNode();
        auto n = makeNode("AccessSpecifierSectionList");
        n->addChild(s);
        pushNode(n);
    }
    | AccessSpecifierSection AccessSpecifierSectionList {
        auto list = popNode();
        auto s    = popNode();
        auto n    = makeNode("AccessSpecifierSectionList");
        n->addChild(s);
        n->addChild(list);
        pushNode(n);
    }
    ;

AccessSpecifierSection:
    AccessSpecifier COLON MemberList {
        auto members = popNode();
        auto spec    = popNode();
        auto n = makeNode("AccessSpecifierSection");
        n->addChild(spec);
        n->addChild(members);
        pushNode(n);
    }
    ;

AccessSpecifier:
    KW_PUBLIC    { pushNode(makeNode("public")); }
    | KW_PRIVATE   { pushNode(makeNode("private")); }
    | KW_PROTECTED { pushNode(makeNode("protected")); }
    ;

MemberList:
    MemberDeclaration {
        auto m = popNode();
        auto n = makeNode("MemberList");
        n->addChild(m);
        pushNode(n);
    }
    | MemberDeclaration MemberList {
        auto list = popNode();
        auto m    = popNode();
        auto n    = makeNode("MemberList");
        n->addChild(m);
        n->addChild(list);
        pushNode(n);
    }
    ;

MemberDeclaration:
    VariableDeclaration {
        auto v = popNode();
        auto n = makeNode("MemberDeclaration");
        n->addChild(v);
        pushNode(n);
    }
    | FunctionDeclaration {
        auto f = popNode();
        auto n = makeNode("MemberDeclaration");
        n->addChild(f);
        pushNode(n);
    }
    | error SEMICOLON {
        pushNode(makeNode("[error recovered]"));
        fprintf(stderr, "Panic mode: skipping malformed member at line %d\n", yylineno);
    }
    ;

VariableDeclaration:
    Type IDENTIFIER SEMICOLON {
        auto t = popNode();
        auto n = makeNode("VariableDeclaration");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        pushNode(n);
        free($2);
    }
    | Type IDENTIFIER EQUALS Expression SEMICOLON {
        auto expr = popNode();
        auto t    = popNode();
        auto n    = makeNode("VariableDeclaration");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        n->addChild(makeNode("="));
        n->addChild(expr);
        pushNode(n);
        free($2);
    }
    ;

Type:
    KW_INT  { pushNode(makeNode("int")); }
    | KW_CHAR { pushNode(makeNode("char")); }
    ;

FunctionDeclaration:
    Type IDENTIFIER LPAREN ParameterList RPAREN LBRACE StatementList RBRACE {
        auto stmts  = popNode();
        auto params = popNode();
        auto t      = popNode();
        auto n = makeNode("FunctionDeclaration");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        n->addChild(params);
        n->addChild(stmts);
        pushNode(n);
        free($2);
    }
    | Type IDENTIFIER LPAREN RPAREN LBRACE StatementList RBRACE {
        auto stmts = popNode();
        auto t     = popNode();
        auto n = makeNode("FunctionDeclaration");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        n->addChild(makeNode("()"));
        n->addChild(stmts);
        pushNode(n);
        free($2);
    }
    ;

ParameterList:
    Parameter {
        auto p = popNode();
        auto n = makeNode("ParameterList");
        n->addChild(p);
        pushNode(n);
    }
    | Parameter COMMA ParameterList {
        auto list = popNode();
        auto p    = popNode();
        auto n    = makeNode("ParameterList");
        n->addChild(p);
        n->addChild(list);
        pushNode(n);
    }
    ;

Parameter:
    Type IDENTIFIER {
        auto t = popNode();
        auto n = makeNode("Parameter");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        pushNode(n);
        free($2);
    }
    ;

StatementList:
    Statement {
        auto s = popNode();
        auto n = makeNode("StatementList");
        n->addChild(s);
        pushNode(n);
    }
    | Statement StatementList {
        auto list = popNode();
        auto s    = popNode();
        auto n    = makeNode("StatementList");
        n->addChild(s);
        n->addChild(list);
        pushNode(n);
    }
    ;

Statement:
    VariableDeclaration  { /* node already on stack */ }
    | AssignmentStatement  { }
    | IfStatement          { }
    | WhileStatement       { }
    | ForStatement         { }
    | ReturnStatement      { }
    | NewStatement         { }
    | DeleteStatement      { }
    | ExpressionStatement  { }
    | error SEMICOLON {
        pushNode(makeNode("[error recovered]"));
        fprintf(stderr, "Panic mode: skipping to next ';' at line %d\n", yylineno);
    }
    ;

AssignmentStatement:
    IDENTIFIER EQUALS Expression SEMICOLON {
        auto expr = popNode();
        auto n = makeNode("AssignmentStatement");
        n->addChild(makeNode(std::string($1)));
        n->addChild(makeNode("="));
        n->addChild(expr);
        pushNode(n);
        free($1);
    }
    ;

IfStatement:
    KW_IF LPAREN Expression RPAREN LBRACE StatementList RBRACE %prec LOWER_THAN_ELSE {
        auto stmts = popNode();
        auto expr  = popNode();
        auto n = makeNode("IfStatement");
        n->addChild(makeNode("if"));
        n->addChild(expr);
        n->addChild(stmts);
        pushNode(n);
    }
    | KW_IF LPAREN Expression RPAREN LBRACE StatementList RBRACE KW_ELSE LBRACE StatementList RBRACE {
        auto elseStmts = popNode();
        auto ifStmts   = popNode();
        auto expr      = popNode();
        auto n = makeNode("IfStatement");
        n->addChild(makeNode("if"));
        n->addChild(expr);
        n->addChild(ifStmts);
        n->addChild(makeNode("else"));
        n->addChild(elseStmts);
        pushNode(n);
    }
    ;

WhileStatement:
    KW_WHILE LPAREN Expression RPAREN LBRACE StatementList RBRACE {
        auto stmts = popNode();
        auto expr  = popNode();
        auto n = makeNode("WhileStatement");
        n->addChild(makeNode("while"));
        n->addChild(expr);
        n->addChild(stmts);
        pushNode(n);
    }
    ;

ForStatement:
    KW_FOR LPAREN SEMICOLON Expression SEMICOLON Expression RPAREN LBRACE StatementList RBRACE {
        auto stmts    = popNode();
        auto stepExpr = popNode();
        auto condExpr = popNode();
        auto n = makeNode("ForStatement");
        n->addChild(makeNode("for"));
        n->addChild(makeNode(";"));
        n->addChild(condExpr);
        n->addChild(stepExpr);
        n->addChild(stmts);
        pushNode(n);
    }
    | KW_FOR LPAREN VariableDeclaration Expression SEMICOLON Expression RPAREN LBRACE StatementList RBRACE {
        auto stmts    = popNode();
        auto stepExpr = popNode();
        auto condExpr = popNode();
        auto init     = popNode();
        auto n = makeNode("ForStatement");
        n->addChild(makeNode("for"));
        n->addChild(init);
        n->addChild(condExpr);
        n->addChild(stepExpr);
        n->addChild(stmts);
        pushNode(n);
    }
    | KW_FOR LPAREN AssignmentStatement Expression SEMICOLON Expression RPAREN LBRACE StatementList RBRACE {
        auto stmts    = popNode();
        auto stepExpr = popNode();
        auto condExpr = popNode();
        auto init     = popNode();
        auto n = makeNode("ForStatement");
        n->addChild(makeNode("for"));
        n->addChild(init);
        n->addChild(condExpr);
        n->addChild(stepExpr);
        n->addChild(stmts);
        pushNode(n);
    }
    ;

ReturnStatement:
    KW_RETURN Expression SEMICOLON {
        auto expr = popNode();
        auto n = makeNode("ReturnStatement");
        n->addChild(makeNode("return"));
        n->addChild(expr);
        pushNode(n);
    }
    | KW_RETURN SEMICOLON {
        auto n = makeNode("ReturnStatement");
        n->addChild(makeNode("return"));
        pushNode(n);
    }
    ;

ExpressionStatement:
    Expression SEMICOLON {
        auto expr = popNode();
        auto n = makeNode("ExpressionStatement");
        n->addChild(expr);
        pushNode(n);
    }
    ;

NewStatement:
    Type IDENTIFIER EQUALS KW_NEW IDENTIFIER LPAREN ArgumentList RPAREN SEMICOLON {
        auto args = popNode();
        auto t    = popNode();
        auto n = makeNode("NewStatement");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        n->addChild(makeNode("new"));
        n->addChild(makeNode(std::string($5)));
        n->addChild(args);
        pushNode(n);
        free($2); free($5);
    }
    | Type IDENTIFIER EQUALS KW_NEW IDENTIFIER LPAREN RPAREN SEMICOLON {
        auto t = popNode();
        auto n = makeNode("NewStatement");
        n->addChild(t);
        n->addChild(makeNode(std::string($2)));
        n->addChild(makeNode("new"));
        n->addChild(makeNode(std::string($5)));
        pushNode(n);
        free($2); free($5);
    }
    ;

DeleteStatement:
    KW_DELETE IDENTIFIER SEMICOLON {
        auto n = makeNode("DeleteStatement");
        n->addChild(makeNode("delete"));
        n->addChild(makeNode(std::string($2)));
        pushNode(n);
        free($2);
    }
    ;

Expression:
    Expression PLUS Term {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Expression(+)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Expression MINUS Term {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Expression(-)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Expression LESSTHAN Term {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Expression(<)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Expression GREATERTHAN Term {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Expression(>)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Term { }
    ;

Term:
    Term STAR Factor {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Term(*)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Term SLASH Factor {
        auto r = popNode(); auto l = popNode();
        auto n = makeNode("Term(/)");
        n->addChild(l); n->addChild(r);
        pushNode(n);
    }
    | Factor { }
    ;

Factor:
    IDENTIFIER {
        pushNode(makeNode(std::string($1)));
        free($1);
    }
    | INTEGER_LITERAL {
        pushNode(makeNode(std::string($1)));
        free($1);
    }
    | LPAREN Expression RPAREN {
        auto expr = popNode();
        auto n = makeNode("( Expression )");
        n->addChild(expr);
        pushNode(n);
    }
    | IDENTIFIER DOT IDENTIFIER {
        auto n = makeNode("MemberAccess");
        n->addChild(makeNode(std::string($1)));
        n->addChild(makeNode(std::string($3)));
        pushNode(n);
        free($1); free($3);
    }
    ;

ArgumentList:
    Expression {
        auto e = popNode();
        auto n = makeNode("ArgumentList");
        n->addChild(e);
        pushNode(n);
    }
    | Expression COMMA ArgumentList {
        auto list = popNode();
        auto e    = popNode();
        auto n    = makeNode("ArgumentList");
        n->addChild(e);
        n->addChild(list);
        pushNode(n);
    }
    ;

%%

void yyerror(const char *s) {
    errorCount++;
    fprintf(stderr, "Syntax error at line %d col %d: %s\n", yylineno, yycolumn, s);
}
