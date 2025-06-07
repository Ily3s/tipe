#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>

#define TAPE_SIZE 80000

using namespace std;

struct Signature{
    string name;
    int left_arity, right_arity;
    bool operator==(const Signature& rhs) const;
};

template<> struct std::hash<Signature>{
    size_t operator()(const Signature& sign) const;
};

class Scope; 

struct Environement{
    unordered_map<string, int> adress_table;
    vector<string> stack_frame;
    int curr_addr;
    unordered_map<Signature, int> op_ids;
    int ops_nb = 0;
    Scope* curr_scope;
};

class SemanticError : public std::runtime_error{
    public :
        using std::runtime_error::runtime_error;
};

#endif
