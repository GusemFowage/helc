export module helloc;

import lexer;
import parser;
import generator;
import symbol.debugger;
import debug;

import <unordered_set>;
import <iostream>;
import <filesystem>;
import <fstream>;

using namespace std;
using namespace hel;

// 编译器真正的入口函数
export int hello(const unordered_multiset<string_view>& parameter) {
//    deb::testLexer();
//     a_=4+10*(10-2)/5; a_; 4 + 16 = 20
//    Lexer lex(new SourceImpl<Source::String>("a_ = (4+10*(10-2)/5) ; if() {a_ = 100;} else {a_ = a_ + 20;} a_ <= 40;"));
    Lexer lex(new SourceImpl<Source::File>("../../test.c"));
    Parser parser(lex);
    Debugger symbol_deb;
    auto tree = parser.get_tree();
//    deb::Check check;
//    tree.visit(check);
//    if (auto f{parameter.find("-o")}; (++f) != parameter.end()) {
//        auto pth{(std::filesystem::path(*f))};
//        if (pth.extension() != ".s" ) {
//            return -1;
//        }
//        std::ofstream fout(pth);
//        CodeGen codeGen(fout);
//        tree.visit(codeGen);
//    } else {
        CodeGen codeGen(cout);
        tree.visit(codeGen);
//    }
    symbol_deb.put_all();
    return 0;
}