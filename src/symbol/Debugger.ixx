export module symbol.debugger;

import <queue>;
import <iostream>;
import <memory>;
import <format>;

import defint;
import lexer.source;

export namespace hel {
    struct debMsg {
        enum Level {
            Note, Info,
            Warn,
            Error,
            Fail
        } level;
        Source::Info src_info;
        string msg;
    };
}

template<>
struct ::std::formatter<hel::debMsg::Level, hel::char_info::char_type> {
    static auto parse(std::format_parse_context& parseContext) {
        auto symbolsEnd = std::ranges::find(parseContext, '}');
        auto symbols = std::string_view(parseContext.begin(), symbolsEnd);
//            std::cout << "parse(" << symbols << ")" << std::endl;
        return symbolsEnd;
    }

    static auto format(hel::debMsg::Level const& lvl, std::format_context& formatContext) {
        string_view lvl_str;
        switch (lvl) {
            case hel::debMsg::Note:
                lvl_str = "Note"; break;
            case hel::debMsg::Info:
                lvl_str = "Info"; break;
            case hel::debMsg::Warn:
                lvl_str = "Warn"; break;
            case hel::debMsg::Error:
                lvl_str = "Error"; break;
            case hel::debMsg::Fail:
                lvl_str = "Fail"; break;
        }
        return std::format_to(formatContext.out(), "[{}]", lvl_str);
    }
};

export namespace hel {
    class Debugger {
        std::queue <debMsg> msgs;
    public:
        Debugger();
        ~Debugger();
        void use();
        void add_msg(const debMsg &msg) {
            msgs.push(msg);
        }

        void put_all() {
            while (!msgs.empty()) {
                auto &front = msgs.front();
                std::cout << front.src_info.code << '\n';
                std::println(std::cout, "{1:~>{0}}{2}: {3}", front.src_info.column, '^', front.level, front.msg);
                msgs.pop();
            }
        }

        static Debugger *interface();
    };
}

namespace hel {
    inline static std::atomic<Debugger*> singleton=nullptr;
    void Debugger::use() {
        singleton.store(this);
    }
    Debugger::Debugger() {
        use();
    }
    Debugger *Debugger::interface()  {
        return singleton.load();
    }
    Debugger::~Debugger()  {
        if (!msgs.empty()) put_all();
        singleton.store(nullptr);
    }
}