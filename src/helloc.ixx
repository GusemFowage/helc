export module helloc;

import lexer;
import parser;
import debug;

import <unordered_set>;
import <iostream>;

using namespace std;
using namespace hel;

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

// 编译器真正的入口函数
export int hello(const unordered_multiset<string_view>& parameter) {
    Lexer lex(new SourceImpl<Source::String>("if( 6*(13+28*5/4-9)-7 ) 10 + a_; \n"));
    Parser parser(lex);
    auto tree = parser.get_tree();
    deb::Check check;
    tree.visit(check);
    return 0;
}