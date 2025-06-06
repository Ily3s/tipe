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
    ID,
    EQUALS,
    OPERATOR,
    RETURN,
    OPID,
    LPAR,
    RPAR,
    NUM,
    SEMICOL,
    IF,
    THEN,
    ELSE,
    LBRACKET,
    RBRACKET
};

struct Token{
    tokent type;
    const char* lexeme = 0;
    struct {
        int line = 0, col = 0;
    } dbg_info;
    Token(tokent type);
    Token(tokent type, const char* lexeme);
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
        tokent type;
        int offset = 0;
        int next(int state, char c);
        DFA(tokent type, string str);
        DFA(tokent type, std::function<void(DFA*)> constructor);
};

// NFA désigne ici un cas (très) particulier de NFA
class NFA{
    public :
        vector<DFA*> dfas;
        unordered_set<int> curr_states;
        void next_state(char c);
        bool est_acceptant();
        tokent premier_acceptant();
        NFA(vector<DFA*> dfas);
        void reset();
    private :
        void parcours_etats(function<void(DFA*, int)> f);
};

class LexicalError : public runtime_error {
    using runtime_error::runtime_error;
};

#endif
