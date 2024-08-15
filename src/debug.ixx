export module debug;

import lexer;
import parser.ast;

import <iostream>;
using namespace std;

export namespace hel :: deb {
    class Check : public AstVisitor{
    public:
        void visit(Ast<EAst::Pragma> & prg) override {
            for (auto& i : prg.funcs) {
                i->accept(this);
            }
        }
        void visit(Ast<EAst::FuncDef> & fnc_def) override {
            unsigned stack_size = 0;
            for (auto& v : fnc_def.local_def.localVars) {
                stack_size += 8;
                v->offset = stack_size;
            }
            cout << fnc_def.nm << "(";
            for (auto& i : fnc_def.args) {
                cout << i->ident << " ";
            }
            cout << ")\n";
            for (auto& i : fnc_def.stmts) {
                i->accept(this);
            }
        }
        void visit(Ast<EAst::ConNumb> & num) override {
            cout << (num.num);
        }
        void visit(Ast<EAst::MutVar> & var) override {
            cout << var.varObj->ident;
        }
        void visit(Ast<EAst::Call> & cll) override {
            cout << cll.nm << "(";
            for (auto& i : cll.args) {
                i->accept(this);
                cout << ',';
            }
            cout << ")";
        }
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
                case Equ:
                    cout << "==";break;
                case lEq:
                    cout << "<=";break;
                case gEq:
                    cout << ">=";break;
                case Lss:
                    cout << '<';break;
                case Gtr:
                    cout << '>';break;
                default:
                    break;
            }
            if (mid.rhs != nullptr)
                mid.rhs->accept(this);
            cout << ')';
        }
        void visit(Ast<EAst::ExprStmt> & estmt) override {
            if (estmt.expr)
                estmt.expr->accept(this);
            cout << ";\n";
        }
        void visit(Ast<EAst::Block> & blck) override {
            for (auto& i : blck.stmts) {
                i->accept(this);
            }
        }
        void visit(Ast<EAst::If_Else> & if_else) override {
            cout << "if ";
            if_else.cond->accept(this);
            cout << "\n";
            if_else.ifS->accept(this);
            if (if_else.elseS != nullptr) {
                if_else.elseS->accept(this);
            }
        }
        void visit(Ast<EAst::While> & whl) override {
            cout << "while (";
            whl.cond->accept(this);
            cout << ")";
            whl.then->accept(this);
        }
        void visit(Ast<EAst::Do_While> & whl) override {
            cout << "do ";
            whl.then->accept(this);
            cout << "while (";
            whl.cond->accept(this);
            cout << ")";
        }
        void visit(Ast<EAst::For> & whl) override {
            cout << "for (";
            whl.init->accept(this);
            cout << ";";
            whl.cond->accept(this);
            cout << ";";
            whl.step->accept(this);
            cout << ")\n";
            whl.then->accept(this);
        }
        void visit(Ast<EAst::Ret> & rt) override {
            cout << "return ";
            rt.expr->accept(this);
            cout << '\n';
        }
    };

    void testSource() {
        cout << boolalpha << detail::is_source_impl < SourceImpl < Source::String >> ::value << endl;
        cout << boolalpha << detail::is_source_impl < SourceImpl < Source::String >> ::willend << endl;
        cout << (int) detail::is_source_impl < SourceImpl < Source::String >> ::type << endl;
    }

    void testLexer() {
//        Lexer lex(new SourceImpl<Source::String>("\n123.456 1. int helloworld \" __ hello world __\" + - * / // hello world\n /*this \n lee*/"));
        Lexer lex(new SourceImpl<Source::File>("../../test.c"));
        while (lex.PeekToken(0).kind != ETokenKind::Eof) {
            if (lex.PeekToken(0).kind <= hel::ETokenKind::Rng)
                cout << get<chr_t>(lex.NextToken().val) << '\n';
            else if (lex.PeekToken(0).kind == hel::ETokenKind::Num)
                cout << get<num_t>(lex.NextToken().val) << '\n';
            else if (lex.PeekToken(0).kind == hel::ETokenKind::Str)
                cout << get<str_t>(lex.NextToken().val) << '\n';
//            else if (lex.PeekToken(0).kind == hel::ETokenKind::Annotation) {
//                for (auto i: get<doc_t>(lex.PeekToken(0).val))
//                    cout << i;
//                cout << '\n';
//                lex.NextToken();
//            }
            else
                cout << (int)lex.PeekToken(0).kind << ": " << get<tag_t>(lex.NextToken().val) << '\n';
        }
    }
}
