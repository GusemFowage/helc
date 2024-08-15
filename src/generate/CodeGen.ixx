export module generator;

import defint;
import parser.ast;

import <format>;
import <fstream>;
import <optional>;
import <iostream>;
import <functional>;
import <array>;

export namespace hel {
    class CodeGen : public AstVisitor{
        std::reference_wrapper<std::ostream> mOutput;
        size_t stackLevel{};
        bool use_var {true};
        unsigned Sequence{};
        string_view FuncNm;

        inline static const char* Reg64[6] = {
            "%rdi", "%rsi", "%rdx",
            "%rcx", "%r8", "%r9"
        };

    public:
        std::ostream& out() { return mOutput.get(); }
        explicit CodeGen(std::ostream & iOstream)
                : mOutput(iOstream) {}
        template <class ... Args>
        void genln(string_view msg, Args&&...reg) {
            // endl to flush the out stream
            out() << std::vformat(msg, std::make_format_args(std::forward<Args>(reg)...)) << std::endl;
        }
    public:
        void visit(Ast<EAst::Pragma> & p) override {
            for (auto& f : p.funcs) {
                f->accept(this);
            }
        }
        void visit(Ast<EAst::FuncDef> & fnc) override {
            // entry a function
            genln("\t.text");
            FuncNm = fnc.nm;
            genln("\t.global {}", FuncNm);
            genln("{}:", FuncNm);
            unsigned stack_size = 0;
            for (auto& v : fnc.local_def.localVars) {
                stack_size += 8;
                v->offset = stack_size;
            }
            stack_size = AlignTo(stack_size, 16);

            genln("\tpush %rbp");
            genln("\tmov %rsp, %rbp");
            genln("\tsub ${}, %rsp", stack_size);

            if (int i{0}; fnc.args.size() < 6) {
                for (auto& a : fnc.args) {
                    genln("\tmov {}, -{}(%rbp)", Reg64[i++], a->offset);
                }
            }

            for (auto& i : fnc.stmts) {
                i->accept(this);
                // TODO: [code_gen]: stack level is not null; now the level is {stackLevel}
                if (stackLevel != 0)
                    throw std::runtime_error("[code_gen]: stack level is not null; now the level is {stackLevel}");
            }
            // leave from the function
            genln(".L.leave_{}:", FuncNm);
            genln("\tmov %rbp, %rsp");
            genln("\tpop %rbp");
            genln("\tret");
        }
        void visit(Ast<EAst::ConNumb> & num) override {
            // assume num.num is an int;
            genln("\tmov ${}, %rax", num.num);
        }
        void visit(Ast<EAst::MutVar> & var) override {
//            if (var.varObj->offset == 16) {
//                genln("\tlea -{}(%rbp), %rax", var.varObj->offset);
//                if (use_var) genln("\tmov (%rax), %rax");
//                return;
//            }
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
                    genln("\tadd %rdi, %rax");
                    break;
                case Sub:
                    genln("\tsub %rdi, %rax");
                    break;
                case Mul:
                    genln("\tmul %rdi");
                    break;
                case Div:
                    genln("\tcqo");
                    genln("\tidiv %rdi");
                    break;
                case Equ: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsete %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                case nEq: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsetne %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                case Lss: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsetl %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                case Gtr: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsetg %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                case lEq: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsetle %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                case gEq: {
                    genln("\tcmp %rdi, %rax");
                    genln("\tsetge %al");
                    genln("\tmovzb %al, %rax");
                }
                    break;
                default:
                    break;
            }
        }
        void visit(Ast<EAst::Call> & cll) override {
            for (auto& a : cll.args) {
                a->accept(this);
                Push();
            }
            for (auto i{cll.args.size()}; i > 0; i--) {
                Pop(Reg64[i-1]);
            }
            genln("\tcall {}", cll.nm);
        }
        void visit(Ast<EAst::Block> & blk) override {
            for (auto& i : blk.stmts) {
                i->accept(this);
            }
        }
        void visit(Ast<EAst::ExprStmt> & es) override {
            if (es.expr)
                es.expr->accept(this);
        }
        void visit(Ast<EAst::If_Else> & elif) override {
            if (elif.cond) elif.cond->accept(this);
            auto n = Sequence++;
            genln("\tcmp $0, %rax");
            if (elif.elseS) {
                genln("\tje .L.else_{}", n);
            } else {
                genln("\tje .L.end_{}", n);
            }
            if (elif.ifS) elif.ifS->accept(this);
            genln("\tjmp .L.end_{}", n);
            if (elif.elseS) {
                genln(".L.else_{}:", n);
                elif.elseS->accept(this);
            }
            genln(".L.end_{}:", n);
        }
        void visit(Ast<EAst::While> & whl) override {
            auto n = Sequence++;
            genln(".L.begin_{}:", n);
            whl.cond->accept(this);
            genln("\tcmp $0, %rax");
            genln("\tje .L.end_{}", n);
            whl.then->accept(this);
            genln("\tjmp .L.begin_{}", n);
            genln(".L.end_{}:", n);
        }
        void visit(Ast<EAst::Do_While> & do_whl) override {
            auto n = Sequence++;
            genln(".L.begin_{}:", n);
            do_whl.then->accept(this);
            do_whl.cond->accept(this);
            genln("\tcmp $0, %rax");
            genln("\tje .L.end_{}", n);
            genln("\tjmp .L.begin_{}", n);
            genln(".L.end_{}:", n);
        }
        void visit(Ast<EAst::For> & fr) override {
            auto n = Sequence++;
            if (fr.init) fr.init->accept(this);
            genln(".L.begin_{}:", n);
            if (fr.cond) {
                fr.cond->accept(this);
                genln("\tcmp $0, %rax");
                genln("\tje .L.end_{}", n);
            }
            fr.then->accept(this);
            if (fr.step) fr.step->accept(this);
            genln("\tjmp .L.begin_{}", n);
            genln(".L.end_{}:", n);
        }
        void visit(Ast<EAst::Ret> & ret) override {
            ret.expr->accept(this);
            genln("\tjmp .L.leave_{}", FuncNm);
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
//        template<unsigned >
        static unsigned AlignTo(unsigned sz, unsigned al) {
            return (sz + al - 1) / al * al;
        }
    };
}
