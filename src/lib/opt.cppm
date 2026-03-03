module;

#include "macros.h"
#include <memory> // for std::construct_at
#include <type_traits>
#include <utility>

export module opt;

import uninitialized_storage;
import is_instance;
import aliases;

template <typename LHS, typename RHS>
concept is_comparable_c = requires(const std::remove_cvref_t<LHS> &lhs,
                                   const std::remove_cvref_t<RHS> &rhs) {
    { lhs == rhs } -> std::same_as<bool>;
};

export using in_place_t = std::in_place_t;
export constexpr auto in_place = std::in_place;

struct Null
{};
export constexpr Null null = {};

export template <typename T> class Opt;

export template <typename T>
concept Reference = std::is_lvalue_reference_v<T>;

export template <typename T>
concept SimpleType = requires {
    requires not std::is_reference_v<T>;
    requires not std::is_constructible_v<T, in_place_t>;
    requires not std::is_constructible_v<T, Null>;
    requires std::is_trivially_destructible_v<T>;
    requires std::is_trivially_move_constructible_v<T>;
    requires std::is_trivially_move_assignable_v<T>;
};

export template <SimpleType T> class Opt<T>
{
  private:
    UninitializedStorage<T> m_value;
    bool m_hasValue = false;

    template <typename OptT>
    constexpr void constructFromOtherOptional(OptT &&other)
    {
        static_assert(std::is_reference_v<decltype(other)>);
        static_assert(IsInstance<std::remove_cvref_t<decltype(other)>, ::Opt>);

        if (!other.hasValue())
            return;

        m_value.fillWithDebugBytes();
        if constexpr (std::is_rvalue_reference_v<decltype(other)>) {
            std::construct_at(std::addressof(m_value.value),
                              std::move(other.m_value.value));
        } else {
            std::construct_at(std::addressof(m_value.value),
                              other.m_value.value);
        }
    }

    template <typename OptT>
    constexpr void assignFromOtherOptional(OptT &&other)
    {
        static_assert(std::is_reference_v<decltype(other)>);
        static_assert(IsInstance<std::remove_cvref_t<decltype(other)>, ::Opt>);
        if (other.hasValue()) {
            if (this->hasValue()) {
                // both sides of assignment exist, just do assignment
                if constexpr (std::is_rvalue_reference_v<decltype(other)>) {
                    m_value.value = std::move(other.m_value.value);
                } else {
                    m_value.value = other.m_value.value;
                }
            } else {
                constructFromOtherOptional(std::forward<OptT>(other));
            }
        } else if (this->hasValue()) {
            // we have something and are getting assigned over with null
            // not calling a destructor, that needs to be okay
            static_assert(std::is_trivially_destructible_v<T>);
            m_value.fillWithDebugBytes();
        }
    }

    template <typename U>
    inline static constexpr bool not_tag =
        !std::is_same_v<in_place_t, std::remove_cvref_t<U>>;

  public:
    template <typename U> friend class Opt;

    using value_type = T;

    constexpr Opt() = default;
    constexpr Opt(Opt &&) = default;
    constexpr Opt &operator=(Opt &&) = default;
    constexpr ~Opt() = default;

    constexpr Opt(Null) NOEXCEPT {}
    constexpr Opt &operator=(Null) NOEXCEPT
    {
        this->reset();
        return *this;
    }

    // construct in_place
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit Opt(in_place_t, Args &&...args) NOEXCEPT
        : m_hasValue(true),
          m_value(in_place, std::forward<Args>(args)...)
    {
    }

    // convert different type of opt rvalue
    template <IsInstance<::Opt> OtherT>
    constexpr Opt(OtherT &&other)
        requires(!std::is_same_v<typename OtherT::value_type, T> and
                 std::is_convertible_v<decltype(*std::move(other)), T>)
        : m_hasValue(other.hasValue())
    {
        constructFromOtherOptional(std::move(other));
    }

    // convert different type of opt lvalue
    template <IsInstance<::Opt> OtherT>
    constexpr Opt(const OtherT &other)
        requires(!std::is_same_v<typename OtherT::value_type, T> and
                 std::is_convertible_v<decltype(*other), T>)
        : m_hasValue(other.hasValue())
    {
        constructFromOtherOptional(other);
    }

    // convert from something that is not an opt rvalue and lvalue
    template <typename Other = T>
    constexpr Opt(Other &&other) NOEXCEPT
        requires(!IsInstance<Other, ::Opt> and not_tag<Other> and
                 std::is_constructible_v<T, decltype(other)> and
                 std::is_convertible_v<decltype(other), T>)
        : m_hasValue(true), m_value(in_place, std::forward<Other>(other))
    {
    }

    // construct (explicit convert) from different types of Opt
    template <IsInstance<::Opt> OtherT>
    constexpr explicit Opt(OtherT &&other)
        requires(!std::is_same_v<typename OtherT::value_type, T> and
                 std::is_constructible_v<T, decltype(*std::move(other))> and
                 !std::is_convertible_v<decltype(*std::move(other)), T>)
        : m_hasValue(other.hasValue())
    {
        constructFromOtherOptional(std::move(other));
    }

    template <IsInstance<::Opt> OtherT>
    constexpr explicit Opt(const OtherT &other)
        requires(!std::is_same_v<typename OtherT::value_type, T> and
                 std::is_constructible_v<T, decltype(*other)> and
                 !std::is_convertible_v<decltype(*other), T>)
        : m_hasValue(other.hasValue())
    {
        constructFromOtherOptional(other);
    }

    // construct (explicit convert) from something that is not an Opt
    template <typename OtherT>
        requires(!IsInstance<OtherT, ::Opt>)
    constexpr Opt(OtherT &&other) NOEXCEPT
        requires(std::is_constructible_v<T, decltype(other)> and
                 !std::is_convertible_v<decltype(other), T>)
        : m_hasValue(true), m_value(in_place, std::forward<OtherT>(other))
    {
    }

    // trivially copy from Optional of the same type if T is trivially copyable
    constexpr Opt(const Opt &other) NOEXCEPT
        requires std::is_trivially_copy_constructible_v<T>
    = default;
    constexpr Opt &operator=(const Opt &) NOEXCEPT
        requires std::is_trivially_copy_assignable_v<T>
    = default;

    // copy from optional of the same type if T is not trivially copyable
    constexpr Opt(const Opt &other) NOEXCEPT
        requires(std::is_copy_constructible_v<T> and
                 !std::is_trivially_copy_constructible_v<T>)
        : m_hasValue(other.m_hasValue)
    {
        if (m_hasValue) {
            std::construct_at(std::addressof(m_value.value),
                              other.m_value.value);
        }
    }

    // assign from optional of the same type if T is not trivially copy
    // assignable
    constexpr Opt &operator=(const Opt &other) NOEXCEPT
        requires(std::is_copy_assignable_v<T> and
                 !std::is_trivially_copy_assignable_v<T>)
    {
        assignFromOtherOptional(other);
        m_hasValue = other.hasValue();
        return *this;
    }

    // assign from optionals of a different but convertible type
    template <typename OtherT>
        requires IsInstance<OtherT, ::Opt>
    constexpr Opt &operator=(OtherT &&other) NOEXCEPT
        requires std::is_convertible_v<decltype(*std::forward<OtherT>(other)),
                                       T>
    {
        assignFromOtherOptional(other);
        m_hasValue = other.hasValue();
        return *this;
    }

    // assign from convertible non-optional type
    template <typename OtherT>
        requires(!IsInstance<OtherT, ::Opt>)
    constexpr Opt &operator=(OtherT &&other) NOEXCEPT
        requires std::is_convertible_v<decltype(other), T>
    {
        if (not this->hasValue()) {
            std::construct_at(std::addressof(m_value.value),
                              std::forward<OtherT>(other));
            m_hasValue = true;
        } else {
            m_value.value = std::forward<OtherT>(other);
        }
        return *this;
    }

    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr void emplace(Args &&...args) NOEXCEPT
    {
        m_value.fillWithDebugBytes();
        static_assert(std::is_trivially_destructible_v<T>);
        std::construct_at(std::addressof(m_value.value),
                          std::forward<Args>(args)...);
        m_hasValue = true;
    }

    constexpr void reset() NOEXCEPT
    {
        m_hasValue = false;
        m_value.fillWithDebugBytes();
    }

    [[nodiscard]] constexpr explicit operator bool() const NOEXCEPT
    {
        return m_hasValue;
    }
    [[nodiscard]] constexpr bool hasValue() const NOEXCEPT
    {
        return m_hasValue;
    }

    [[nodiscard]] constexpr T *operator->() & NOEXCEPT
    {
        w_assert(this->m_hasValue, "attempt to dereference null opt");
        return std::addressof(this->m_value.value);
    }

    [[nodiscard]] constexpr const T *operator->() const &NOEXCEPT
    {
        w_assert(this->m_hasValue, "attempt to dereference null opt");
        return std::addressof(this->m_value.value);
    }

    [[nodiscard]] constexpr const T *operator->() && = delete;

    [[nodiscard]] constexpr T &unwrap() & NOEXCEPT
    {
        w_assert(this->m_hasValue, "attempt to unwrap null opt");
        return this->m_value.value;
    }

    [[nodiscard]] constexpr const T &unwrap() const &NOEXCEPT
    {
        w_assert(this->m_hasValue, "attempt to unwrap null opt");
        return this->m_value.value;
    }

    [[nodiscard]] constexpr T &&unwrap() && NOEXCEPT
    {
        w_assert(this->m_hasValue, "attempt to unwrap null opt");
        return std::move(this->m_value.value);
    }

    template <typename OtherT>
    [[nodiscard]] constexpr bool
    deepCompare(const Opt<OtherT> &other) const NOEXCEPT
        requires is_comparable_c<T, std::remove_reference_t<OtherT>>
    {
        if (other.hasValue() and this->hasValue()) {
            if constexpr (Reference<OtherT>) {
                return *other.m_value == this->m_value.value;
            } else {
                return other.m_value.value == this->m_value.value;
            }
        }
        return other.hasValue() == this->hasValue();
    }

    template <typename OtherT>
    [[nodiscard]] constexpr bool
    operator==(const Opt<OtherT> &other) const NOEXCEPT
        requires is_comparable_c<T, std::remove_reference_t<OtherT>>
    {
        return deepCompare(other);
    }

    template <typename OtherT>
        requires(not IsInstance<OtherT, Opt>)
    [[nodiscard]] constexpr bool deepCompare(const OtherT &other) const NOEXCEPT
        requires is_comparable_c<T, OtherT>
    {
        if (not this->hasValue())
            return false;
        return this->m_value.value == other;
    }

    template <typename OtherT>
        requires(not IsInstance<OtherT, Opt>)
    [[nodiscard]] constexpr bool operator==(const OtherT &other) const NOEXCEPT
        requires is_comparable_c<T, OtherT>
    {
        return deepCompare(other);
    }
};

