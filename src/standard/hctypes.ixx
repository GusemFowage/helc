#include <utility>

export module hc.types;

import <list>;
import <memory>;
import <type_traits>;

import defint;

using namespace std;

export namespace hel {
    enum class TypeKind {
        BuildIn,
        Pointer,
        Function
    };
    template<TypeKind kind>
    struct TypeImpl;

    struct Type {
        TypeKind kind{};
        unsigned size{};
        unsigned align{};

        Type(TypeKind k, unsigned s, unsigned a)
            : kind(k), size(s), align(a){}
        virtual ~Type() = default;

        [[nodiscard]] bool IsInteger() const;
        [[nodiscard]] bool IsPointer() const;
        [[nodiscard]] bool IsFunction() const;
    };

    struct MutableValue {
        std::shared_ptr<Type> type;
        tag_t ident;
        unsigned offset;
        friend bool operator<(const MutableValue& a, const MutableValue& b) {
            return (a.ident < b.ident);
        }
    };

    template<>
    struct TypeImpl<TypeKind::BuildIn> : public Type{
        enum class Kind {
            Int,
        } clss;
        TypeImpl(Kind k, unsigned s, unsigned a)
            : Type(TypeKind::BuildIn, s, a), clss(k) {}
    };

    template<>
    struct TypeImpl<TypeKind::Pointer> : public Type{
    private:
        shared_ptr<Type> basTy;
    public:
        explicit TypeImpl(std::shared_ptr<Type> b)
            : Type(TypeKind::Pointer, 8, 8), basTy(std::move(b)) {}
    };

    struct FuncParam{
        std::shared_ptr<Type> type;
        string_view name;
    };

    template<>
    struct TypeImpl<TypeKind::Function> : public Type{
    private:
        shared_ptr<Type> retTy;
    public:
        list<shared_ptr<FuncParam>> args;
        explicit TypeImpl(std::shared_ptr<Type> r)
            : Type{TypeKind::Function, 8, 8}, retTy(std::move(r)) {}
    };

    namespace type_def {
//        std::shared_ptr<TypeImpl<TypeKind::BuildIn>>
//            IntTy(make_shared(TypeImpl<TypeKind::BuildIn>::Kind::Int, 8, 8);
        auto IntTy{make_shared<TypeImpl<TypeKind::BuildIn>>(
                TypeImpl<TypeKind::BuildIn>::Kind::Int,
                4, 4
        )};
    }
}

namespace hel {
    bool Type::IsInteger() const {
        if (kind == TypeKind::BuildIn) {
            auto t{dynamic_cast<const TypeImpl<TypeKind::BuildIn>*>(this)};
            return t->clss == TypeImpl<TypeKind::BuildIn>::Kind::Int;
        }
        return false;
    }

    bool Type::IsPointer() const {
        return kind == TypeKind::Pointer;
    }

    bool Type::IsFunction() const {
        return kind == TypeKind::Function;
    }
}