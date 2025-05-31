#include "lexer.h"

#include <sstream>

vector<Token> lexer(string input)
{
    vector<Token> tokens;
    stringstream sstream{input};
    string current;
    while (!sstream.eof())
    {
        if(!(sstream >> current)) break;
        else if (current == "=") tokens.emplace_back(EQUALS);
        else if (current == "(") tokens.emplace_back(LPAR);
        else if (current == ")") tokens.emplace_back(RPAR);
        else if (current == ";") tokens.emplace_back(SEMICOL);
        else if (current == "let") tokens.emplace_back(LET);
        else if (current == "operator") tokens.emplace_back(OPERATOR);
        else if (current == "return") tokens.emplace_back(RETURN);
        else if (current[0] <= '9' && current[0] >= '0') {
            tokens.emplace_back(NUM, atoi(&current[0]));
        }
        else {
            tokent t = binop_hds.find(current[0]) == string::npos ? ID : OPID;
            auto it = ids_map.find(current);
            if (it == ids_map.end()) {
                ids_map.insert({current, symbol_nb});
                tokens.emplace_back(t, symbol_nb);
                symbol_nb++;
            }
            else {
                tokens.emplace_back(t, it->second);
            }
        }
    }
    return tokens;
}