export template <Reference T> class Opt<T>
{
  private:
    using Pointer = std::add_pointer_t<std::remove_reference_t<T>>;

    Pointer m_value = nullptr;

  public:
    using value_type = T;

    constexpr Opt() = default;
    constexpr Opt(Opt &&) = default;
    constexpr Opt &operator=(Opt &&) = default;
    constexpr Opt(const Opt &) = default;
    constexpr Opt &operator=(const Opt &) = default;

    constexpr Opt(Pointer pointer) NOEXCEPT : m_value(pointer) {}

    constexpr Opt(T reference) NOEXCEPT : m_value(std::addressof(reference)) {}

    template <Reference OtherRef>
        requires std::is_convertible_v<Pointer, typename Opt<OtherRef>::Pointer>
    constexpr operator Opt<OtherRef>() const NOEXCEPT
    {
        return m_value;
    }

    constexpr void emplace(T reference) NOEXCEPT
    {
        m_value = std::addressof(reference);
    }

    constexpr void reset() NOEXCEPT { m_value = nullptr; }

    constexpr Opt &operator=(std::nullptr_t) NOEXCEPT
    {
        reset();
        return *this;
    }

    constexpr explicit operator bool() const NOEXCEPT { return m_value; }

    constexpr bool hasValue() const NOEXCEPT { return m_value; }

    constexpr Pointer operator->() const NOEXCEPT
    {
        w_assert(this->hasValue(), "attempt to dereference null opt");
        return m_value;
    }

    constexpr T unwrap() const NOEXCEPT
    {
        w_assert(this->hasValue(), "attempt to dereference null opt");
        return *m_value;
    }

    constexpr T operator*() const NOEXCEPT { return unwrap(); }

    constexpr bool isAliasFor(const Opt &other) const NOEXCEPT
    {
        return m_value == other.m_value;
    }

    constexpr bool
    isAliasFor(const std::remove_cvref_t<T> &other) const NOEXCEPT
    {
        return m_value == std::addressof(other);
    }

    template <typename OtherT>
    [[nodiscard]] constexpr bool
    deepCompare(const Opt<OtherT> &other) const NOEXCEPT
        requires is_comparable_c<T, std::remove_reference_t<OtherT>>
    {
        if constexpr (std::is_same_v<T, OtherT>)
            if (isAliasFor(other))
                return true;
        if (not other.hasValue() and not this->hasValue())
            return true;
        if (other.hasValue() and this->hasValue()) {
            if constexpr (Reference<OtherT>) {
                return *m_value == *other.m_value;
            } else {
                return *m_value == other.unwrap();
            }
        }
        return false;
    }

    template <typename OtherT>
        requires(not IsInstance<OtherT, Opt>)
    constexpr bool deepCompare(const OtherT &other) const NOEXCEPT
        requires is_comparable_c<T, OtherT>
    {
        if (m_value == std::addressof(other))
            return true;
        if (this->hasValue())
            return *m_value == other;
        return false;
    }
};

