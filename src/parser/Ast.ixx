export module parser.ast;

import <list>;
import <memory>;
import <functional>;
import <set>;

import defint;
export import hc.types;

using std::shared_ptr;

export namespace hel {
    struct AstTree;
    struct AstVisitor;

    struct LocalDefines {
        std::set<shared_ptr<MutableValue>> localVars;
    };

    enum class EAst {
        Root,
        Pragma,
        Expr, Stmt,
        ExprStmt,
        If_Else, While,
        ConNumb, MutVar,
        PriExpr, MidExpr,
    };

    template<EAst eA = EAst::Root>
    struct Ast {
        friend class AstTree;
        friend class AstVisitor;
        virtual void accept(AstVisitor *vis) = 0;
    public:
        virtual ~Ast() = default;
    };
    template<EAst eAst>
    using AstNode = shared_ptr<Ast<eAst>>;
    template<>
    struct Ast<EAst::Expr> : Ast<EAst::Root> {

    };
    template<>
    struct Ast<EAst::Stmt> : Ast<EAst::Root> {};

    template<>
    struct Ast<EAst::Pragma> : public Ast<EAst::Root> {
        void accept(AstVisitor *vis) override;
        std::list<AstNode<EAst::Stmt>> stmts;
        LocalDefines local_def;
    };
    template<>
    struct Ast<EAst::ExprStmt> : public Ast<EAst::Stmt> {
        void accept(AstVisitor *vis) override;
        AstNode<EAst::Expr> expr;
    };
    template<>
    struct Ast<EAst::If_Else> : public Ast<EAst::Stmt> {
        void accept(AstVisitor *vis) override;
        AstNode<EAst::Expr> condition;
        AstNode<EAst::Stmt> ifS, elseS;
    };
    template<>
    struct Ast<EAst::While> : public Ast<EAst::Stmt> {

    };

    template<>
    struct Ast<EAst::PriExpr> : public Ast<EAst::Expr> {
        bool lvalue{false};
//        void accept(AstVisitor *vis) override;
//        enum class PriOperator {
//            Plus,
//        } opt{PriOperator::Plus};
    };
    template<>
    struct Ast<EAst::MidExpr> : public Ast<EAst::PriExpr> {
        void accept(AstVisitor *vis) override;
        enum class MidOperator {
            UKn = 0,        // unknown
            Com,            // comma
            Mul, Div, Mod,  // * / %
            Add, Sub,       // + -
            Ass,            // =
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

    struct AstTree {
        friend class Parser;
        void visit(AstVisitor& iVisitor) {
            vis = &iVisitor;
            root->accept(vis);
        }
        template<EAst eAst>
        static AstNode<eAst> make_node() {
            return std::make_shared<Ast<eAst>>();
        }
    private:
        AstNode<EAst::Root> root;
        AstVisitor* vis;
    };

    struct AstVisitor {
//        virtual void visit(AstNode<EAst::Root>&) = 0;
        virtual void visit(Ast<EAst::Pragma>&) = 0;

        virtual void visit(Ast<EAst::ConNumb>&) = 0;
        virtual void visit(Ast<EAst::MutVar>&) = 0;

//        virtual void visit(Ast<EAst::PriExpr>&) = 0;
        virtual void visit(Ast<EAst::MidExpr>&) = 0;

        virtual void visit(Ast<EAst::ExprStmt>&) = 0;
        virtual void visit(Ast<EAst::If_Else>&) = 0;
    };

    void Ast<EAst::Pragma>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::ConNumb>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::MutVar>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
//    void Ast<EAst::PriExpr>::accept(AstVisitor *vis) {
//        vis->visit(*this);
//    }
    void Ast<EAst::MidExpr>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::ExprStmt>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
    void Ast<EAst::If_Else>::accept(AstVisitor *vis) {
        vis->visit(*this);
    }
}
