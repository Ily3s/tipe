#ifndef _H
#define _H

#include "parser.h"
#include "codegen.h"

#include <fstream>
#include <memory>

class Node {
    public :
        virtual void codegen(ofstream& out, Environement& env) = 0;
};

class Expr : public Node {};

class Rvalue : public Expr {};

class RvalToken : public Rvalue {
    public :
        Token id;
        RvalToken(const Token& id);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class RvalAccess : public Rvalue {
    public :
        unique_ptr<Expr> index;
        RvalAccess(unique_ptr<Expr>&& index);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class OpApply : public Expr {
    public :
        Token op;
        vector<unique_ptr<Expr>> lhs, rhs;
        OpApply(const Token& op,
                vector<unique_ptr<Expr>>&& lhs,
                vector<unique_ptr<Expr>>&& rhs);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class Lvalue : public Node {
    public :
        virtual string get_name(Environement& env) = 0;
        virtual int size() = 0;
};

class Var : public Lvalue {
    public :
        Token id;
        int offset;
        Var(const Token& id);
        virtual void codegen(ofstream& out, Environement& env) override;
        void define_in_scope(Environement& env);
        string get_name(Environement& env) override;
        int size() override;
};

class LvalAccess : public Lvalue {
    public :
        unique_ptr<Expr> index;
        LvalAccess(unique_ptr<Expr>&& index);
        virtual void codegen(ofstream& out, Environement& env) override;
        string get_name(Environement& env) override;
        int size() override;
};

class Statement : public Node {};

class FuncCall : public Statement {
    public :
        unique_ptr<Expr> expr;
        FuncCall(unique_ptr<Expr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class Define : public Statement {
    public :
        unique_ptr<Var> lval;
        unique_ptr<Expr> expr;
        Define(unique_ptr<Var>&& lval, unique_ptr<Expr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class Assign : public Statement {
    public :
        unique_ptr<Lvalue> lval;
        unique_ptr<Expr> expr;
        Assign(unique_ptr<Lvalue>&& lval, unique_ptr<Expr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class Return : public Statement {
    public :
        unique_ptr<Expr> expr;
        Return(unique_ptr<Expr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class IfStatement : public Expr {
    public :
        unique_ptr<Expr> cond;
        unique_ptr<Expr> expr_true;
        unique_ptr<Expr> expr_false;
        IfStatement(unique_ptr<Expr>&& cond,
                unique_ptr<Expr>&& expr_true,
                unique_ptr<Expr>&& expr_false);
        virtual void codegen(ofstream& out, Environement& env) override;
        static int branch_count;
};

class Scope : public Node {
    public :
        int int_def_nb = 0;
        int bytes_owned = 0;
        void init_scope(ofstream& out, Environement& env);
        void del_scope(ofstream& out, Environement& env);
};

class OpDef : public Scope {
    public :
        Token op;
        vector<unique_ptr<Var>> lhs_args, rhs_args;
        vector<unique_ptr<Statement>> statements;
        OpDef(const Token& op,
                vector<unique_ptr<Var>>&& lhs_args,
                vector<unique_ptr<Var>>&& rhs_args,
                vector<unique_ptr<Statement>>&& statements);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class AST : public Scope {
    public :
        vector<unique_ptr<OpDef>> ops;
        AST(vector<unique_ptr<OpDef>>&& ops);
        virtual void codegen(ofstream& out, Environement& env) override;
};

AST toAST(const parseTree& tree);

#endif
