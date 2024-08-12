#ifndef HELLO_ATTRDEF_HXX
#define HELLO_ATTRDEF_HXX

#define NODISCARD [[nodiscard]]     // [[nodiscard]]
#define ALWAYS_INLINE inline        // [[always_inline]]

#define STD ::std::

using char_t = char;
template<class T> using pointer_t = T*;

#endif //HELLO_ATTRDEF_HXX