constexpr bool testEqualityAndReset()
{
    Opt<i32> i;
    w_assert(not i.hasValue(), "");

    i = 12;
    w_assert(i.hasValue(), "");
    w_assert(i == 12, "");
    w_assert(i.unwrap() == 12, "");
    w_assert(i == Opt<i32>(12), "");
    w_assert(i == Opt<i64>(12), "");
    w_assert(i != 3, "");
    w_assert(i.unwrap() != 3, "");
    w_assert(i != Opt<i32>(3), "");
    w_assert(i != Opt<i64>(3), "");

    i = null;
    w_assert(not i.hasValue(), "");
    w_assert(i != 12, "");

    i = -12;
    w_assert(i.hasValue(), "");
    w_assert(i == -12, "");

    i.reset();
    w_assert(not i.hasValue(), "");
    w_assert(i != -12, "");

    return true;
}

constexpr bool testEmplace()
{
    struct Test
    {
        i32 i;
        i64 j;
        f32 f;

        constexpr bool operator==(const Test &) const = default;
    };

    Opt<Test> test;
    Test comparison{};
    w_assert(comparison != test, "");

    test.emplace(1, 2, 0.3f);
    comparison = {1, 2, 0.3f};
    w_assert(comparison == test, "");

    return true;
}

constexpr bool testDereference()
{
    struct Foo
    {
        i32 i;
        f32 f;
        constexpr bool operator==(const Foo &) const = default;
    };

    // rvalue dereference
    const bool same = Opt<Foo>(in_place, 1, 0.3f).unwrap() == Foo{1, 0.3f};
    w_assert(same, "");

    Opt<Foo> opt(in_place, 1, 0.3f);
    const auto comparison = Foo{1, 0.3f};
    Foo &f = opt.unwrap();
    f.f = 0.4f;
    f.i = 2;
    w_assert(opt->f == 0.4f, "");
    w_assert(opt.unwrap() != comparison, "");

    const Opt<Foo> optConst(in_place, 1, 0.3f);
    const Foo &f2 = optConst.unwrap();
    w_assert(optConst->f == 0.3f, "");
    w_assert(optConst == comparison, "");

    return true;
}

