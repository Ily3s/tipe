#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <unordered_map>
#include <string>

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


#endif
