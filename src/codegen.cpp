#include "codegen.h"
#include "ast.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <cassert>
#include <unordered_map>

void AST::codegen(ofstream& out, Environement& env) {
    out << "section .text\n\n"
            "global _start\n\n";
    for (auto& op : ops)
        op->codegen(out, env);
}

void OpDef::codegen(ofstream& out, Environement& env) {
    init_scope(out, env);
    Signature sign = {op.lexeme, (int)lhs_args.size(), (int)rhs_args.size()};
    auto [_, success] = env.op_ids.insert({sign, env.ops_nb++});
    if (!success) {
        stringstream error_msg;
        error_msg << "operator \"" << op.lexeme << "\" is redefined here";
        throw SemanticError(error_msg.str());
    }
    const static Signature main_sign = {":main", 0, 0};
    if (sign == main_sign)
        out << "_start:\n"
            << "\tpush r15\n"
            << "\tsub rsp, " << TAPE_SIZE << '\n'
            << "\tmov r15, rsp\n";
    else
        out << "op" << env.ops_nb-1 << ":\n";
    out << "\tpush rbp\n"
        << "\tmov rbp, rsp\n";
    env.curr_addr = -8;

    int offset = (lhs_args.size() + rhs_args.size()-1)*8+16;
    for (auto& arg : lhs_args) {
        arg->offset = offset;
        arg->define_in_scope(env);
        arg->codegen(out, env);
        offset -= 8;
    }
    for (auto& arg : rhs_args) {
        arg->offset = offset;
        arg->define_in_scope(env);
        arg->codegen(out, env);
        offset -= 8;
    }

    for (auto& statement : statements)
        statement->codegen(out, env);

    del_scope(out, env);

    out << "\tpop rbp\n";
    if (sign == main_sign)
        out << "\tadd rsp, " << TAPE_SIZE << '\n'
            << "\tpop r15\n"
            << "\tmov rdi, rax\n"
            << "\tmov rax, 60\n"
            << "\tsyscall\n\n";
    else
        out << "\tret\n\n";
}

void Scope::init_scope(ofstream& out, Environement& env) {
    env.curr_scope = this;
}

void Scope::del_scope(ofstream& out, Environement& env) {
    out << "\tadd rsp, " << bytes_owned << "\n";
    for (int i = 0; i < int_def_nb; i++) {
        auto it = env.adress_table.find(env.stack_frame.back());
        assert(it != env.adress_table.end());
        env.adress_table.erase(it);
        env.stack_frame.pop_back();
    }
}

void Var::define_in_scope(Environement& env) {
    auto [_, success] = env.adress_table.insert({id.lexeme, offset});
    if (!success) {
        stringstream err;
        err << "identifier \"" << id.lexeme << "\" is redefined here";
        throw SemanticError(err.str());
    }
    env.stack_frame.push_back(id.lexeme);
    env.curr_scope->int_def_nb++;
}

void Var::codegen(ofstream&, Environement& env) {
    return;
}

void Define::codegen(ofstream& out, Environement& env) {
    lval->offset = env.curr_addr;
    lval->define_in_scope(env);
    lval->codegen(out, env);
    expr->codegen(out, env);
    out << "\tpush rax\n";
    env.curr_scope->bytes_owned += 8;
    env.curr_addr -= 8;
}

void RvalToken::codegen(ofstream& out, Environement& env) {
    if (id.type == NUM)
        out << "\tmov rax, " << atoll(id.lexeme) << '\n';
    else {
        auto it = env.adress_table.find(id.lexeme);
        if (it == env.adress_table.end()) {
            stringstream err;
            err << "identifier \"" << id.lexeme << "\" is used without being defined here";
            throw SemanticError(err.str());
        }
        out << "\tmov rax, QWORD [rbp+" << it->second << "]\n";
    }
}

void RvalAccess::codegen(ofstream& out, Environement& env) {
    index->codegen(out, env);
    out << "\tadd rax, r15\n"
        << "\tmov rsi, rax\n"
        << "\txor rax, rax\n" // clear rax bcz of int promotion
        << "\tmov al, BYTE [rsi]\n";
}

unordered_map<string, string> prelude_binops;

