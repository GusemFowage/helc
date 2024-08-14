export module hc.types;

import <type_traits>;

import defint;

export namespace hel {
    struct MutableValue {
        tag_t ident;
        unsigned offset;
        friend bool operator<(const MutableValue& a, const MutableValue& b) {
            return (a.ident < b.ident);
        }
    };
}