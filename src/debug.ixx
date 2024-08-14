export module debug;

import lexer;
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
            cout << var.varObj->ident;
        }
//        void visit(Ast<EAst::PriExpr> & pri) override {
//            pri.lhs->accept(this);
//        }
        void visit(Ast<EAst::MidExpr> & mid) override {
            using enum Ast<EAst::MidExpr>::MidOperator;
            cout << '(';
            if (mid.lhs != nullptr)
                mid.lhs->accept(this);
            switch (mid.opt)  {
                case Add:
                    cout << '+'; break;
                case Sub:
                    cout << '-'; break;
                case Mul:
                    cout << '*'; break;
                case Div:
                    cout << '/'; break;
                case Ass:
                    cout << '='; break;
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

    void testSource() {
        cout << boolalpha << detail::is_source_impl < SourceImpl < Source::String >> ::value << endl;
        cout << boolalpha << detail::is_source_impl < SourceImpl < Source::String >> ::willend << endl;
        cout << (int) detail::is_source_impl < SourceImpl < Source::String >> ::type << endl;
    }

    void testLexer() {
        Lexer lex(new SourceImpl<Source::String>("\n123.456 1. int helloworld \" __ hello world __\" + - * / // hello world\n /*this \n lee*/"));
        while (lex.PeekToken(0).kind != ETokenKind::Eof) {
            if (lex.PeekToken(0).kind <= hel::ETokenKind::Rng)
                cout << get<chr_t>(lex.NextToken().val) << '\n';
            else if (lex.PeekToken(0).kind == hel::ETokenKind::Num)
                cout << get<num_t>(lex.NextToken().val) << '\n';
            else if (lex.PeekToken(0).kind == hel::ETokenKind::Str)
                cout << get<str_t>(lex.NextToken().val) << '\n';
            else if (lex.PeekToken(0).kind == hel::ETokenKind::Annotation) {
                for (auto i: get<doc_t>(lex.PeekToken(0).val))
                    cout << i;
                cout << '\n';
                lex.NextToken();
            }
            else
                cout << (int)lex.PeekToken(0).kind << ": " << get<tag_t>(lex.NextToken().val) << '\n';
        }
    }
}
