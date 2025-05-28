#ifndef TIPE_H
#define TIPE_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>
#include <exception>

using namespace std;

// all variables below are defined in main.cpp :

extern void* symbol_table;
extern size_t symbol_nb;

extern string binop_hds;

extern unordered_map<string, int> ids_map;

enum tokent{
    LET,
    ID,
    EQUALS,
    OPERATOR,
    RETURN,
    OPID,
    LPAR,
    RPAR,
    NUM,
    SEMICOL
};

struct Token{
    tokent type;
    int value = -1;
    Token(tokent t) : type(t) {}
    Token(tokent t, int v) : type(t), value(v) {}
};

vector<Token> lexer(string input);

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
