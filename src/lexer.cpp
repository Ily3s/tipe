#include "lexer.h"

#include <sstream>
#include <cassert>
#include <unordered_map>

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

ConsList<string> lexemes;

string binop_hds = "*+-/\\><={}^!:%.@?|&";

vector<Token> lexer(string input)
{
    vector<Token> tokens;
    stringstream sstream{input};
    string current;
    string line;
    unordered_map<string, tokent> keywords{
        {"=", EQUALS}, {"(", LPAR}, {")", RPAR},
        {"[", LBRACKET}, {"]", RBRACKET},
        {"let", LET}, {";", SEMICOL},
        {"operator", OPERATOR}, {"return", RETURN},
        {"if", IF}, {"then", THEN}, {"else", ELSE}
    };
    for (int i = 0; getline(sstream, line); i++)
    {
        stringstream sstream{line};
        while (!sstream.eof()) {
            if(!(sstream >> current)) break;
            auto it = keywords.find(current);
            if (it != keywords.end())
                tokens.emplace_back(it->second);
            else if (current[0] <= '9' && current[0] >= '0') {
                tokens.emplace_back(NUM, atoll(&current[0]));
            }
            else {
                tokent t = binop_hds.find(current[0]) == string::npos ? ID : OPID;
                tokens.emplace_back(t);
                lexemes.push(current);
                tokens.back().name = lexemes.front().c_str();
            }
            tokens.back().dbg_info.line = i+1;
            tokens.back().dbg_info.col = (size_t)sstream.tellg() - current.size() + 1;
        }
    }
    return tokens;
}
