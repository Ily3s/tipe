#include "parser.h"
#include <memory>

SyntaxError::SyntaxError(const char* str, optional<parseNode> expected)
    : runtime_error::runtime_error(str), expected(expected) {}

parseNode::parseNode(nonTerm nt)
    : tag(NONTERM), val({.nt = nt}) {}
parseNode::parseNode(Token tok)
    : tag(TOKEN), val({.tok = tok}) {}

class TokenOpt : public optional<Token> {
public:
    using optional<Token>::optional;
    bool operator==(tokent tt) {
        return has_value() && value().type == tt;
    }
    bool operator!=(tokent tt) { return !(operator==(tt)); }
};

class TokenStream{
    const vector<Token>& m_tokens;
    int m_index;
public :
    TokenStream(const vector<Token>& tok)
        : m_tokens(tok), m_index(0) {}

    TokenStream& operator>>(TokenOpt& t) {
        if (m_index >= m_tokens.size()) t = nullopt;
        else t = m_tokens[m_index];
        m_index++;
        return *this;
    }

    operator bool(){
        return m_index <= m_tokens.size();
    }

    void go_back() {
        m_index--;
    }

    bool ended() {
        return m_index >= m_tokens.size();
    }
};

#define DEF_PARSE_NONTERM(V) \
parseTree parse_##V(TokenStream& stream)
DEF_PARSE_NONTERM(START); DEF_PARSE_NONTERM(OP_BLOCK); DEF_PARSE_NONTERM(ID_LIST); DEF_PARSE_NONTERM(STAT_LIST);
DEF_PARSE_NONTERM(STATEMENT); DEF_PARSE_NONTERM(EXPR); DEF_PARSE_NONTERM(EXPR_LIST);

parseTree parser(const vector<Token>& tokens)
{
    TokenStream tokens_stream = {tokens};
    return parse_START(tokens_stream);
    //throw SyntaxError("Program must contains :main function");
}

parseTree parse_START(TokenStream& stream)
{
    TokenOpt t;
    if (stream.ended())
        return parseTree{.root = parseNode{START}, .childs = {}};
    parseTree op_block = parse_OP_BLOCK(stream);
    return parseTree{.root = parseNode{START}, .childs = {op_block, parse_START(stream)}};
}

// todo : better way to "expect & raise error or add to tree"

parseTree parse_OP_BLOCK(TokenStream& stream)
{
    TokenOpt t;
    stream >> t;
    // gestion d'erreurs reste à améliorer pour inclure le numéro de ligne ... here à préciser
    if (t != OPERATOR) throw SyntaxError("expected \"operator\" keyword here", Token{OPERATOR});
    parseTree res = {.root = parseNode{OP_BLOCK}};
    res.add_token(t.value());
    stream >> t;
    if (t != LPAR) throw SyntaxError("expected \"(\" here", Token{LPAR});
    res.add_token(t.value());
    res.childs.push_back(parse_ID_LIST(stream));
    stream >> t;
    if (t != OPID) throw SyntaxError("expected an operator here", Token{OPID});
    res.add_token(t.value());
    res.childs.push_back(parse_ID_LIST(stream));
    stream >> t;
    if (t != RPAR) throw SyntaxError("expected \")\" here", Token{RPAR});
    res.add_token(t.value());
    //if (!subtree3.has_value()) throw SyntaxError("operator must have a return value");
    res.childs.push_back(parse_STAT_LIST(stream));
    return res;
}

parseTree parse_ID_LIST(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    if (tok == ID)
        return {.root = {ID_LIST}, .childs = {{.root = {tok.value()}}, parse_ID_LIST(stream)}};
    stream.go_back();
    return {.root = {ID_LIST}};
}

parseTree parse_STAT_LIST(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    stream.go_back();
    if (tok == LET || tok == ID || tok == RETURN) {
        parseTree statement = parse_STATEMENT(stream);
        return {.root = {STAT_LIST}, .childs = {statement, parse_STAT_LIST(stream)}};
    } else
        return {.root = {STAT_LIST}};
};

// précondition : !stream.ended()
parseTree parse_STATEMENT(TokenStream& stream) {
    parseTree res = {.root = {STATEMENT}};
    TokenOpt tok;
    stream >> tok;
    res.add_token(tok.value());
    if (tok == LET) {
        stream >> tok;
        if (tok != ID) throw SyntaxError("expected variable name here", Token{ID});
        res.add_token(tok.value());
    }
    if (tok == LET || tok == ID) {
        stream >> tok;
        if (tok != EQUALS) throw SyntaxError("expected \"=\" here", Token{EQUALS});
        res.add_token(tok.value());
    }
    res.childs.push_back(parse_EXPR(stream));
    stream >> tok;
    if (tok != SEMICOL) throw SyntaxError("expected \";\" here", Token{SEMICOL});
    res.add_token(tok.value());
    return res;
};