void OpApply::codegen(ofstream& out, Environement& env) {
    Signature sign = {op.lexeme, (int)lhs.size(), (int)rhs.size()};
    prelude_binops.insert({string("+"), "add"});
    prelude_binops.insert({string("-"), "sub"});
    prelude_binops.insert({string("*"), "imul"});
    prelude_binops.insert({string("/"), "idiv"});
    auto it1 = prelude_binops.find(op.lexeme);
    if (sign.left_arity == 1 && sign.right_arity == 1 && it1 != prelude_binops.end()) {
        rhs[0]->codegen(out, env);
        out << "\tpush rax\n";
        lhs[0]->codegen(out, env);
        out << "\tpop rsi\n";
        if (op.lexeme == string("/"))
            out << "\txor rdx, rdx\n"
                << "\tidiv rsi\n";
        else
            out << "\t" << it1->second << " rax, rsi\n";
        return;
    }
    for (auto& l_arg : lhs) {
        l_arg->codegen(out, env);
        out << "\tpush rax\n";
    }
    for (auto& r_arg : rhs) {
        r_arg->codegen(out, env);
        out << "\tpush rax\n";
    }
    if ((op.lexeme == string(":print") || op.lexeme == string(":read"))
            && sign.left_arity == 0 && sign.right_arity == 2) {
        out << "\tmov rax, " << (op.lexeme == string(":print")) << "\n"
            << "\tmov rdi, 1\n"
            << "\tpop rdx\n"
            << "\tpop rsi\n"
            << "\tadd rsi, r15\n"
            << "\tsyscall\n";
        return;
    }
    auto it = env.op_ids.find(sign);
    if (it == env.op_ids.end()) {
        stringstream err;
        err << "operator \"" << op.lexeme << "\" with these arguments is used without being defined here";
        throw SemanticError(err.str());
    }
    out << "\tcall op" << it->second << '\n';
    out << "\tadd rsp, " << (lhs.size()+rhs.size())*8 << '\n';
}

void Return::codegen(ofstream& out, Environement& env) {
    expr->codegen(out, env);
}

int IfStatement::branch_count = 0;

void IfStatement::codegen(ofstream& out, Environement& env) {
    cond->codegen(out, env);
    int branchfalse = branch_count;
    out << "\tcmp rax, 0\n"
        << "\tje branch" << branch_count++ << '\n';
    expr_true->codegen(out, env);
    int branchtrue = branch_count;
    out << "\tjmp branch" << branch_count++ << '\n';
    out << "branch" << branchfalse << ":\n";
    expr_false->codegen(out, env);
    out << "branch" << branchtrue << ":\n";
}

void LvalAccess::codegen(ofstream& out, Environement& env) {
    index->codegen(out, env);
    out << "\tadd rax, r15\n";
}

string Var::get_name(Environement& env) {
    auto it = env.adress_table.find(id.lexeme);
    if (it == env.adress_table.end()) {
        stringstream err;
        err << "identifier \"" << id.lexeme << "\" is used without being defined here";
        throw SemanticError(err.str());
    }
    stringstream res;
    res << "QWORD [rbp+" << it->second << "]";
    return res.str();
}

string LvalAccess::get_name(Environement& env) {
    return "BYTE [rax]";
}

int Var::size() { return 8; }
int LvalAccess::size() { return 1; }

void Assign::codegen(ofstream& out, Environement& env) {
    string size_to_str[9];
    size_to_str[8] = "";
    size_to_str[4] = "D"; // dword
    size_to_str[2] = "W"; // word
    size_to_str[1] = "B"; // byte
    expr->codegen(out, env);
    out << "\tpush rax\n";
    lval->codegen(out, env);
    out << "\tpop r8\n";
    out << "\tmov "<< lval->get_name(env) << ", r8" << size_to_str[lval->size()] << "\n";
}

void FuncCall::codegen(ofstream& out, Environement& env) {
    expr->codegen(out, env);
}

template <class T>
inline void hash_combine(std::size_t & s, const T & v)
{
  std::hash<T> h;
  s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

size_t std::hash<Signature>::operator()(const Signature& sign) const {
    size_t res = 0;
    hash_combine(res, sign.name);
    hash_combine(res, sign.left_arity);
    hash_combine(res, sign.right_arity);
    return res;
}

bool Signature::operator==(const Signature& rhs) const {
    return name == rhs.name && left_arity == rhs.left_arity && right_arity == rhs.right_arity;
}
