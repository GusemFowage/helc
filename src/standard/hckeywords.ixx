export module hc.keywords;

export import defint;

import <map>;

#include "attrdef.hxx"

export namespace hel {

    ALWAYS_INLINE constexpr bool is_ident(char_t c) {
        return (char_info::is_digit(c) || char_info::is_alpha(c) || c == '_');
    }

    enum class KeyDef : unsigned {
        Ukn = 0,
        Auto,
        Int, Float,
        True, False, Null,
        If, Else,
        Do, While, For,
        Return,
    };

    std::map<string_view, KeyDef> key_table{
        {"auto", KeyDef::Auto},
        {"int", KeyDef::Int},
        {"float", KeyDef::Float},
        {"true", KeyDef::True},
        {"false", KeyDef::False},
        {"null", KeyDef::Null},
        {"if", KeyDef::If},
        {"else", KeyDef::Else},
        {"do", KeyDef::Do},
        {"while", KeyDef::While},
        {"for", KeyDef::For},
        {"return", KeyDef::Return},
    };
}