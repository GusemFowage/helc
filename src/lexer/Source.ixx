export module lexer.source;

import <string>;
import <limits>;
import <filesystem>;
import <list>;
import <fstream>;

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
    class SourceImpl<Source::File> : public Source{
        std::list<string> code;
        decltype(code)::iterator curL;
        string::iterator curC;
        std::filesystem::path SrcPath;
    public:
        explicit SourceImpl(const decltype(SrcPath)& src) : SrcPath(src) {
            std::ifstream fin;
            fin.open(SrcPath);
            string buff;
            code.push_back("//"+SrcPath.string());
            while (std::getline(fin, buff)) {
                code.push_back(buff);
            }
            fin.close();

            curL = code.begin();
            curC = curL->begin();
            mCurLineInfo.line = {};
            mCurLineInfo.column = {};
            mCurLineInfo.code = *curL;
        }
        bool had_end() const override{
            return (curL==code.end());
        }
        char_t NextChar() override{
            if (had_end()) return char_info::eof();
            if (curC == curL->end()) {
                curL ++;
                mCurLineInfo.line++;
                if (curL == code.end()) {
                    return '\n';
                }
                curC = curL->begin();
                mCurLineInfo.column = 0;
                mCurLineInfo.code=*curL;
                return '\n';
            }
            mCurLineInfo.column++;
            return (*curC++);
        }
        char_t PeekChar(hel::pos_t pos) override{
            if (had_end()) return char_info::eof();
            if (curC != curL->end() && pos == 0) return *curC;
            auto tmpLn = curL;
            auto tmpCh = curC;
            decltype(pos) tmp;
            while ((tmp = (tmpLn->end() - tmpCh)) <= pos) {
                pos -= tmp;
                if (pos==0) {
                    return '\n';
                }
                tmpLn++;
                if (tmpLn == code.end()) {
                    return char_info::eof();
                }
                tmpCh = tmpLn->begin();
                pos--;
            }
            tmpCh += pos;
            return *tmpCh;
        }
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