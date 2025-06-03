#include "lexer.h"
#include "parser.h"
#include "codegen.h"

#include <cstdlib>
#include <fstream>
#include <cassert>
#include <iostream>

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

    vector<Token> tokens = lexer(input);
    parseTree tree = parser(tokens);
    AST ast = toAST(tree);

    Environement env;
    ofstream out{"out.asm"};
    ast.codegen(out, env);
    out.close();

    system("nasm -felf64 out.asm");
    system("ld out.o");

    return 0;
}
