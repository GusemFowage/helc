export module parser;

import <memory>;
import <functional>;
import <sstream>;
import <stack>;

import lexer;
import parser.ast;
import symbol.debugger;

#include "attrdef.hxx"

using std::shared_ptr;
using std::make_shared;

export namespace hel {
    class Parser {
        mutable std::reference_wrapper<Lexer> mLex;
        LocalDefines* local{nullptr};
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
                // TODO: [error]:
                decltype(debMsg::msg) msg;
                if ( kind >= ETokenKind::Flr && kind < ETokenKind::Rng) {
                    msg = std::format(
                        "expect '{}' but now it is '{}';",
                        char(kind), (lex().cur() == char_info::eof() ? "\\0" : string("")+lex().cur())
                    );
                }

                Debugger::interface()->add_msg({
                       .level=debMsg::Level::Error,
                       .src_info=cur().addr,
                       .msg=msg
               });
            }
        }
        shared_ptr<MutableValue> FindLocalVar(const string_view& nm) const {
            for (auto& v : local->localVars) {
                auto&[ident, offset](*v);
                if (ident == nm) {
                    return v;
                }
            }
            return nullptr;
        }
        shared_ptr<MutableValue> MakeLocalVar(const string_view& nm) {
            auto var = make_shared<MutableValue>();
            var->ident = nm;
            var->offset = {};
            local->localVars.insert(var);
            return var;
        }
        AstNode<EAst::Pragma> ParsePragma () {
            auto pragma = AstTree::make_node<EAst::Pragma>();
            local = &(pragma->local_def);
            while (cur().kind != ETokenKind::Eof) {
                pragma->stmts.push_back(ParseStmt());
            }
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
        AstNode<EAst::Expr> ParsePriExpr() {
//            auto pri_expr{AstTree::make_node<EAst::PriExpr>()};
            if (cur().kind == ETokenKind::Num) {
                return ParseConNumb();
            } else if (cur().kind == ETokenKind('(')) {
                ExpectToken(ETokenKind('('));
                auto pri_expr = ParseExpr();
                ExpectToken(ETokenKind(')'));
                return pri_expr;
            } else if (cur().kind == ETokenKind::Tag) {
                auto var_expr = AstTree::make_node<EAst::MutVar>();
                auto nm = cur().get<tag_t>();
                if (auto obj{FindLocalVar(nm)}; !obj) {
                    obj = MakeLocalVar(nm);
                    var_expr->varObj = obj;
                } else {
                    var_expr->varObj = obj;
                }
                ExpectToken(ETokenKind::Tag);
                return var_expr;
            }
            return nullptr;
        }
        AstNode<EAst::Expr> ParseMidExpr() {
            using MidOperator = Ast<EAst::MidExpr>::MidOperator;
            using enum MidOperator;
            struct mid_operator {
                MidOperator opt;
                [[nodiscard]] unsigned priority() const {
                    // operator priority
                    switch (opt) {
                        case UKn: return 0;
                        case Ass: return 1;
                        case Add: case Sub: return 2;
                        case Mul: case Div: return 3;
                        default:
                            //TODO: [sorry]: '{operator}' is not supported;
                            throw std::runtime_error("[sorry]: '{operator}' is not supported;");
                    }
                }
            };
            auto mid_expr{AstTree::make_node<EAst::MidExpr>()};
            std::stack <AstNode<EAst::Expr>> expt;
            std::stack <mid_operator> opts;
            bool is_end = false;
            auto curOpt = [&]() -> MidOperator {
                switch (cur().kind) {
                case ETokenKind('+'): return Add;
                case ETokenKind('-'): return Sub;
                case ETokenKind('*'): return Mul;
                case ETokenKind('/'): return Div;
                case ETokenKind('='): return Ass;
                default: return UKn;
                }
            };
            auto nextOpt{[&](MidOperator opt) {
                if (false) {
                    lex().NextToken();
                }
                lex().NextToken();
            }};
            while (!is_end) {
                if (auto priExpr{ParsePriExpr()}; priExpr == nullptr) {
                    // cur() is a character
                    auto opt{mid_operator(curOpt())};
                    if (opt.opt == UKn) {
                        is_end = true;
                        if (opts.empty()) {
                            mid_expr->rhs = expt.top();
                            break;
                        }
                    }
                    if (!opts.empty() && (opt.priority() <= opts.top().priority())) {
                        do {
                            mid_expr->opt = opts.top().opt;
                            opts.pop();
                            mid_expr->rhs = expt.top();
                            expt.pop();
                            mid_expr->lhs = expt.top();
                            expt.pop();
                            expt.push(mid_expr);
                            mid_expr = AstTree::make_node<EAst::MidExpr>();
                        } while (!opts.empty() && (opt.priority() <= opts.top().priority()));
                        if (is_end) break;
                    } else {
                        opts.emplace(opt);
                        nextOpt(opt.opt);
                    }
                } else { expt.push(priExpr); }
            }
            return expt.top();
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