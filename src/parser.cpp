#include "parser.h"
#include "lexer.h"

SyntaxError::SyntaxError(const char* str, optional<parseNode> expected)
    : runtime_error::runtime_error(str), expected(expected) {}

parseNode::parseNode(nonTerm nt)
    : tag(NONTERM), val({.nt = nt}) {}
parseNode::parseNode(Token tok)
    : tag(TOKEN), val({.tok = tok}) {}

parseTree::parseTree(parseNode root, vector<parseTree>&& childs)
    : root(root), childs(std::move(childs)) {}

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
DEF_PARSE_NONTERM(STATEMENT); DEF_PARSE_NONTERM(EXPR); DEF_PARSE_NONTERM(EXPR_LIST); DEF_PARSE_NONTERM(ACCESS);

parseTree parse(const vector<Token>& tokens)
{
    TokenStream tokens_stream = {tokens};
    return parse_START(tokens_stream);
}

template<typename T, typename... Args>
auto make_vec(Args&&... args) {
    std::vector<T> vec;
    vec.reserve(sizeof...(Args));
    (vec.emplace_back(std::forward<Args>(args)), ...);
    return vec;
}

parseTree parse_START(TokenStream& stream)
{
    TokenOpt t;
    if (stream.ended())
        return parseTree{parseNode{START}};
    parseTree op_block = parse_OP_BLOCK(stream);
    auto childs = make_vec<parseTree>(std::move(op_block), parse_START(stream));
    return parseTree{parseNode{START}, std::move(childs)};
}

// todo : better way to "expect & raise error or add to tree"

parseTree parse_OP_BLOCK(TokenStream& stream)
{
    TokenOpt t;
    stream >> t;
    // gestion d'erreurs reste à améliorer pour inclure le numéro de ligne ... here à préciser
    if (t != OPERATOR) throw SyntaxError("expected \"operator\" keyword here", Token{OPERATOR});
    parseTree res = {parseNode{OP_BLOCK}};
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
    res.childs.push_back(parse_STAT_LIST(stream));
    return res;
}

parseTree parse_ID_LIST(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    if (tok == ID)
        return {{ID_LIST}, make_vec<parseTree>( parseTree{{tok.value()}}, parse_ID_LIST(stream) )};
    stream.go_back();
    return {{ID_LIST}};
}

parseTree parse_STAT_LIST(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    stream.go_back();
    if (tok != OPERATOR && tok.has_value()) {
        parseTree statement = parse_STATEMENT(stream);
        return {{STAT_LIST}, make_vec<parseTree>(std::move(statement), parse_STAT_LIST(stream))};
    } else
        return {{STAT_LIST}};
};

// précondition : !stream.ended()
parseTree parse_STATEMENT(TokenStream& stream) {
    parseTree res = {{STATEMENT}};
    TokenOpt tok;
    stream >> tok;
    if (tok != LET && tok != RETURN && tok != ID)
        stream.go_back();
    else
        res.add_token(tok.value());
    if (tok == LBRACKET)
        res.childs.push_back(parse_ACCESS(stream));
    if (tok == LET) {
        stream >> tok;
        if (tok != ID) throw SyntaxError("expected variable name here", Token{ID});
        res.add_token(tok.value());
    }
    TokenOpt old_tok = tok;
    if (tok == LET || tok == ID || tok == LBRACKET) {
        stream >> tok;
        if ((old_tok == LBRACKET || old_tok == ID) && tok == SEMICOL)
            return {{STATEMENT}, make_vec<parseTree>(parseTree{{EXPR}, std::move(res.childs)}, parseTree{{SEMICOL}})};
        if (tok != EQUALS) throw SyntaxError("expected \"=\" here", Token{EQUALS});
        res.add_token(tok.value());
    }

    res.childs.push_back(parse_EXPR(stream));
    stream >> tok;
    if (tok != SEMICOL) throw SyntaxError("expected \";\" here", Token{SEMICOL});
    res.add_token(tok.value());
    return res;
}

parseTree parse_EXPR(TokenStream& stream) {
    TokenOpt tok;
    stream >> tok;
    parseTree res = {{EXPR}};
    if (tok == NUM || tok == ID)
        return {{EXPR}, make_vec<parseTree>(parseTree{{tok.value()}})};
    else if (tok == LPAR) {
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
    } else if (tok == IF) {
        res.add_token(tok.value());
        res.childs.push_back(parse_EXPR(stream));
        stream >> tok;
        if (tok != THEN) throw SyntaxError("expected \"then\" here", Token{THEN});
        res.add_token(tok.value());
        res.childs.push_back(parse_EXPR(stream));
        stream >> tok;
        if (tok != ELSE) throw SyntaxError("expected \"else\" here", Token{ELSE});
        res.add_token(tok.value());
        res.childs.push_back(parse_EXPR(stream));
        return res;
    } else if (tok == LBRACKET) {
        stream.go_back();
        res.childs.push_back(parse_ACCESS(stream));
        return res;
    } else
        throw SyntaxError("expected an expression here", nonTerm{EXPR});
}

parseTree parse_ACCESS(TokenStream& stream) {
    TokenOpt token;
    stream >> token;
    if (token != LBRACKET)
        throw SyntaxError("expected \"[\" here", Token{LBRACKET});
    parseTree res{ACCESS};
    res.add_token(token.value());
    res.childs.push_back(parse_EXPR(stream));
    stream >> token;
    if (token != RBRACKET)
        throw SyntaxError("expected \"]\" here", Token{RBRACKET});
    res.add_token(token.value());
    return res;
}

parseTree parse_EXPR_LIST(TokenStream& stream) {
    try {
        parseTree expr = parse_EXPR(stream);
        return {{EXPR_LIST}, make_vec<parseTree>(std::move(expr), parse_EXPR_LIST(stream))};
    } catch (const SyntaxError& e) {
        if (e.expected.has_value() && e.expected->tag == parseNode::NONTERM && e.expected->val.nt == EXPR) {
            stream.go_back();
            return {{EXPR_LIST}};
        }
        else throw e;
    }
}
