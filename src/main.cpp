#include "tipe.h"

#include <fstream>
#include <cassert>
#include <iostream>

void* symbol_table;
size_t symbol_nb = 0;

string binop_hds = "*+-/\\><={}[]^!:%.";

unordered_map<string, int> ids_map;

int main(int argc, char** argv)
{
    assert(argc == 2);

    ifstream file{argv[1]};
    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);
    string input(size,0);
    file.read(&input[0], size);
    file.close();

    cout << input << endl;

    vector<Token> tokens = lexer(input);
    parseTree tree = parser(tokens);

    return 0;
}
