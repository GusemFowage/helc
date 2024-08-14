export module generator;

import defint;
import parser.ast;

import <format>;
import <fstream>;
import <optional>;
import <iostream>;
import <functional>;

export namespace hel {
    class CodeGen : public AstVisitor{
        mutable std::reference_wrapper<std::ostream> mOutput;
        size_t stackLevel{};
        bool use_var {true} ;
    public:
        std::ostream& out() const { return mOutput.get(); }
        explicit CodeGen(std::ostream & iOstream)
            : mOutput(iOstream) {}
        template <class ... Args>
        void genln(string_view msg, Args&&...reg) {
            out() << std::vformat(msg, std::make_format_args(std::forward<Args>(reg)...)) << '\n';
        }
    public:
        void visit(Ast<EAst::Pragma> & p) override {
            // pragma-text head
            genln("\t.text");
            // entry a function
            genln("\t.global prog");
            genln("prog:");
            genln("\tpush %rbp");
            genln("\tmov %rsp, %rbp");
            genln("\tsub $32, %rsp");
            size_t stack_size = 0;
            for (auto& v : p.local_def.localVars) {
                stack_size += 8;
                v->offset = stack_size;
            }
            for (auto& i : p.stmts) {
                i->accept(this);
            }
            // TODO: [code_gen]: stack level is not null; now the level is {stackLevel}
            if (stackLevel != 0)
                throw std::runtime_error("[code_gen]: stack level is not null; now the level is {stackLevel}");
            // leave from the function
            genln("\tmov %rbp, %rsp");
            genln("\tpop %rbp");
            genln("\tret");
        }
        void visit(Ast<EAst::ConNumb> & num) override {
            // assume num.num is an int;
            genln("\tmov ${}, %rax", num.num);
        }
        void visit(Ast<EAst::MutVar> & var) override {
            genln("\tlea -{}(%rbp), %rax", var.varObj->offset);
            if (use_var) genln("\tmov (%rax), %rax");
        }
        void visit(Ast<EAst::MidExpr> & mid) override {
            using enum Ast<EAst::MidExpr>::MidOperator;
            if (mid.opt == Ass) {
                use_var = false;
                mid.lhs->accept(this);
                use_var = true;
                Push();
                mid.rhs->accept(this);
                Pop("%rdi");
                genln("\tmov %rax, (%rdi)");
                return;
            }
            mid.rhs->accept(this);
            Push();
            mid.lhs->accept(this);
            Pop("%rdi");
            switch (mid.opt) {
                case Add:
                    out() << "\tadd %rdi, %rax\n";
                    break;
                case Sub:
                    out() << "\tsub %rdi, %rax\n";
                    break;
                case Mul:
                    out() << "\tmul %rdi\n";
                    break;
                case Div:
                    out() << "\tcqo\n";
                    out() << "\tidiv %rdi\n";
                    break;
                default:
                    break;
            }
        }
        void visit(Ast<EAst::ExprStmt> & es) override {
            es.expr->accept(this);
        }
        void visit(Ast<EAst::If_Else> &) override {

        }
    private:
        void Push() {
            genln("\tpush %rax");
            stackLevel++;
        }
        void Pop(const char* reg) {
            genln("\tpop {}", reg);
            stackLevel--;
        }
    };
}
