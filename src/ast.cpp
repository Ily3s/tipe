#include "ast.h"

AST::AST(vector<unique_ptr<OpDef>>&& ops)
    : ops(std::move(ops)) {}

OpDef::OpDef(const Token& op,
        vector<unique_ptr<Var>>&& lhs_args,
        vector<unique_ptr<Var>>&& rhs_args,
        vector<unique_ptr<Statement>>&& statements)
    : op(op), lhs_args(std::move(lhs_args)), rhs_args(std::move(rhs_args)), statements(std::move(statements)) {}

Define::Define(unique_ptr<Var>&& lval, unique_ptr<Expr>&& expr)
    : lval(std::move(lval)), expr(std::move(expr)) {}

Assign::Assign(unique_ptr<Lvalue>&& lval, unique_ptr<Expr>&& expr)
    : lval(std::move(lval)), expr(std::move(expr)) {}

Return::Return(unique_ptr<Expr>&& expr)
    : expr(std::move(expr)) {}

IfStatement::IfStatement(unique_ptr<Expr>&& cond,
        unique_ptr<Expr>&& expr_true,
        unique_ptr<Expr>&& expr_false)
    : cond(std::move(cond)), expr_true(std::move(expr_true)), expr_false(std::move(expr_false)) {}

FuncCall::FuncCall(unique_ptr<Expr>&& expr)
    : expr(std::move(expr)) {}

RvalToken::RvalToken(const Token& id) : id(id) {}

RvalAccess::RvalAccess(unique_ptr<Expr>&& index)
    : index(std::move(index)) {}

OpApply::OpApply(const Token& op,
        vector<unique_ptr<Expr>>&& lhs,
        vector<unique_ptr<Expr>>&& rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

Var::Var(const Token& id)
    : id(id) {}

LvalAccess::LvalAccess(unique_ptr<Expr>&& index)
    : index(std::move(index)) {}

#define DEF_to(V) unique_ptr<V> to##V(const parseTree&)
DEF_to(Lvalue); DEF_to(Rvalue); DEF_to(Statement); DEF_to(OpDef);
DEF_to(Expr); DEF_to(OpApply); DEF_to(Define); DEF_to(Assign); DEF_to(Return);
DEF_to(IfStatement); DEF_to(Var); DEF_to(LvalAccess); DEF_to(RvalToken);
DEF_to(RvalAccess); DEF_to(FuncCall);

#define DEF_listTo(V) \
        vector<unique_ptr<V>> listTo##V(const parseTree& tree) { \
            vector<unique_ptr<V>> res; \
            const parseTree* curr = &tree; \
            while (curr->childs.size()) { \
                res.emplace_back(to##V(curr->childs[0])); \
                curr = &curr->childs[1]; \
            } \
            return res; \
        }
DEF_listTo(Expr); DEF_listTo(Statement);
DEF_listTo(Var); DEF_listTo(OpDef);

AST toAST(const parseTree& tree) {
    return {listToOpDef(tree)};
}

unique_ptr<OpDef> toOpDef(const parseTree& tree) {
    return make_unique<OpDef>(OpDef{
            tree.childs[3].root.val.tok,
            listToVar(tree.childs[2]),
            listToVar(tree.childs[4]),
            listToStatement(tree.childs[6])
            });
}

unique_ptr<Lvalue> toLvalue(const parseTree& tree) {
    if (tree.childs.empty()) return toVar(tree);
    else return toLvalAccess(tree);
}

unique_ptr<Var> toVar(const parseTree& tree) {
    return make_unique<Var>(Var{ tree.root.val.tok});
}

unique_ptr<LvalAccess> toLvalAccess(const parseTree& tree) {
    return make_unique<LvalAccess>( toExpr(tree.childs[1]) );
}

unique_ptr<Rvalue> toRvalue(const parseTree& tree) {
    if (tree.childs.empty()) return toRvalToken(tree);
    else return toRvalAccess(tree);
}

unique_ptr<RvalToken> toRvalToken(const parseTree& tree) {
    return make_unique<RvalToken>(RvalToken{ tree.root.val.tok});
}

unique_ptr<RvalAccess> toRvalAccess(const parseTree& tree) {
    return make_unique<RvalAccess>( toExpr(tree.childs[1]) );
}

unique_ptr<Statement> toStatement(const parseTree& tree) {
    if (tree.childs.size() == 2) return toFuncCall(tree);
    if (tree.childs.size() == 3) return toReturn(tree);
    if (tree.childs.size() == 4) return toAssign(tree);
    if (tree.childs.size() == 5) return toDefine(tree);
    return 0;
}

unique_ptr<FuncCall> toFuncCall(const parseTree& tree) {
    return make_unique<FuncCall>( toExpr(tree.childs[0]) );
}

unique_ptr<Return> toReturn(const parseTree& tree) {
    return make_unique<Return>(Return{ toExpr(tree.childs[1])});
}

unique_ptr<Assign> toAssign(const parseTree& tree) {
    return make_unique<Assign>(Assign{
            toLvalue(tree.childs[0]),
            toExpr(tree.childs[2])
            });
}

unique_ptr<Define> toDefine(const parseTree& tree) {
    return make_unique<Define>(Define{
            toVar(tree.childs[1]),
            toExpr(tree.childs[3])
            });
}

unique_ptr<IfStatement> toIfStatement(const parseTree& tree) {
    return make_unique<IfStatement>(IfStatement{
            toExpr(tree.childs[1]),
            toExpr(tree.childs[3]),
            toExpr(tree.childs[5])
            });
}

unique_ptr<Expr> toExpr(const parseTree& tree) {
    if (tree.childs.size() == 1) return toRvalue(tree.childs[0]);
    if (tree.childs.size() == 5) return toOpApply(tree);
    if (tree.childs.size() == 6) return toIfStatement(tree);
    return 0;
}

unique_ptr<OpApply> toOpApply(const parseTree& tree) {
    return make_unique<OpApply>(OpApply{
            tree.childs[2].root.val.tok,
            listToExpr(tree.childs[1]),
            listToExpr(tree.childs[3])
            });
}
