#include "lexer.h"

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
DFA _ID(ID, [](DFA* ceci) {
    ceci->F = {0, 1};
    ceci->transi.resize(2);
    for (int c = 0; c < 255; c++) {
        if (isalpha(c) || c == '_')
            ceci->transi[0][c] = ceci->transi[1][c] = 1;
        else if (isnum(c)) {
            ceci->transi[1][c] = 1;
            ceci->transi[0][c] = -1;
        }
        else
            ceci->transi[0][c] = ceci->transi[1][c] = -1;
    }
});
DFA _EQUALS(EQUALS, "=");
DFA _OPERATOR(OPERATOR, "operator");
DFA _RETURN(RETURN, "return");
DFA _OPID(OPID, [](DFA* ceci) {
    ceci->F = {0, 1};
    ceci->transi.resize(2);
    for (int c = 0; c < 255; c++) {
        ceci->transi[0][c] = ceci->transi[1][c] = -1;
        if (op_hds.find(c) != string::npos)
            ceci->transi[0][c] = ceci->transi[1][c] = 1;
        if (isalnum(c) || c == '_')
            ceci->transi[1][c] = 1;
    }
});
DFA _LPAR(LPAR, "(");
DFA _RPAR(RPAR, ")");
DFA _NUM(NUM, [](DFA* ceci) {
    ceci->F = {0, 1};
    ceci->transi.resize(2);
    for (int c = 0; c < 255; c++)
        ceci->transi[0][c] = ceci->transi[1][c] = isnum(c) ? 1 : -1;
});
DFA _SEMICOL(SEMICOL, ";");
DFA _IF(IF, "if");
DFA _THEN(THEN, "then");
DFA _ELSE(ELSE, "else");
DFA _LBRACKET(LBRACKET, "[");
DFA _RBRACKET(RBRACKET, "]");

NFA automaton({&_LET, &_EQUALS, &_OPERATOR, &_RETURN, &_LPAR,
                &_RPAR, &_SEMICOL, &_IF, &_THEN, &_ELSE,
                &_LBRACKET, &_RBRACKET, &_NUM, &_OPID, &_ID});

Token::Token(tokent type) : type(type) {}
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
        tokent type;
        while (automaton.curr_states.size() && forward < (int)input.size()) {
            forward++;
            automaton.next_state(input[forward]);
            if (automaton.est_acceptant()) {
                last_accept = forward;
                type = automaton.premier_acceptant();
            }
        }
        if (last_accept < curr) {
            stringstream err_msg;
            err_msg << "lexeme at line " << line << ", column " << col << " is not recognized";
            throw LexicalError(err_msg.str());
        }
        lexemes.push({input.begin()+curr, input.begin()+last_accept+1});
        tokens.emplace_back(type, lexemes.front().c_str());
        tokens.back().dbg_info = {.line = line, .col = col};
        automaton.reset();
        col += lexemes.front().size();
        curr = last_accept+1;
    }
    return tokens;
}

DFA::DFA(tokent type, function<void(DFA*)> constructor)
    : type(type)
{
    constructor(this);
}

DFA::DFA(tokent type, string str)
    : type(type)
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

int DFA::next(int state, char c) {
    assert(transi[state][c] != -1);
    return transi[state][c]+offset;
}

NFA::NFA(vector<DFA*> dfas)
    : dfas(dfas)
{
    int curr_offset = 0;
    for (DFA* dfa : dfas) {
        dfa->offset = curr_offset;
        curr_states.insert(dfa->offset);
        curr_offset += dfa->transi.size();
    }
}

void NFA::reset() {
    unordered_set<int> init_states;
    for (DFA* dfa : dfas)
        init_states.insert(dfa->offset);
    curr_states = init_states;
}

void NFA::parcours_etats(function<void(DFA*, int)> f) {
    for (int state : curr_states) {
        for (DFA* dfa : dfas) {
            int r_state = state - dfa->offset;
            if (r_state < 0 || r_state >= dfa->transi.size())
                continue;
            f(dfa, r_state);
        }
    }
}

void NFA::next_state(char c) {
    unordered_set<int> new_states;
    parcours_etats([&new_states, c](DFA* dfa, int state) {
            if (dfa->transi[state][c] != -1)
                new_states.insert(dfa->next(state, c));
    });
    curr_states = new_states;
}

bool NFA::est_acceptant() {
    bool res = false;
    parcours_etats([&res](DFA* dfa, int state) {
            res |= dfa->F[state];
    });
    return res;
}

// précondition : est_acceptant() doit être vrai
tokent NFA::premier_acceptant() {
    set<int> sorted_states;
    for (int state : curr_states)
        sorted_states.insert(state);
    int dfa_i = 0;
    for (int state : sorted_states) {
        while (state >= dfas[dfa_i]->offset + dfas[dfa_i]->transi.size())
            dfa_i++;
        if (dfas[dfa_i]->F[state-dfas[dfa_i]->offset])
            return dfas[dfa_i]->type;
    }
    assert("précondition non respecté : est_acceptant()");
    return tokent{};
}
