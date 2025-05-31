#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>

enum nonTerm{
    START,
    OP_BLOCK,
    ID_LIST,
    STAT_LIST,
    STATEMENT,
    EXPR,
    EXPR_LIST
};

class parseNode{
    public :
    union {
        nonTerm nt; Token tok;
    } val;
    enum {
        NONTERM, TOKEN
    } tag;
    public :
    parseNode(nonTerm nt);
    parseNode(Token tok);
};

struct parseTree{
    parseNode root;
    vector<parseTree> childs{};
    inline void add_token(const Token& tok) {
        childs.push_back({parseNode{tok}});
    }
};

class SyntaxError : public std::runtime_error{
    public :
    optional<parseNode> expected;
    SyntaxError(const char* str, optional<parseNode> expected = nullopt);
};

parseTree parser(const vector<Token>& tokens);




class ASTNode {

};

class ASTLvalue : public ASTNode {
    public :
    Token id;
};

class ASTStatement : public ASTNode {

};

class ASTOpDef : public ASTNode {
    public :
    Token op;
    vector<unique_ptr<ASTLvalue>> lhs_args, rhs_args;
    vector<unique_ptr<ASTStatement>> statements;
};

class ASTExpr : public ASTNode {

};

class ASTRvalueVar : public ASTExpr {
    public :
    Token id;
};

class ASTOpApply : public ASTExpr {
    public :
    Token op;
    vector<unique_ptr<ASTExpr>> lhs, rhs;
};

class ASTDefine : public ASTStatement {
    public :
    unique_ptr<ASTLvalue> lval;
    unique_ptr<ASTExpr> expr;
};

class ASTAssign : public ASTStatement {
    public :
    unique_ptr<ASTLvalue> lval;
    unique_ptr<ASTExpr> expr;
};

class ASTReturn : public ASTStatement {
    public :
    unique_ptr<ASTExpr> expr;
};

class AST : public ASTNode {
    public :
    vector<unique_ptr<ASTOpDef>> ops;
};

AST toAST(const parseTree&);

#endif
