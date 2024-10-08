export module parser;

import <memory>;
import <functional>;
import <sstream>;
import <stack>;

import lexer;
import parser.ast;
import symbol.debugger;
import hc.types;

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
                        char(kind), char_info::to_seen_char(char(cur().kind))
                    );
                }

                Debugger::interface()->add_msg({
                   .level=debMsg::Level::Error,
                   .src_info=cur().addr,
                   .msg=msg
               });
            }
        }
        shared_ptr<MutableValue> FindLocalVar(const string_view& nm) {
            for (auto& v : local->localVars) {
                auto&[ty, ident, offset](*v);
                if (ident == nm) {
                    return v;
                }
            }
            return nullptr;
        }
        shared_ptr<MutableValue> MakeLocalVar(const string_view& nm, std::shared_ptr<Type> ty) {
            auto var = make_shared<MutableValue>();
            var->ident = nm;
            var->offset = {};
            local->localVars.push_back(var);
            return var;
        }
        AstNode<EAst::MutVar> MakeVarNode(shared_ptr<MutableValue> var) {
            auto vn{AstTree::make_node<EAst::MutVar>()};
            vn->varObj = var;
            vn->ty = var->type;
            return vn;
        }

        shared_ptr<Type> ParseDeclarationSpec() {
            switch (exchange_key()) {
            case KeyDef::Int:
                lex().NextToken();
                return type_def::IntTy;
            default:
                Debugger::interface()->add_msg({
                    .level = debMsg::Fail,
                    .src_info = cur().addr,
                    .msg = "type not support yet",
                });
                return nullptr;
            }
        }
        shared_ptr<Type> ParseDeclarator(shared_ptr<Type> baseTy, tag_t& name) {
            auto ty = baseTy;
            while (cur().kind == ETokenKind('*')) {
                ty = std::make_shared<TypeImpl<TypeKind::Pointer>>(ty);
                lex().NextToken();
            }
            if (cur().kind != ETokenKind::Tag) {
                Debugger::interface()->add_msg({
                    .level = debMsg::Error,
                    .src_info = cur().addr,
                    .msg = "expected a variable name"
                });
            }
            name = cur().get<tag_t >();
            lex().NextToken();
            return ParseTypeSuffix(ty);
        }
        shared_ptr<Type> ParseTypeSuffix(const shared_ptr<Type>& baseTy) {
            if (cur().kind == ETokenKind('(')) {
                lex().NextToken();
                auto funcTy = make_shared<TypeImpl<TypeKind::Function>>(baseTy);
                if (cur().kind!=ETokenKind(')')) {
//                    auto ty = ;
                    tag_t nm;
                    auto ty{
                        ParseDeclarator(ParseDeclarationSpec(), nm)
                    };
                    auto p{
                        make_shared<FuncParam>(ty, nm)
                    };
                    funcTy->args.push_back(p);
                    while (cur().kind != ETokenKind(')')) {
                        ExpectToken(ETokenKind(','));

                        tag_t nm_;
                        auto ty_{
                                ParseDeclarator(ParseDeclarationSpec(), nm_)
                        };
                        auto p_{
                                make_shared<FuncParam>(ty_, nm_)
                        };
                        funcTy->args.push_back(p_);
                    }

                }
                ExpectToken(ETokenKind(')'));
                return funcTy;
            }
            return baseTy;
        }

        AstNode<EAst::Pragma> ParsePragma () {
            auto pragma = AstTree::make_node<EAst::Pragma>();
            while (cur().kind != ETokenKind::Eof) {
                pragma->funcs.push_back(ParseFunction());
            }
            return pragma;
        }
        AstNode<EAst::FuncDef> ParseFunction() {
            auto fnc = AstTree::make_node<EAst::FuncDef>();
            local = &fnc->local_def;

            auto ty = ParseDeclarationSpec();
            tag_t name;
            ty = ParseDeclarator(ty, name);

            fnc->nm = name;
            fnc->ty = ty;

            auto funcTy = std::dynamic_pointer_cast<TypeImpl<TypeKind::Function>>(ty);
            std::for_each(
            funcTy->args.rbegin(),
            funcTy->args.rend(),
                [&](auto& i) {
                fnc->args.push_front(MakeLocalVar(i->name, i->type));
            });

            ExpectToken(ETokenKind('{'));
            while (cur().kind != ETokenKind('}')) {
                fnc->stmts.push_back(ParseStmt());
            }
            ExpectToken(ETokenKind('}'));
            return fnc;
        }
        AstNode<EAst::Stmt> ParseStmt() {
            switch (exchange_key()) {
                case KeyDef::If:
                    return ParseIfStmt();
                case KeyDef::For:
                    return ParseForStmt();
                case KeyDef::Do:
                    return ParseDoWhileStmt();
                case KeyDef::While:
                    return ParseWhileStmt();
                case KeyDef::Return:
                    return ParseRetStmt();
                case KeyDef::Int: {
                    auto decl{AstTree::make_node<EAst::Decl>()};
                    auto ty{ParseDeclarationSpec()};
                    int i = 0;
                    while (cur().kind != ETokenKind(';')) {
                        if (i > 0) {
                            ExpectToken(ETokenKind(','));
                        }
                        tag_t nm;
                        ty = ParseDeclarator(ty, nm);
                        auto var{MakeLocalVar(nm, ty)};
                        i++;
                        if (cur().kind != ETokenKind('=')) {
                            continue;
                        }
                        ExpectToken(ETokenKind('='));
                        auto ass = AstTree::make_node<EAst::MidExpr>();
                        ass->opt = Ast<EAst::MidExpr>::MidOperator::Ass;
                        ass->lhs = MakeVarNode(var);
                        ass->rhs = ParseExpr();
                        decl->ass.push_back(ass);
                    }
                    ExpectToken(ETokenKind(';'));
                    return decl;
                }
                default:
                    if (cur().kind == ETokenKind('{')) {
                        return ParseBlock();
                    } else return ParseExprStmt();
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
            }
            else if (cur().kind == ETokenKind('(')) {
                if (lex().PeekToken(1).kind == ETokenKind('{')) {
                    ExpectToken(ETokenKind('('));
                    ExpectToken(ETokenKind('{'));
                    auto se{AstTree::make_node<EAst::StmtExpr>()};
                    while (cur().kind != ETokenKind('}')) {
                        se->stmts.push_back(ParseStmt());
                    }
                    lex().NextToken();
                    ExpectToken(ETokenKind(')'));
                    return se;
                }
                ExpectToken(ETokenKind('('));
                auto pri_expr = ParseExpr();
                ExpectToken(ETokenKind(')'));
                return pri_expr;
            }
            else if (cur().kind == ETokenKind::Tag) {
                if (lex().PeekToken(1).kind == ETokenKind('(')) {
                    auto fnc_call{AstTree::make_node<EAst::Call>()};
                    fnc_call->nm = cur().get<tag_t>();
                    ExpectToken(ETokenKind::Tag);
                    ExpectToken(ETokenKind('('));
                    if (cur().kind != ETokenKind(')')) {
                        fnc_call->args.push_back(ParseExpr());
                        while (cur().kind == ETokenKind(',')){
                            lex().NextToken();
                            fnc_call->args.push_back(ParseExpr());
                        }
                    }
                    ExpectToken(ETokenKind(')'));
                    return fnc_call;
                }
                auto var_expr = AstTree::make_node<EAst::MutVar>();
                auto nm = cur().get<tag_t>();
                if (auto obj{FindLocalVar(nm)}; obj) {
                    var_expr->varObj = obj;
                } else {
//                    var_expr->varObj = MakeLocalVar(nm, type_def::IntTy);
                    Debugger::interface()->add_msg({
                        .level = debMsg::Error,
                        .src_info = cur().addr,
                        .msg = std::format("can't find variable '{}'", nm)
                    });
                }
                ExpectToken(ETokenKind::Tag);
                return var_expr;
            }
            else return nullptr;
        }
        AstNode<EAst::Expr> ParseUnaryExpr() {
            using UnaryOperator = Ast<EAst::UnaryExpr>::UnaryOperator;
            using enum UnaryOperator;
            auto unry{AstTree::make_node<EAst::UnaryExpr>()};
            switch (cur().kind) {
            case ETokenKind('+'): {
                unry->opt = Plus;
                break;
            }
            case ETokenKind('-'): {
                unry->opt = Minus;
                break;
            }
            case ETokenKind('*'): {
                unry->opt = Fetch;
                break;
            }
            case ETokenKind('&'): {
                unry->opt = GetAddr;
                break;
            }
            default:
                return ParsePriExpr();
            }
            lex().NextToken();
            unry->expr = ParseUnaryExpr();
            return unry;
        }
        AstNode<EAst::Expr> ParseMidExpr() {
            using MidOperator = Ast<EAst::MidExpr>::MidOperator;
            using enum MidOperator;
            static Parser* par;
            par = this;
            struct mid_operator {
                MidOperator opt;
                unsigned chr_cnt;
                [[nodiscard]] unsigned priority() const {
                    // operator priority
                    switch (opt) {
                        case UKn: return 0;
                        case Ass: return 1;
                        case Equ: case nEq:
                        case lEq: case gEq:
                        case Lss: case Gtr:
                            return 2;
                        case Add: case Sub: return 3;
                        case Mul: case Div: return 4;
                        default:
                            Debugger::interface()->add_msg({
                                .level = debMsg::Fail,
                                .src_info = par->cur().addr,
                                .msg = std::format("[sorry]: '{}' is not supported;", (int)opt)
                            });
                            //TODO: [sorry]: '{operator}' is not supported;
                            throw std::runtime_error("[sorry]: '{operator}' is not supported;");
                    }
                }
            };
            auto mid_expr{AstTree::make_node<EAst::MidExpr>()};
            std::stack <AstNode<EAst::Expr>> expt;
            std::stack <mid_operator> opts;
            bool is_end = false;
            auto curOpt = [&]() -> mid_operator {
                switch (cur().kind) {
                case ETokenKind('+'): return mid_operator(Add, 1);
                case ETokenKind('-'): return mid_operator(Sub, 1);
                case ETokenKind('*'): return mid_operator(Mul,1);
                case ETokenKind('/'): return mid_operator(Div,1);
                case ETokenKind('='): {
                    if (lex().PeekToken(1).kind == ETokenKind('=')) {
                        return mid_operator(Equ, 2);
                    }
                    return mid_operator(Ass, 1);
                }
                case ETokenKind('<'): {
                    if (lex().PeekToken(1).kind == ETokenKind('=')) {
                        return mid_operator(lEq, 2);
                    }
                    return mid_operator(Lss, 1);
                }
                case ETokenKind('>'): {
                    if (lex().PeekToken(1).kind == ETokenKind('=')) {
                        return mid_operator(gEq, 2);
                    }
                    return mid_operator(Gtr, 1);
                }
                case ETokenKind('!'): {
                    if (lex().PeekToken(1).kind == ETokenKind('=')) {
                        return mid_operator(nEq, 2);
                    }
                }
                default: return mid_operator(UKn, 0);
                }
            };
            while (!is_end) {
                if (auto unary{ParseUnaryExpr()}; unary == nullptr) {
                    // cur() is a character
                    auto opt{mid_operator(curOpt())};
                    if (opt.opt == UKn) {
                        is_end = true;
                        if (opts.empty()) {
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
                        auto i = opt.chr_cnt;
                        while (i--) {
                            lex().NextToken();
                        }
                    }
                } else { expt.push(unary); }
            }
            if (expt.empty()) {
                Debugger::interface()->add_msg({
                   .level=debMsg::Level::Fail,
                   .src_info=cur().addr,
                   .msg=std::format("here need a expression")
                });
                return nullptr;
            }
            return expt.top();
        }
        AstNode<EAst::Block> ParseBlock() {
            auto blck = AstTree::make_node<EAst::Block>();
            ExpectToken(ETokenKind('{'));
            while (cur().kind != ETokenKind::Eof && cur().kind != ETokenKind('}')) {
                blck->stmts.push_back(ParseStmt());
            }
            ExpectToken(ETokenKind('}'));
            return blck;
        }
        AstNode<EAst::ExprStmt> ParseExprStmt() {
            auto expr_stmt = AstTree::make_node<EAst::ExprStmt>();
            if (cur().kind != ETokenKind(';')) {
                expr_stmt->expr = ParseExpr();
            }
            ExpectToken(ETokenKind(';'));
            return expr_stmt;
        }
        AstNode<EAst::If_Else> ParseIfStmt() {
            auto if_else = AstTree::make_node<EAst::If_Else>();
            lex().NextToken();
            ExpectToken(ETokenKind('('));
            if_else->cond = ParseExpr();
            ExpectToken(ETokenKind(')'));
            if_else->ifS = ParseStmt();
            if (exchange_key() == KeyDef::Else) {
                lex().NextToken();
                if_else->elseS = ParseStmt();
            }
            return if_else;
        }
        AstNode<EAst::For> ParseForStmt() {
            auto fr = AstTree::make_node<EAst::For>();
            lex().NextToken();
            ExpectToken(ETokenKind('('));
            if (cur().kind != ETokenKind(';'))
                fr->init = ParseExpr();
            ExpectToken(ETokenKind(';'));
            if (cur().kind != ETokenKind(';'))
                fr->cond = ParseExpr();
            ExpectToken(ETokenKind(';'));
            if (cur().kind != ETokenKind(')'))
                fr->step = ParseExpr();
            ExpectToken(ETokenKind(')'));
            fr->then = ParseStmt();
            return fr;
        }
        AstNode<EAst::While> ParseWhileStmt() {
            auto whl = AstTree::make_node<EAst::While>();
            lex().NextToken();
            ExpectToken(ETokenKind('('));
            whl->cond = ParseExpr();
            ExpectToken(ETokenKind(')'));
            whl->then = ParseStmt();
            return whl;
        }
        AstNode<EAst::Do_While> ParseDoWhileStmt() {
            auto do_whl = AstTree::make_node<EAst::Do_While>();
            lex().NextToken();
            do_whl->then = ParseStmt();
            ExpectToken(ETokenKind((signed)ETokenKind::Key + (signed)(KeyDef::While)));
            ExpectToken(ETokenKind('('));
            do_whl->cond = ParseExpr();
            ExpectToken(ETokenKind(')'));
            return do_whl;
        }
        AstNode<EAst::Ret> ParseRetStmt() {
            auto ret = AstTree::make_node<EAst::Ret>();
            lex().NextToken();
            ret->expr = ParseExpr();
            ExpectToken(ETokenKind(';'));
            return ret;
        }

    };
}