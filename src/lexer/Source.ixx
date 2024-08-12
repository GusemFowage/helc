export module lexer.source;

import <string>;
import <limits>;

export import defint;

#include "attrdef.hxx"

export namespace hel {
    struct Source {
    public:
        virtual ~Source() = default;
        enum Type {
            String, File, Inline
        };
        struct Info {
            size_t line, column;
            string_view code;
        };
        NODISCARD virtual bool had_end() const = 0;
        virtual char_t NextChar() = 0;
        virtual char_t PeekChar(pos_t pos)  = 0;
        NODISCARD Info cur_line() const { return mCurLineInfo; }
    protected:
        explicit Source() = default;
        Info mCurLineInfo{.line={}, .column={}, .code={}};
    };

    template<Source::Type sTp>
    class SourceImpl;

    template<>
    class SourceImpl<Source::String> : public Source {
    private:
        string code;
        size_t index;
    public:
        explicit SourceImpl(string_view str)
                : code(str), index(0) {
            mCurLineInfo.code = code;
        }
        NODISCARD bool had_end()
        const override { return (index >= code.size()); }
        char_t NextChar() override {
            if (index >= code.size())
                return char_info::eof();
            mCurLineInfo.column += 1;
            return code[index++];
        }
        char_t PeekChar(pos_t pos) override {
            if (index + pos < 0 || index + pos >= code.size())
                return char_info::eof();
            return code.at(index+pos);
        }
    };

    template<>
    class SourceImpl<Source::File> : public Source {

    };
    template<>
    class SourceImpl<Source::Inline> : public Source {

    };

    namespace detail {
        using namespace std;
        template<class SrcTp> struct is_source_impl
            : public is_base_of<Source, SrcTp> { };

        template<Source::Type t>
        struct is_source_impl<SourceImpl<t>> : true_type {
            static constexpr bool willend = false;
            static constexpr Source::Type type = t;
        };

        template<>
        struct is_source_impl<SourceImpl<Source::String>> : true_type {
            static constexpr bool willend = true;
            static constexpr Source::Type type = Source::String;
        };
    }
}