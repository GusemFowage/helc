export module debug;

import parser.ast;

import <iostream>;
using namespace std;

export namespace hel :: deb {
    class Check : public AstVisitor{
    public:
        void visit(Ast<EAst::Pragma> & prg) override {
            for (auto& i : prg.stmts) {
                i->accept(this);
            }
        }
        void visit(Ast<EAst::ConNumb> & num) override {
            cout << (num.num);
        }
        void visit(Ast<EAst::MutVar> & var) override {
            cout << var.nm;
        }
        void visit(Ast<EAst::PriExpr> & pri) override {
            pri.rhs->accept(this);
        }
        void visit(Ast<EAst::MidExpr> & mid) override {
            using enum Ast<EAst::MidExpr>::MidOperator;
            cout << '(';
            if (mid.lhs != nullptr)
                mid.lhs->accept(this);
            switch (mid.opt)  {
                case Add:
                    cout << '+';
                    break;
                case Sub:
                    cout << '-';
                    break;
                case Mul:
                    cout << '*';
                    break;
                case Div:
                    cout << '/';
                    break;
                default:
                    break;
            }
            if (mid.rhs != nullptr)
                mid.rhs->accept(this);
            cout << ')';
        }
        void visit(Ast<EAst::ExprStmt> & estmt) override {
            estmt.expr->accept(this);
        }
        void visit(Ast<EAst::If_Else> & if_else) override {
            cout << "if ";
            if_else.condition->accept(this);
            cout << "\n";
            if_else.ifS->accept(this);
            if (if_else.elseS != nullptr) {
                if_else.elseS->accept(this);
            }
        }
    };
}