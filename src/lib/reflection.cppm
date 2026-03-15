module;

#include "macros.h"
#include <glaze/glaze.hpp>
#include <string_view>

export module reflection;

import aliases;

export enum class VisitorControlFlow {
    Continue,
    Break,
};

export template <typename T> constexpr std::string_view typeName() NOEXCEPT
{
    return glz::type_name<T>;
}

export template <typename T> constexpr u64 typeHash() NOEXCEPT
{
    return std::hash<std::string_view>{}(typeName<T>());
}

export template <size_t index, typename Struct>
constexpr decltype(auto) getMemberAtIndex(Struct &&value) NOEXCEPT
{
    if constexpr (glz::reflectable<Struct>) {
        return glz::get_member(
            std::forward<Struct>(value),
            glz::get<index>(glz::to_tie(std::forward<Struct>(value))));
    } else {
        return glz::get_member(std::forward<Struct>(value),
                               glz::get<index>(glz::reflect<Struct>::values));
    }
}

export template <typename Callable>
constexpr void forEachStructMember(auto &&value, Callable &&callable) NOEXCEPT
{
    using Struct = std::remove_cvref_t<decltype(value)>;
    constexpr auto N = glz::reflect<Struct>::size;
    if constexpr (N > 0) {

        using ReturnType = decltype(std::invoke(
            std::forward<Callable>(callable),
            getMemberAtIndex<0>(std::forward<Struct>(value)),
            glz::member_nameof<0, Struct>));

        static_assert(
            std::is_void_v<ReturnType> ||
                std::is_same_v<ReturnType, VisitorControlFlow>,
            "Lambda should return either void or VisitorControlFlow enum");

        if constexpr (std::is_void_v<ReturnType>) {
            [&]<size_t... I>(std::index_sequence<I...>) constexpr {
                (std::invoke(std::forward<Callable>(callable),
                             (getMemberAtIndex<I>(std::forward<Struct>(value))),
                             glz::member_nameof<I, Struct>),
                 ...);
            }(std::make_index_sequence<N>{});
        } else {
            [&]<size_t... I>(std::index_sequence<I...>) constexpr {
                const bool broke =
                    ((std::invoke(
                          std::forward<Callable>(callable),
                          (getMemberAtIndex<I>(std::forward<Struct>(value))),
                          glz::member_nameof<I, Struct>) ==
                      VisitorControlFlow::Break) ||
                     ...);
            }(std::make_index_sequence<N>{});
        }
    }
}

export template <typename Struct>
constexpr size_t structMemberCount = glz::reflect<Struct>::size;

namespace refl {
struct TestStruct
{
    i32 i = 0;
    f32 f = 0.0f;

    constexpr bool operator==(const TestStruct &other) const = default;
};

struct TestStructWithBool
{
    i32 i = 0;
    bool boolean = false;
    f32 f = 0.0f;

    constexpr bool operator==(const TestStructWithBool &other) const = default;
};

void testForEachStructMember()
{
    TestStruct test{1, 2.0f};

    bool success = true;
    const auto visitor = [&]<typename T>(T &member, std::string_view) {
        if constexpr (std::is_same_v<T, i32>) {
            member = 2;
        } else if constexpr (std::is_same_v<T, f32>) {
            member = 3.0f;
        } else {
            success = false;
            return VisitorControlFlow::Break;
        }
        return VisitorControlFlow::Continue;
    };
    forEachStructMember(test, visitor);

    w_assert(success, "");
    w_assert((test == TestStruct{2, 3.0f}), "");

    success = false;

    TestStructWithBool testWithBool{1, true, 2.0f};

    forEachStructMember(testWithBool, visitor);

    w_assert(not success, "");
    w_assert((testWithBool == TestStructWithBool{2, true, 2.0f}), "");

    size_t memberCount = 0;
    const auto voidVisitor = [&](auto &member, std::string_view) {
        member = {};
        ++memberCount;
    };
    forEachStructMember(testWithBool, voidVisitor);
    w_assert(memberCount == 3, "");
    w_assert(memberCount == structMemberCount<TestStructWithBool>, "");
    w_assert((testWithBool == TestStructWithBool{{}, {}, {}}), "");

    memberCount = 0;
    forEachStructMember(test, voidVisitor);
    w_assert(memberCount == structMemberCount<TestStruct>, "");
    w_assert((test == TestStruct{{}, {}}), "");
}
} // namespace refl

export namespace tests {
void reflection() { refl::testForEachStructMember(); }
} // namespace tests
