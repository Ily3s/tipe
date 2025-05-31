#ifndef LEXER_H
#define LEXER_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>
#include <exception>

using namespace std;

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

#endif
