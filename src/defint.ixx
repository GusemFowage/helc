export module defint;

import <string>;
import <string_view>;

import <tuple>;
import <list>;

#include "attrdef.hxx"

export namespace hel {
    using string = STD basic_string<char_t>;
    using string_view = STD basic_string_view<char_t>;
    using pos_t = ptrdiff_t;
    using upos_t = STD make_unsigned_t<pos_t>;

    struct char_info : STD char_traits<char_t> {
        using traits = STD char_traits<char_t>;
        using int_type = traits::int_type;
        static constexpr char_t flr() {
            return STD numeric_limits<char_t>::min();
        }
        static constexpr char_t rng() {
            return STD numeric_limits<char_t>::max();
        }
        ALWAYS_INLINE static constexpr bool is_eof(char_t c) noexcept {
            return (c == traits::eof());
        }
        ALWAYS_INLINE static constexpr bool is_digit(char_t c) noexcept {
            return (c >= '0' && c <= '9');
        }
        ALWAYS_INLINE static constexpr int_type to_digit(char_t c) noexcept {
            return (is_digit(c) ? (c - '0') : c);
        }
        ALWAYS_INLINE static constexpr bool is_lower(char_t c) noexcept {
            return (c >= 'a' && c <= 'z');
        }
        ALWAYS_INLINE static constexpr bool is_upper(char_t c) noexcept {
            return (c >= 'A' && c <= 'Z');
        }
        ALWAYS_INLINE static constexpr bool is_alpha(char_t c) noexcept {
            return (is_lower(c) || is_upper(c));
        }
        ALWAYS_INLINE static constexpr bool is_space(char_t c) noexcept {
            return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        }
        ALWAYS_INLINE static constexpr bool is_sign(char_t c) noexcept {
            constexpr string_view signs{R"(`#$.~^&|!?%,*-+=()<>{}[]\'")"};
            return (signs.find(c) >= signs.size());
        }
        ALWAYS_INLINE static constexpr string to_seen_char(char_t c) noexcept {
            if (c == '\0') return "\\0";
            if (c == '\n') return "\\n";
            if (c == '\r') return "\\r";
            return string()+c;
        }
    };

    template<class T, class ... Ts>
    struct is_in {};
    template<class T1, class T2, class ... Ts>
    struct is_in<T1, T2, Ts...>
        : std::conditional_t<std::is_same_v<T1, T2>, std::true_type, typename is_in<T1, Ts...>::bool_type> {
        using bool_type = std::conditional_t<std::is_same_v<T1, T2>, std::true_type, typename is_in<T1, Ts...>::bool_type>;
    };
    template<class T> struct is_in<T> : std::false_type {
        using bool_type = std::false_type;
    };
    template<class ... Ts> struct typelist {};
    template<class Tlist, class...Ts>
    struct unique_typelist {};
    template<class...Tv, class T, class ... Ts>
    struct unique_typelist<typelist<Tv...>, T, Ts...> {
        using type = std::conditional_t<
            is_in<T, Ts...>::value,
            typename unique_typelist<typelist<Tv...>, Ts...>::type,
            typename unique_typelist<typelist<Tv..., T>, Ts...>::type
        >;
    };
    template<class...Tv>
    struct unique_typelist<typelist<Tv...>> {
        using type = typelist<Tv...>;
    };
    template<class ... Ts>
    using make_unique_typelist = unique_typelist<typelist<>, Ts...>::type;
}

export {
    using chr_t = char_t;
    using num_t = hel::string_view;
    using str_t = hel::string_view;
    using tag_t = hel::string_view;
    using doc_t = std::list<str_t>;
}
//using namespace hel;
//template<class... Ts>
//void func(typelist<Ts...>) {
//    std::tuple<Ts...> a{};
//}
//func(make_unique_typelist<int, double, int, short, char>{});