parseTree parse_EXPR(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    if (tok == NUM || tok == ID)
        return {.root = {EXPR}, .childs = {{.root = {tok.value()}}}};
    else if (tok == LPAR) {
        parseTree res = {.root = {EXPR}};
        res.add_token(tok.value());
        res.childs.push_back(parse_EXPR_LIST(stream));
        stream >> tok;
        if (tok != OPID) throw SyntaxError("expected an operator here", Token{OPID});
        res.add_token(tok.value());
        res.childs.push_back(parse_EXPR_LIST(stream));
        stream >> tok;
        if (tok != RPAR) throw SyntaxError("expected \")\" here", Token{RPAR});
        res.add_token(tok.value());
        return res;
    } else
        throw SyntaxError("expected an expression here", nonTerm{EXPR});
}

parseTree parse_EXPR_LIST(TokenStream& stream) {
    try {
        parseTree expr = parse_EXPR(stream);
        return {.root = {EXPR_LIST}, .childs = {expr, parse_EXPR_LIST(stream)}};
    } catch (const SyntaxError& e) {
        if (e.expected.has_value() && e.expected->tag == parseNode::NONTERM && e.expected->val.nt == EXPR) {
            stream.go_back();
            return {.root = {EXPR_LIST}};
        }
        else throw e;
    }
}

#define DEF_toAST(V) \
unique_ptr<AST##V> toAST##V(const parseTree&)
DEF_toAST(Lvalue); DEF_toAST(RvalueVar); DEF_toAST(Statement); DEF_toAST(OpDef);
DEF_toAST(Expr); DEF_toAST(OpApply); DEF_toAST(Define); DEF_toAST(Assign); DEF_toAST(Return);

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
    return {.ops = listToASTOpDef(tree)};
}

unique_ptr<ASTOpDef> toASTOpDef(const parseTree& tree) {
    return make_unique<ASTOpDef>(ASTOpDef{
        .op = tree.childs[3].root.val.tok,
        .lhs_args = listToASTLvalue(tree.childs[2]),
        .rhs_args = listToASTLvalue(tree.childs[4]),
        .statements = listToASTStatement(tree.childs[6])
    });
}

unique_ptr<ASTLvalue> toASTLvalue(const parseTree& tree) {
    return make_unique<ASTLvalue>(ASTLvalue{.id = tree.root.val.tok});
}

unique_ptr<ASTRvalueVar> toASTRvalueVar(const parseTree& tree) {
    return make_unique<ASTRvalueVar>(ASTRvalueVar{.id = tree.root.val.tok});
}

unique_ptr<ASTStatement> toASTStatement(const parseTree& tree) {
    if (tree.childs.size() == 3) return toASTReturn(tree);
    if (tree.childs.size() == 4) return toASTAssign(tree);
    if (tree.childs.size() == 5) return toASTDefine(tree);
    return 0;
}

unique_ptr<ASTReturn> toASTReturn(const parseTree& tree) {
    return make_unique<ASTReturn>(ASTReturn{.expr = toASTExpr(tree.childs[1])});
}

unique_ptr<ASTAssign> toASTAssign(const parseTree& tree) {
    return make_unique<ASTAssign>(ASTAssign{
            .lval = toASTLvalue(tree.childs[0]),
            .expr = toASTExpr(tree.childs[2])
            });
}

unique_ptr<ASTDefine> toASTDefine(const parseTree& tree) {
    return make_unique<ASTDefine>(ASTDefine{
            .lval = toASTLvalue(tree.childs[1]),
            .expr = toASTExpr(tree.childs[3])
            });
}

unique_ptr<ASTExpr> toASTExpr(const parseTree& tree) {
    if(tree.childs.size() == 1) return toASTRvalueVar(tree);
    else return toASTOpApply(tree);
}

unique_ptr<ASTOpApply> toASTOpApply(const parseTree& tree) {
    return make_unique<ASTOpApply>(ASTOpApply{
            .op = tree.childs[2].root.val.tok,
            .lhs = listToASTExpr(tree.childs[1]),
            .rhs = listToASTExpr(tree.childs[3])
            });
}
