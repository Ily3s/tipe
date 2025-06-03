#include "codegen.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <cassert>
#include <unordered_map>

ASTOpDef::ASTOpDef(const Token& op,
        vector<unique_ptr<ASTLvalue>>&& lhs_args,
        vector<unique_ptr<ASTLvalue>>&& rhs_args,
        vector<unique_ptr<ASTStatement>>&& statements)
    : op(op), lhs_args(std::move(lhs_args)), rhs_args(std::move(rhs_args)), statements(std::move(statements)) {}

    ASTRvalue::ASTRvalue(const Token& id) : id(id) {}

ASTOpApply::ASTOpApply(const Token& op,
        vector<unique_ptr<ASTExpr>>&& lhs,
        vector<unique_ptr<ASTExpr>>&& rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

ASTDefine::ASTDefine(unique_ptr<ASTLvalue>&& lval, unique_ptr<ASTExpr>&& expr)
    : lval(std::move(lval)), expr(std::move(expr)) {}

ASTAssign::ASTAssign(unique_ptr<ASTLvalue>&& lval, unique_ptr<ASTExpr>&& expr)
    : lval(std::move(lval)), expr(std::move(expr)) {}

ASTReturn::ASTReturn(unique_ptr<ASTExpr>&& expr)
    : expr(std::move(expr)) {}

AST::AST(vector<unique_ptr<ASTOpDef>>&& ops)
    : ops(std::move(ops)) {}

ASTLvalue::ASTLvalue(const Token& id)
    : id(id) {}

ASTIfStatement::ASTIfStatement(unique_ptr<ASTExpr>&& cond,
        unique_ptr<ASTExpr>&& expr_true,
        unique_ptr<ASTExpr>&& expr_false)
    : cond(std::move(cond)), expr_true(std::move(expr_true)), expr_false(std::move(expr_false)) {}

#define DEF_toAST(V) unique_ptr<AST##V> toAST##V(const parseTree&)
    DEF_toAST(Lvalue); DEF_toAST(Rvalue); DEF_toAST(Statement); DEF_toAST(OpDef);
    DEF_toAST(Expr); DEF_toAST(OpApply); DEF_toAST(Define); DEF_toAST(Assign); DEF_toAST(Return);
    DEF_toAST(IfStatement);

#define DEF_listToAST(V) \
        vector<unique_ptr<AST##V>> listToAST##V(const parseTree& tree) { \
            vector<unique_ptr<AST##V>> res; \
            const parseTree* curr = &tree; \
            while (curr->childs.size()) { \
                res.emplace_back(toAST##V(curr->childs[0])); \
                curr = &curr->childs[1]; \
            } \
            return res; \
        }
    DEF_listToAST(Expr); DEF_listToAST(Statement);
    DEF_listToAST(Lvalue); DEF_listToAST(OpDef);

AST toAST(const parseTree& tree) {
    return {listToASTOpDef(tree)};
}

unique_ptr<ASTOpDef> toASTOpDef(const parseTree& tree) {
    return make_unique<ASTOpDef>(ASTOpDef{
            tree.childs[3].root.val.tok,
            listToASTLvalue(tree.childs[2]),
            listToASTLvalue(tree.childs[4]),
            listToASTStatement(tree.childs[6])
            });
}

unique_ptr<ASTLvalue> toASTLvalue(const parseTree& tree) {
    return make_unique<ASTLvalue>(ASTLvalue{ tree.root.val.tok});
}

unique_ptr<ASTRvalue> toASTRvalue(const parseTree& tree) {
    return make_unique<ASTRvalue>(ASTRvalue{ tree.root.val.tok});
}

unique_ptr<ASTStatement> toASTStatement(const parseTree& tree) {
    if (tree.childs.size() == 3) return toASTReturn(tree);
    if (tree.childs.size() == 4) return toASTAssign(tree);
    if (tree.childs.size() == 5) return toASTDefine(tree);
    return 0;
}

unique_ptr<ASTReturn> toASTReturn(const parseTree& tree) {
    return make_unique<ASTReturn>(ASTReturn{ toASTExpr(tree.childs[1])});
}

unique_ptr<ASTAssign> toASTAssign(const parseTree& tree) {
    return make_unique<ASTAssign>(ASTAssign{
            toASTLvalue(tree.childs[0]),
            toASTExpr(tree.childs[2])
            });
}

unique_ptr<ASTDefine> toASTDefine(const parseTree& tree) {
    return make_unique<ASTDefine>(ASTDefine{
            toASTLvalue(tree.childs[1]),
            toASTExpr(tree.childs[3])
            });
}

unique_ptr<ASTIfStatement> toASTIfStatement(const parseTree& tree) {
    return make_unique<ASTIfStatement>(ASTIfStatement{
            toASTExpr(tree.childs[1]),
            toASTExpr(tree.childs[3]),
            toASTExpr(tree.childs[5])
            });
}

unique_ptr<ASTExpr> toASTExpr(const parseTree& tree) {
    if (tree.childs.size() == 1) return toASTRvalue(tree.childs[0]);
    if (tree.childs.size() == 5) return toASTOpApply(tree);
    if (tree.childs.size() == 6) return toASTIfStatement(tree);
    return 0;
}

unique_ptr<ASTOpApply> toASTOpApply(const parseTree& tree) {
    return make_unique<ASTOpApply>(ASTOpApply{
            tree.childs[2].root.val.tok,
            listToASTExpr(tree.childs[1]),
            listToASTExpr(tree.childs[3])
            });
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

void AST::codegen(ofstream& out, Environement& env) {
    out << "section .text\n\n"
            "global _start\n\n";
    for (auto& op : ops)
        op->codegen(out, env);
}

void ASTOpDef::codegen(ofstream& out, Environement& env) {
    init_scope(out, env);
    Signature sign = {op.name, (int)lhs_args.size(), (int)rhs_args.size()};
    auto [_, success] = env.op_ids.insert({sign, env.ops_nb++});
    if (!success) {
        stringstream error_msg;
        error_msg << "operator \"" << op.name << "\" is redefined here";
        throw SemanticError(error_msg.str());
    }
    const static Signature main_sign = {":main", 0, 0};
    if (sign == main_sign)
        out << "_start:\n";
    else
        out << "op" << env.ops_nb-1 << ":\n";
    out << "\tpush rbp\n"
        << "\tmov rbp, rsp\n";
    env.curr_addr = -8;

    int offset = (lhs_args.size() + rhs_args.size()-1)*8+16;
    for (auto& arg : lhs_args) {
        arg->offset = offset;
        arg->codegen(out, env);
        offset -= 8;
    }
    for (auto& arg : rhs_args) {
        arg->offset = offset;
        arg->codegen(out, env);
        offset -= 8;
    }

    for (auto& statement : statements)
        statement->codegen(out, env);

    del_scope(out, env);

    out << "\tpop rbp\n";
    if (sign == main_sign)
        out << "\tmov rdi, rax\n"
            << "\tmov rax, 60\n"
            << "\tsyscall\n\n";
    else
        out << "\tret\n\n";
}

void ASTScope::init_scope(ofstream& out, Environement& env) {
    env.curr_scope = this;
}

void ASTScope::del_scope(ofstream& out, Environement& env) {
    out << "\tadd rsp, " << bytes_owned << "\n";
    for (int i = 0; i < int_def_nb; i++) {
        auto it = env.adress_table.find(env.stack_frame.back());
        assert(it != env.adress_table.end());
        env.adress_table.erase(it);
        env.stack_frame.pop_back();
    }
}

void ASTLvalue::codegen(ofstream& out, Environement& env) {
    auto [_, success] = env.adress_table.insert({id.name, offset});
    if (!success) {
        stringstream err;
        err << "identifier \"" << id.name << "\" is redefined here";
        throw SemanticError(err.str());
    }
    env.stack_frame.push_back(id.name);
    env.curr_scope->int_def_nb++;
}

void ASTDefine::codegen(ofstream& out, Environement& env) {
    lval->offset = env.curr_addr;
    lval->codegen(out, env);
    expr->codegen(out, env);
    out << "\tpush rax\n";
    env.curr_scope->bytes_owned += 8;
    env.curr_addr -= 8;
}

void ASTRvalue::codegen(ofstream& out, Environement& env) {
    if (id.type == NUM)
        out << "\tmov rax, " << id.value << '\n';
    else {
        auto it = env.adress_table.find(id.name);
        if (it == env.adress_table.end()) {
            stringstream err;
            err << "identifier \"" << id.name << "\" is used without being defined here";
            throw SemanticError(err.str());
        }
        out << "\tmov rax, QWORD [rbp+" << it->second << "]\n";
    }
}

unordered_map<string, string> prelude_binops;

void ASTOpApply::codegen(ofstream& out, Environement& env) {
    Signature sign = {op.name, (int)lhs.size(), (int)rhs.size()};
    Signature negate_sign = {string("-"), 0, 1};
    prelude_binops.insert({string("+"), "add"});
    prelude_binops.insert({string("-"), "sub"});
    prelude_binops.insert({string("*"), "imul"});
    prelude_binops.insert({string("/"), "idiv"});
    auto it1 = prelude_binops.find(op.name);
    if (sign == negate_sign) {
        rhs[0]->codegen(out, env);
        out << "\tneg rax\n";
        return;
    } else if (sign.left_arity == 1 && sign.right_arity == 1 && it1 != prelude_binops.end()) {
        rhs[0]->codegen(out, env);
        out << "\tpush rax\n";
        lhs[0]->codegen(out, env);
        out << "\tpop rsi\n";
        if (op.name == string("/"))
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
    auto it = env.op_ids.find(sign);
    if (it == env.op_ids.end()) {
        stringstream err;
        err << "operator \"" << op.name << "\" with these arguments is used without being defined here";
        throw SemanticError(err.str());
    }
    out << "\tcall op" << it->second << '\n';
    out << "\tadd rsp, " << (lhs.size()+rhs.size())*8 << '\n';
}

void ASTReturn::codegen(ofstream& out, Environement& env) {
    expr->codegen(out, env);
}

int ASTIfStatement::branch_count = 0;

void ASTIfStatement::codegen(ofstream& out, Environement& env) {
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

void ASTAssign::codegen(ofstream& out, Environement& env) {
    expr->codegen(out, env);
    auto it = env.adress_table.find(lval->id.name);
    if (it == env.adress_table.end()) {
        stringstream err;
        err << "identifier \"" << lval->id.name << "\" is used without being defined here";
        throw SemanticError(err.str());
    }
    out << "\tmov QWORD [rbp+" << it->second << "], rax\n";
}
