export module parser.ast;

import <list>;
import <memory>;
import <functional>;
import <set>;
import <unordered_set>;

import defint;
export import hc.types;

using std::shared_ptr;

export namespace hel {
    struct AstTree;
    struct AstVisitor;

    struct LocalDefines {
        std::list<shared_ptr<MutableValue>> localVars;
    };

    enum class EAst {
        Root, Pragma,
        Expr, Stmt,
        FuncDef, Block, Decl,
        ExprStmt, If_Else,
        For, While, Do_While,
        ConNumb, MutVar,
        PriExpr, MidExpr, UnaryExpr,
        Call, Ret, StmtExpr
    };

    template<EAst eA = EAst::Root>
    struct Ast {
        friend class AstTree;
        friend class AstVisitor;
        shared_ptr<Type> ty;
        virtual void accept(AstVisitor *vis) = 0;
    public:
        virtual ~Ast() = default;
    };
    template<EAst eAst>
    using AstNode = shared_ptr<Ast<eAst>>;
    template<>
    struct Ast<EAst::Expr> : Ast<EAst::Root> {};
    template<>
    struct Ast<EAst::Stmt> : Ast<EAst::Root> {};

    template<>
    struct Ast<EAst::Pragma> : public Ast<EAst::Root> {
        void accept(AstVisitor *vis) override;
        std::list<AstNode<EAst::Root>> funcs;
    };
    template<>
    struct Ast<EAst::FuncDef> : public Ast<EAst::Expr> {
        void accept(hel::AstVisitor *vis) override;
        string_view nm;
        std::list<shared_ptr<MutableValue>> args;
        LocalDefines local_def;
        std::list<AstNode<EAst::Stmt>> stmts;
    };
    template<>
    struct Ast<EAst::Block> : public Ast<EAst::Stmt> {
        void accept(AstVisitor *vis) override;
        std::list<AstNode<EAst::Stmt>> stmts;
    };
    template<>
    struct Ast<EAst::ExprStmt> : public Ast<EAst::Stmt> {
        void accept(AstVisitor *vis) override;
        AstNode<EAst::Expr> expr;
    };
    template<>
    struct Ast<EAst::If_Else> : public Ast<EAst::Stmt> {
        void accept(AstVisitor *vis) override;
        AstNode<EAst::Expr> cond;
        AstNode<EAst::Stmt> ifS, elseS;
    };
    template<>
    struct Ast<EAst::While> : public Ast<EAst::Stmt> {
        void accept(hel::AstVisitor *vis) override;
        AstNode<EAst::Expr> cond;
        AstNode<EAst::Stmt> then;
    };
    template<>
    struct Ast<EAst::Do_While> : public Ast<EAst::While> {
        void accept(hel::AstVisitor *vis) override;
    };
    template<>
    struct Ast<EAst::For> : public Ast<EAst::While> {
        void accept(hel::AstVisitor *vis) override;
        AstNode<EAst::Expr> init{nullptr}, step{nullptr};
        AstNode<EAst::Stmt> then;
    };
    template<>
    struct Ast<EAst::Ret> : public Ast<EAst::Stmt> {
        void accept(hel::AstVisitor *vis) override;
        AstNode<EAst::Expr> expr;
    };
    template<>
    struct Ast<EAst::PriExpr> : public Ast<EAst::Expr> {
        bool lvalue{false};

    };
    template<>
    struct Ast<EAst::UnaryExpr> : public Ast<EAst::PriExpr>{
        void accept(AstVisitor *vis) override;
        enum class UnaryOperator {
            Ukn,
            Plus, Minus, Fetch, GetAddr
        } opt{UnaryOperator::Ukn};
        AstNode<EAst::Expr> expr;
    };
    template<>
    struct Ast<EAst::MidExpr> : public Ast<EAst::PriExpr> {
        void accept(AstVisitor *vis) override;
        enum class MidOperator {
            UKn = 0,        // unknown
            Com,            // ,
            Mul, Div, Mod,  // * / %
            Add, Sub,       // + -
            Ass,            // =
            Equ, nEq,       // == !=
            Lss, Gtr,       // < >
            lEq, gEq,       // <= >=
        } opt{MidOperator::UKn};
        AstNode<EAst::Expr> lhs{nullptr}, rhs{nullptr};
    };
    template<>
    struct Ast<EAst::ConNumb> : public Ast<EAst::PriExpr> {
        void accept(AstVisitor *vis) override;
        long double num{};
    };
    template<>
    struct Ast<EAst::MutVar> : public Ast<EAst::PriExpr> {
        void accept(AstVisitor *vis) override;
        shared_ptr<MutableValue> varObj;
    };
    template<>
    struct Ast<EAst::Call> : public Ast<EAst::PriExpr> {
        void accept(hel::AstVisitor *vis) override;
        string_view nm;
        std::list<AstNode<EAst::Expr>> args;
    };
    template<>
    struct Ast<EAst::StmtExpr> : public Ast<EAst::PriExpr> {
        void accept(hel::AstVisitor *vis) override;
        std::list<AstNode<EAst::Stmt>> stmts;
    };

    template<>
    struct Ast<EAst::Decl> : public Ast<EAst::Stmt> {
    public:
        std::list<AstNode<EAst::MidExpr>> ass;
        void accept(hel::AstVisitor *vis) override;
    };

    struct AstTree {
        friend class Parser;
        void visit(AstVisitor& iVisitor) {
            vis = &iVisitor;
            root->accept(vis);
        }
        template<EAst eAst>
        static shared_ptr <Ast<eAst>> make_node() {
            return std::make_shared<Ast<eAst>>();
        }
    private:
        AstNode<EAst::Root> root;
        AstVisitor* vis;
    };

    struct AstVisitor {
        virtual ~AstVisitor() = default;
//        virtual void visit(AstNode<EAst::Root>&) = 0;
        virtual void visit(Ast<EAst::Pragma>&) = 0;
        virtual void visit(Ast<EAst::FuncDef>&) = 0;

        virtual void visit(Ast<EAst::ConNumb>&) = 0;
        virtual void visit(Ast<EAst::MutVar>&) = 0;
        virtual void visit(Ast<EAst::UnaryExpr>&) = 0;
        virtual void visit(Ast<EAst::MidExpr>&) = 0;
        virtual void visit(Ast<EAst::Call>&) = 0;
        virtual void visit(Ast<EAst::StmtExpr>&) = 0;

        virtual void visit(Ast<EAst::Block>&) = 0;
        virtual void visit(Ast<EAst::Decl>&) = 0;
        virtual void visit(Ast<EAst::ExprStmt>&) = 0;
        virtual void visit(Ast<EAst::If_Else>&) = 0;
        virtual void visit(Ast<EAst::For>&) = 0;
        virtual void visit(Ast<EAst::While>&) = 0;
        virtual void visit(Ast<EAst::Do_While>&) = 0;
        virtual void visit(Ast<EAst::Ret>&) = 0;
    };

    void Ast<EAst::Pragma>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::FuncDef>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::ConNumb>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::MutVar>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::UnaryExpr>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::MidExpr>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::Call>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::StmtExpr>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::Block>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::Decl>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::ExprStmt>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::If_Else>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::For>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::While>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::Do_While>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::Ret>::accept(hel::AstVisitor *vis) {
        vis->visit(*this);
    }
}
