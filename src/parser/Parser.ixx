export module parser;

import lexer;
import parser.ast;

import <memory>;
import <functional>;
import <sstream>;
import <stack>;

#include "attrdef.hxx"

using std::shared_ptr;
using std::make_shared;

export namespace hel {
    class Parser {
        mutable std::reference_wrapper<Lexer> mLex;
    public:
        explicit Parser(Lexer& iLex) : mLex(iLex) {}
        AstTree get_tree() {
            AstTree tree{};
            tree.root = ParsePragma();
            return tree;
        }
        NODISCARD const Token & cur() const {
            return lex().PeekToken(0);
        }
    protected:
        Lexer& lex() const {
            return mLex.get();
        }
    private:
        KeyDef exchange_key() const {
            if (cur().kind >= ETokenKind::Key) {
                return KeyDef(static_cast<signed>(cur().kind) - static_cast<signed>(ETokenKind::Key));
            }
            return KeyDef::Ukn;
        }
        void ExpectToken(ETokenKind kind) const {
            if (cur().kind == kind || cur().kind == ETokenKind::Annotation) {
                lex().NextToken();
            } else {
                // TODO: [error]: expect token '{expect_kind}' but now the token is '{cur_kind}';
                throw std::runtime_error("[error]: expect token '{expect_kind}' but now the token is '{cur_kind}'");
            }
        }
        AstNode<EAst::Pragma> ParsePragma () {
            auto pragma = AstTree::make_node<EAst::Pragma>();
            pragma->stmts.push_back(ParseStmt());
            return pragma;
        }
        AstNode<EAst::Stmt> ParseStmt() {
            switch (exchange_key()) {
                case KeyDef::If:
                    return ParseIfStmt();
                default:
                    return ParseExprStmt();
            }
        }
        AstNode<EAst::Expr> ParseExpr() {
            return ParseMidExpr();
        }
        AstNode<EAst::ConNumb> ParseConNumb() const {
            auto numb{AstTree::make_node<EAst::ConNumb>()};
            std::stringstream ss;
            ss << cur().get<num_t>();
            ss >> numb->num;
            ExpectToken(ETokenKind::Num);
            return numb;
        }
        AstNode<EAst::PriExpr> ParsePriExpr() {
            auto pri_expr{AstTree::make_node<EAst::PriExpr>()};
            if (cur().kind == ETokenKind::Num) {
                pri_expr->rhs = ParseConNumb();
                return pri_expr;
            } else if (cur().kind == ETokenKind('(')) {
                ExpectToken(ETokenKind('('));
                pri_expr->rhs = ParseExpr();
                ExpectToken(ETokenKind(')'));
                return pri_expr;
            } else if (cur().kind == ETokenKind::Tag) {
                auto var = AstTree::make_node<EAst::MutVar>();
                var->nm = cur().get<tag_t>();
                ExpectToken(ETokenKind::Tag);
                pri_expr->rhs = var;
                return pri_expr;
            }
            return nullptr;
        }
        AstNode<EAst::MidExpr> ParseMidExpr() {
            using MidOperator = Ast<EAst::MidExpr>::MidOperator;
            using enum MidOperator;
            auto mid_expr{AstTree::make_node<EAst::MidExpr>()};
            std::stack <AstNode<EAst::Expr>> expt;
            std::stack <MidOperator> opts;
            bool is_end = false;
            auto cmpOpt{[&](MidOperator opt1, MidOperator opt2) -> bool {
                if (is_end)
                    return true;            // >
                if (opt1 == Add || opt1 == Sub) {
                    if (opt2 == Add || opt2 == Sub)
                        return true;        // >
                    else if (opt2 == Mul || opt2 == Div)
                        return false;       // <
                } else if (opt1 == Mul || opt1 == Div)
                    return true;            // >
                //TODO: [sorry]: '{operator}' is not supported;
                throw std::runtime_error("[sorry]: '{operator}' is not supported;");
            }};
            auto curOpt = [&]() -> MidOperator {
                switch (cur().kind) {
                    case ETokenKind('+'):
                        return Add;
                    case ETokenKind('-'):
                        return Sub;
                    case ETokenKind('*'):
                        return Mul;
                    case ETokenKind('/'):
                        return Div;
                    default:
                        return UKn;
                }
            };
            while (!is_end) {
                if (auto priExpr{ParsePriExpr()}; priExpr == nullptr) {
                    // cur() is a character
                    auto opt{curOpt()};
                    if (opt == UKn) {
                        is_end = true;
                        if (opts.empty()) {
                            mid_expr->rhs = expt.top();
                            break;
                        }
                    }
                    if (!opts.empty() && cmpOpt(opts.top(), opt)) {
                        mid_expr->opt = opts.top();
                        opts.pop();
                        mid_expr->rhs = expt.top();
                        expt.pop();
                        mid_expr->lhs = expt.top();
                        expt.pop();
                        expt.push(mid_expr);
                        if (is_end) break;
                        mid_expr = AstTree::make_node<EAst::MidExpr>();
                    } else {
                        opts.push(opt);
                        lex().NextToken();
                    }
                } else { expt.push(priExpr); }
            }
            return mid_expr;
        }
        AstNode<EAst::ExprStmt> ParseExprStmt() {
            auto expr_stmt = AstTree::make_node<EAst::ExprStmt>();
            expr_stmt->expr = ParseExpr();
            ExpectToken(ETokenKind(';'));
            return expr_stmt;
        }
        AstNode<EAst::If_Else> ParseIfStmt() {
            auto if_else = AstTree::make_node<EAst::If_Else>();
            lex().NextToken();
            ExpectToken(ETokenKind('('));
            if_else->condition = ParseExpr();
            ExpectToken(ETokenKind(')'));
            if_else->ifS = ParseStmt();
            if (exchange_key() == KeyDef::Else) {
                lex().NextToken();
                if_else->elseS = ParseStmt();
            }
            return if_else;
        }
    };
}