constexpr bool testConvertingConstructorsAndAssignment()
{
    Opt<i32> i = 3UL; // rvalue converting non-opt constructor
    // Opt<i64> j = i;   // lvalue converting opt constructor
    const i64 five = 5;
    Opt<u32> k = five; // lvalue converting non-opt constructor
    // Opt<u32> l = Opt<i8>(3); // rvalue converting opt constructor
    // Opt<u32> m = Opt<i8>();  // rvalue converting opt constructor
    // w_assert(not m.hasValue(), "");
    // w_assert(l == 3, "");
    w_assert(k == 5, "");
    // w_assert(j == 3, "");
    i = 4UL; // rvalue converting non-opt assignment
    w_assert(i == 4, "");
    const u64 four = 4UL;
    i = four; // lvalue converting non-opt assignment
    w_assert(i == 4, "");

    // j = i; // lvalue converting opt assignment
    // w_assert(j == 4, "");

    // j = Opt<u64>{5}; // rvalue converting opt assignment
    // w_assert(j == 5, "");

    return true;
}

constexpr bool testExplicitConstructors()
{
    struct Int
    {
        Int() = delete;
        constexpr explicit Int(u64 us) NOEXCEPT : i(us) {}
        i32 i = 0;
    };

    static_assert(not std::is_convertible_v<u64, Int>);

    // rvalue converting non-opt constructor
    Opt<Int> i = 32UL;
    w_assert(i->i == 32, "");

    // lvalue converting non-opt constructor
    const u64 thirty = 30;
    Opt<Int> j = thirty;
    w_assert(j->i == 30, "");

    // Opt<Int> k(Opt<u64>(3));
    Opt<Int> k(Opt<Int>(3));

    const auto three = Opt<u64>(3);
    // Opt<Int> l(three);
    Opt<Int> l(3);

    w_assert(l->i == 3, "");
    w_assert(k->i == 3, "");

    return true;
}

