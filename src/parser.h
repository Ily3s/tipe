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
    EXPR_LIST,
    ACCESS
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

class parseTree{
    public :
    parseNode root;
    vector<parseTree> childs{};

    parseTree(parseNode, vector<parseTree>&& = {});
    parseTree(const parseTree&) = delete;
    parseTree& operator=(const parseTree&) = delete;
    parseTree(parseTree&&) noexcept = default;
    parseTree& operator=(parseTree&&) noexcept = default;

    inline void add_token(const Token& tok) {
        childs.push_back({parseNode{tok}});
    }
};

class SyntaxError : public std::runtime_error{
    public :
        optional<parseNode> expected;
        SyntaxError(const char* str, optional<parseNode> expected = nullopt);
};

parseTree parse(const vector<Token>& tokens);

#endif
