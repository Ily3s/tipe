#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

#include <fstream>
#include <memory>
#include <unordered_map>

#define TAPE_SIZE 80000

struct Signature{
    string name;
    int left_arity, right_arity;
    bool operator==(const Signature& rhs) const;
};

template<> struct std::hash<Signature>{
    size_t operator()(const Signature& sign) const;
};

class ASTScope; 

struct Environement{
    unordered_map<string, int> adress_table;
    vector<string> stack_frame;
    int curr_addr;
    unordered_map<Signature, int> op_ids;
    int ops_nb = 0;
    ASTScope* curr_scope;
};

class ASTNode {
    public :
        virtual void codegen(ofstream& out, Environement& env) = 0;
};

class ASTLvalue : public ASTNode {
    public :
        virtual string get_name(Environement& env) = 0;
        virtual int size() = 0;
};

class ASTVar : public ASTLvalue {
    public :
        Token id;
        int offset;
        ASTVar(const Token& id);
        virtual void codegen(ofstream& out, Environement& env) override;
        void define_in_scope(Environement& env);
        string get_name(Environement& env) override;
        int size() override;
};

class ASTExpr;

class ASTLvalAccess : public ASTLvalue {
    public :
        unique_ptr<ASTExpr> index;
        ASTLvalAccess(unique_ptr<ASTExpr>&& index);
        virtual void codegen(ofstream& out, Environement& env) override;
        string get_name(Environement& env) override;
        int size() override;
};

class ASTStatement : public ASTNode {
    public :

};

class ASTFuncCall : public ASTStatement {
    public :
        unique_ptr<ASTExpr> expr;
        ASTFuncCall(unique_ptr<ASTExpr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;

};

class ASTScope : public ASTNode {
    public :
        int int_def_nb = 0;
        int bytes_owned = 0;
        void init_scope(ofstream& out, Environement& env);
        void del_scope(ofstream& out, Environement& env);
};

class ASTOpDef : public ASTScope {
    public :
        Token op;
        vector<unique_ptr<ASTVar>> lhs_args, rhs_args;
        vector<unique_ptr<ASTStatement>> statements;
        ASTOpDef(const Token& op,
                vector<unique_ptr<ASTVar>>&& lhs_args,
                vector<unique_ptr<ASTVar>>&& rhs_args,
                vector<unique_ptr<ASTStatement>>&& statements);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTExpr : public ASTNode {

};

class ASTRvalue : public ASTExpr {

};

class ASTRvalToken : public ASTRvalue {
    public :
        Token id;
        ASTRvalToken(const Token& id);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTRvalAccess : public ASTRvalue {
    public :
        unique_ptr<ASTExpr> index;
        ASTRvalAccess(unique_ptr<ASTExpr>&& index);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTOpApply : public ASTExpr {
    public :
        Token op;
        vector<unique_ptr<ASTExpr>> lhs, rhs;
        ASTOpApply(const Token& op,
                vector<unique_ptr<ASTExpr>>&& lhs,
                vector<unique_ptr<ASTExpr>>&& rhs);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTDefine : public ASTStatement {
    public :
        unique_ptr<ASTVar> lval;
        unique_ptr<ASTExpr> expr;
        ASTDefine(unique_ptr<ASTVar>&& lval, unique_ptr<ASTExpr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTAssign : public ASTStatement {
    public :
        unique_ptr<ASTLvalue> lval;
        unique_ptr<ASTExpr> expr;
        ASTAssign(unique_ptr<ASTLvalue>&& lval, unique_ptr<ASTExpr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTReturn : public ASTStatement {
    public :
        unique_ptr<ASTExpr> expr;
        ASTReturn(unique_ptr<ASTExpr>&& expr);
        virtual void codegen(ofstream& out, Environement& env) override;
};

class ASTIfStatement : public ASTExpr {
    public :
        unique_ptr<ASTExpr> cond;
        unique_ptr<ASTExpr> expr_true;
        unique_ptr<ASTExpr> expr_false;
        ASTIfStatement(unique_ptr<ASTExpr>&& cond,
                       unique_ptr<ASTExpr>&& expr_true,
                       unique_ptr<ASTExpr>&& expr_false);
        virtual void codegen(ofstream& out, Environement& env) override;
        static int branch_count;
};

class AST : public ASTScope {
    public :
        vector<unique_ptr<ASTOpDef>> ops;
        AST(vector<unique_ptr<ASTOpDef>>&& ops);
        virtual void codegen(ofstream& out, Environement& env) override;
};

AST toAST(const parseTree&);

class SemanticError : public std::runtime_error{
    public :
        using std::runtime_error::runtime_error;
};

#endif