constexpr bool testCopying()
{
    struct Counters
    {
        u64 copyConstructs;
        u64 copyAssigns;
        u64 destructs;
    };

    Counters counters{};

    struct CounterType
    {
        int i = 0;
        Counters *counters;

        CounterType() = delete;
        constexpr CounterType(Counters &counters) : i(), counters(&counters) {}
        constexpr CounterType(Counters &counters, int value)
            : i(value), counters(&counters)
        {
        }

        constexpr CounterType(const CounterType &t)
            : i(t.i), counters(t.counters)
        {
            counters->copyConstructs++;
        }

        constexpr CounterType &operator=(const CounterType &t)
        {
            i = t.i;
            counters = t.counters;
            counters->copyAssigns++;
            return *this;
        }

        constexpr CounterType &operator=(CounterType &&) = default;
        constexpr CounterType(CounterType &&) = default;
    };

    Opt<CounterType> obj1(in_place, counters, 42);

    w_assert(obj1.hasValue(), "");
    w_assert(obj1->i == 42, "");
    w_assert(counters.copyConstructs == 0, "");
    w_assert(counters.copyAssigns == 0, "");

    Opt<CounterType> obj2 = obj1;

    w_assert(obj1.hasValue(), "");
    w_assert(obj2.hasValue(), "");
    w_assert(obj1->i == 42, "");
    w_assert(obj2->i == 42, "");
    w_assert(counters.copyConstructs == 1, "");
    w_assert(counters.copyAssigns == 0, "");

    Opt<CounterType> obj3;
    obj3 = obj1;

    w_assert(obj3.hasValue(), "");
    w_assert(obj1.hasValue(), "");
    w_assert(obj1->i == 42, "");
    w_assert(obj2->i == 42, "");
    w_assert(obj3->i == 42, "");
    // there was nothing inside obj3 so it is constructed, not assigned over
    w_assert(counters.copyConstructs == 2, "");
    w_assert(counters.copyAssigns == 0, "");

    obj3->i = 27;
    obj3 = obj1;
    w_assert(obj3->i == 42, "");

    w_assert(counters.copyConstructs == 2, "");
    w_assert(counters.copyAssigns == 1, "");

    Opt<CounterType> obj4(counters);
    w_assert(obj4->i == 0, "");
    obj4 = obj3;
    w_assert(obj4->i == 42, "");
    w_assert(obj3.hasValue(), "");
    w_assert(counters.copyConstructs == 2, "");
    w_assert(counters.copyAssigns == 2, "");

    return true;
}

constexpr bool testReference()
{
    int i = 1;
    Opt<int &> intRef = i;

    w_assert(intRef.isAliasFor(i), "");
    w_assert(not intRef.deepCompare(3), "");
    w_assert(intRef.deepCompare(1), "");
    w_assert(not intRef.deepCompare(Opt<int>(3)), "");
    w_assert(intRef.deepCompare(Opt<int>(1)), "");

    int j = 2;
    Opt<int &> nonconstRef = j;
    Opt<const int &> constRef = j;

    w_assert(constRef.deepCompare(2), "");
    *nonconstRef = 3;
    w_assert(not constRef.deepCompare(2), "");
    w_assert(constRef.deepCompare(3), "");

    w_assert(nonconstRef, "");
    nonconstRef = nullptr;
    w_assert(not nonconstRef, "");

    w_assert(intRef.isAliasFor(i), "");
    intRef = j;
    w_assert(not intRef.isAliasFor(i), "");
    w_assert(intRef.isAliasFor(j), "");

    intRef.emplace(i);
    w_assert(intRef.isAliasFor(i), "");
    w_assert(not intRef.isAliasFor(j), "");

    return true;
}

static_assert(testEqualityAndReset());
static_assert(testEmplace());
static_assert(testDereference());
static_assert(testConvertingConstructorsAndAssignment());
static_assert(testExplicitConstructors());
static_assert(testCopying());
static_assert(testReference());

static_assert(std::is_trivially_copy_constructible_v<::Opt<i32>>);
static_assert(std::is_trivially_copy_assignable_v<::Opt<i32>>);
static_assert(std::is_trivially_destructible_v<::Opt<i32>>);
