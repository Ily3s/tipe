#ifndef LEXER_H
#define LEXER_H

#include <array>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <optional>
#include <exception>

using namespace std;

extern string binop_hds;

enum tokent{
    LET,
    IF,
    THEN,
    ELSE,
    RETURN,
    OPERATOR,
    SEMICOL,
    EQUALS,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    NUM,
    ID,
    OPID
};

struct Token{
    tokent type;
    const char* lexeme = 0;
    struct {
        int line = 0, col = 0;
    } dbg_info;
    Token(tokent type, const char* lexeme = 0);
};

vector<Token> lex(const string& input);

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

class DFA{
    public :
        vector<array<int, 256>> transi;
        vector<bool> F;
        tokent tag;
        DFA(tokent type, string str);
        DFA(tokent type, std::function<void(DFA*)> constructor);
};

class NFA{
    private :
        // on choisi epsilon comme 257ème charactère
        vector<array<unordered_set<int>, 257>> transi;
        vector<bool> F;
        vector<tokent> tag;
        unordered_set<int> curr_states;
    public :
        NFA(const vector<DFA>& dfas);
        void next_state(char c);
        bool est_acceptant();
        tokent output();
        void reset();
        bool is_blocked();
};

class LexicalError : public runtime_error {
    using runtime_error::runtime_error;
};

#endif
