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

extern string binop_hds;

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
    const char* name = 0;
    struct {
        int line = 0, col = 0;
    } dbg_info;
    Token(tokent t) : type(t) {}
    Token(tokent t, int v) : type(t), value(v) {}
};

vector<Token> lexer(string input);

template <typename T> struct maillon;
template <typename T>
class ConsList{
    public :
        maillon<T>* data = NULL;
        void push(const T& val);
        T& front();
        ~ConsList();
};

template <typename T>
struct maillon{
    T val;
    ConsList<T> next = NULL;
    maillon(const T& val, const ConsList<T>& next);
};

#endif
