#include "lexer.h"

#include <algorithm>
#include <cwctype>
#include <sstream>
#include <cassert>
#include <unordered_set>
#include <cctype>
#include <set>

string op_hds = "!#$%&\'\"*+,-./:<=>?@\\^`{|}~";

bool isnum(char c) {
    return c >= '0' && c <= '9';
}

DFA _LET(LET, "let");
DFA _ID(ID, [](DFA* dfa) {
    dfa->F = {0, 1};
    dfa->transi.resize(2);
    for (int c = 0; c < 255; c++) {
        if (isalpha(c) || c == '_')
            dfa->transi[0][c] = dfa->transi[1][c] = 1;
        else if (isnum(c)) {
            dfa->transi[1][c] = 1;
            dfa->transi[0][c] = -1;
        }
        else
            dfa->transi[0][c] = dfa->transi[1][c] = -1;
    }
});
DFA _EQUALS(EQUALS, "=");
DFA _OPERATOR(OPERATOR, "operator");
DFA _RETURN(RETURN, "return");
DFA _OPID(OPID, [](DFA* dfa) {
    dfa->F = {0, 1};
    dfa->transi.resize(2);
    for (int c = 0; c < 255; c++) {
        dfa->transi[0][c] = dfa->transi[1][c] = -1;
        if (op_hds.find(c) != string::npos)
            dfa->transi[0][c] = dfa->transi[1][c] = 1;
        if (isalnum(c) || c == '_')
            dfa->transi[1][c] = 1;
    }
});
DFA _LPAR(LPAR, "(");
DFA _RPAR(RPAR, ")");
DFA _NUM(NUM, [](DFA* dfa) {
    dfa->F = {0, 1};
    dfa->transi.resize(2);
    for (int c = 0; c < 255; c++)
        dfa->transi[0][c] = dfa->transi[1][c] = isnum(c) ? 1 : -1;
});
DFA _SEMICOL(SEMICOL, ";");
DFA _IF(IF, "if");
DFA _THEN(THEN, "then");
DFA _ELSE(ELSE, "else");
DFA _LBRACKET(LBRACKET, "[");
DFA _RBRACKET(RBRACKET, "]");

NFA automata({_LET, _EQUALS, _OPERATOR, _RETURN, _LPAR,
                _RPAR, _SEMICOL, _IF, _THEN, _ELSE,
                _LBRACKET, _RBRACKET, _NUM, _OPID, _ID});

Token::Token(tokent type, const char* lexeme) : type(type), lexeme(lexeme) {}

template <typename T>
maillon<T>::maillon(const T& val, const ConsList<T>& next)
    : val(val), next(next) {}

template <typename T>
void ConsList<T>::push(const T& val) {
    data = new maillon(val, *this);
}

template <typename T>
T& ConsList<T>::front() {
    assert(data);
    return data->val;
}

template <typename T>
ConsList<T>::~ConsList() {
    if (data) delete data;
}

// ConsList nécessaire (et pas vector) pour assurer
// que les char* qui pointent vers un lexeme restent
// valables
ConsList<string> lexemes;

vector<Token> lex(const string& input)
{
    vector<Token> tokens;
    int line = 1, col = 1;
    int curr = 0, last_accept = -1, forward = -1;
    while (curr < input.size()) {
        if (iswspace(input[curr])) {
            if (input[curr] == '\n') {
                line++;
                col = 1;
            } else col++;
            curr++;
            continue;
        }
        forward = curr-1;
        tokent tag;
        while (!automata.is_blocked() && forward < (int)input.size()) {
            forward++;
            automata.next_state(input[forward]);
            if (automata.est_acceptant()) {
                last_accept = forward;
                tag = automata.output();
            }
        }
        if (last_accept < curr) {
            stringstream err_msg;
            err_msg << "lexeme at line " << line << ", column " << col << " is not recognized";
            throw LexicalError(err_msg.str());
        }
        lexemes.push({input.begin()+curr, input.begin()+last_accept+1});
        tokens.emplace_back(tag, lexemes.front().c_str());
        tokens.back().dbg_info = {.line = line, .col = col};
        automata.reset();
        col += lexemes.front().size();
        curr = last_accept+1;
    }
    return tokens;
}

DFA::DFA(tokent tag, function<void(DFA*)> constructor)
    : tag(tag)
{
    constructor(this);
}

DFA::DFA(tokent tag, string str)
    : tag(tag)
{
    if (!str.size()) return;
    for (char c : str) {
        F.push_back(0);
        transi.push_back({});
        for (int i = 0; i < 256; i++)
            transi.back()[i] = -1;
        transi.back()[c] = transi.size();
    }
    F.push_back(1);
    transi.push_back({});
    for (int i = 0; i < 256; i++)
        transi.back()[i] = -1;
}

NFA::NFA(const vector<DFA>& dfas)
{
    int offset = 1;
    F.push_back(0);
    transi.emplace_back();
    for (const DFA& dfa : dfas) {
        int n = dfa.F.size();
        F.insert(F.end(), dfa.F.begin(), dfa.F.end());
        transi.resize(transi.size()+n);
        tag.resize(transi.size()+n);
        for (int i = 0; i < n; i++) {
            for (int c = 0; c < 256; c++) {
                if (dfa.transi[i][c] != -1)
                    transi[offset+i][c].insert(dfa.transi[i][c]+offset);
            }
            tag[offset+i] = dfa.tag;
        }
        transi[0][256].insert(offset);
        offset += n;
    }
    reset();
}

void NFA::reset() {
    unordered_set<int> init_states;
    init_states.insert(0);
    int size;
    do {
        size = init_states.size();
        for (int state : init_states) {
            for (int next : transi[state][256])
                init_states.insert(next);
        }
    } while (size != init_states.size());
    curr_states = init_states;
}

void NFA::next_state(char c) {
    unordered_set<int> new_states;
    for (int state : curr_states) {
        for (int next : transi[state][c])
            new_states.insert(next);
    }
    int size;
    do {
        size = new_states.size();
        for (int state : new_states) {
            for (int next : transi[state][256])
                new_states.insert(next);
        }
    } while (size != new_states.size());
    curr_states = new_states;
}

bool NFA::est_acceptant() {
    for (int state : curr_states) {
        if (F[state]) return true;
    }
    return false;
}

// précondition : est_acceptant() doit être vrai
tokent NFA::output() {
    int min_state = transi.size();
    for (int state : curr_states) {
        if (state < min_state && F[state])
            min_state = state;
    }
    assert(min_state < transi.size() && "précondition non respecté : est_acceptant()");
    return tag[min_state];
}

bool NFA::is_blocked() {
    return !curr_states.size();
}
