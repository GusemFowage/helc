export module lexer;

export import lexer.source;

export import hc.keywords;

import <list>;
import <memory>;
import <functional>;
import <variant>;

#include "attrdef.hxx"

template<class sT>
concept SrcType = hel::detail::is_source_impl<sT>::value;

template<class ... Ts>
struct token_types {
    explicit consteval token_types(hel::typelist<Ts...>) {}
    using variant = std::variant<Ts...>;
};
constexpr token_types TokenInfoImpl(
    hel::make_unique_typelist<bool, chr_t, num_t, str_t, tag_t/*, doc_t*/>{}
);
using token_info = decltype(TokenInfoImpl);

export namespace hel{
    enum class ETokenKind : signed {
        Flr = char_info::flr(),
        Eof = char_info::eof(),
        Rng = char_info::rng() + 1, // as unknown
        Annotation,
        Num, Chr,
        Str, Tag, /*...things to add*/
        Key, /*key...*/
    };
    struct Token {
        Source::Info addr;
        ETokenKind kind;
        token_info::variant val;
//        union {
//            bool  bl;   //boolean
//            chr_t chr;  //character
//            num_t num;  //number
//            str_t str;  //string
//            tag_t tag;  //tag
//        };
        explicit Token(const Source::Info& iAddr)
            : addr(iAddr), kind(ETokenKind::Eof) {
            val.emplace<chr_t>(char_info::eof());
        }
        template<class Tp>
        const Tp& get() const {
            return std::get<Tp>(val);
        }
    };
    class Lexer {
        std::list<Token> tokenQueue;
        std::shared_ptr<Source> src;
    public:
        template<SrcType SrcTp>
            explicit Lexer(const SrcTp& iSrc) noexcept : src(&iSrc) {}
        template<SrcType SrcTp>
            explicit Lexer(SrcTp* iSrc) noexcept : src(iSrc) {}
        template<SrcType SrcTp>
            explicit Lexer(auto&&...args) : src(new SrcTp(std::forward<decltype(args)>(args)...)) {}
        NODISCARD char_t cur() const {return src->PeekChar(0);}
        Token NextToken() {
            if (tokenQueue.empty()) return GainToken();
            auto sv = tokenQueue.front();
            tokenQueue.pop_front();
            return sv;
        }
        /* tokenQueue.size() = 3
         * p = 3            p = 2
         * 1 2 3 [4]        1 2 [3]
         * ^ . .  .         ^ .  .
         */
        const Token& PeekToken(upos_t p = 0) {
            if (tokenQueue.size()<=p) {
                auto t{p-tokenQueue.size()+1};
                while (t--) {
                    tokenQueue.push_back(GainToken());
                }
                return tokenQueue.back();
            }
            auto it = tokenQueue.begin();
            while (p--) it++;
            return std::move(*it);
        }
    private:
        void SkipWhite() {
            while (char_info::is_space(cur()) || cur() == '/') {
                if (cur() == '/') {
                    if (auto pc{src->PeekChar(1)};pc=='/' || pc=='*') {
                        src->NextChar();
//                    ret.kind = ETokenKind::Annotation;
//                    ret.val.emplace<doc_t>(LexDocs());
                        LexDocs();
                    } else return;
                } else
                    src->NextChar();
            }

        }
        Token GainToken() {
            SkipWhite();
            Token ret{src->cur_line()};
            if (char_info::is_digit(cur())) {
                ret.kind = ETokenKind::Num;
                ret.val.emplace<num_t>(LexToken<ETokenKind::Num>());
            } else if (is_ident(cur())) {
                ret.val.emplace<tag_t>(LexToken<ETokenKind::Tag>());
                if (auto f{key_table.find(std::get<tag_t>(ret.val))}; f == key_table.end()) {
                    ret.kind = ETokenKind::Tag;
                } else {
                    auto& [key, idx] {*f};
                    // debug() << key << " " << idx << '\n';
                    ret.kind = ETokenKind(static_cast<signed >(ETokenKind::Key) + static_cast<signed >(idx));
                }
            } else if (cur() == '"') {
                ret.kind=ETokenKind::Str;
                ret.val.emplace<str_t>(LexToken<ETokenKind::Str>());
                src->NextChar();
            } else switch (cur()) {
                case char_info::eof():
                    ret.kind = ETokenKind::Eof;
                    break;
                default:
                    auto& c = ret.val.emplace<chr_t>(cur());
                    ret.kind = ETokenKind(c);
                    src->NextChar();
                    break;
            }
            return ret;
        }
        template<ETokenKind kind>
        static auto tokenRequires(char_t c, auto& r) -> bool {
            if constexpr (kind == ETokenKind::Num) {
                return (char_info::is_digit(c) || c == '.');
            } else if constexpr (kind == ETokenKind::Tag || kind == ETokenKind::Key) {
                return (is_ident(c));
            } else if constexpr (kind == ETokenKind::Str) {
                if (c != '"')
                    return true;
                else r+=1;
                    return false;
            }
        }
        template<ETokenKind kind>
        string_view LexToken() {
            auto l=src->cur_line().column;
            decltype(l) r{};
            do {
                r ++;
                src->NextChar();
            } while (tokenRequires<kind>(cur(), r));
            return src->cur_line().code.substr(l, r);
        }
        void LexDocs() {
            // TODO: doc_t
//            doc_t retDocs;
            auto c = src->NextChar();
//            auto left{src->cur_line().column};
//            decltype(left) right{};
            if (c == '/') {
                // single-line annotation
                while (c != char_info::eof() &&c != '\n') {
//                    right++;
                    c = src->NextChar();
                }
//                retDocs.push_back(src->cur_line().code.substr(left, right));
            } else {
                // lines annotation
                while (true) {
                    if (c == '\n') {
//                        retDocs.push_back(src->cur_line().code.substr(left, right));
                        c = src->NextChar();
//                        left = src->cur_line().column, right = {};
                        continue;
                    } else if (cur() == '*' && src->PeekChar(1) == '/') {
//                        retDocs.push_back(src->cur_line().code.substr(left, right));
                        break;
                    }
//                    right++;
                    c = src->NextChar();
                }
                src->NextChar(); src->NextChar();
            }
//            return retDocs;
        }
    };

}
