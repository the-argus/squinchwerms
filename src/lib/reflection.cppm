module;

#include "macros.h"
#include <glaze/glaze.hpp>

export module reflection;

import aliases;

export enum class VisitorControlFlow {
    Continue,
    Break,
};

export template <typename Struct, typename Callable>
constexpr void forEachStructMember(Struct &&value, Callable &&callable) NOEXCEPT
{
    constexpr auto N = glz::reflect<Struct>::size;
    if constexpr (N > 0) {

        using ReturnType = decltype(std::invoke(
            std::forward<Callable>(callable),
            glz::get_member(
                std::forward<Struct>(value),
                glz::get<0>(glz::to_tie(std::forward<Struct>(value))))));

        static_assert(
            std::is_void_v<ReturnType> ||
                std::is_same_v<ReturnType, VisitorControlFlow>,
            "Lambda should return either void or VisitorControlFlow enum");

        if constexpr (std::is_void_v<ReturnType>) {
            [&]<size_t... I>(std::index_sequence<I...>) constexpr {
                (std::invoke(
                     std::forward<Callable>(callable),
                     (glz::get_member(std::forward<Struct>(value),
                                      glz::get<I>(glz::to_tie(
                                          std::forward<Struct>(value)))))),
                 ...);
            }(std::make_index_sequence<N>{});
        } else {
            [&]<size_t... I>(std::index_sequence<I...>) constexpr {
                const bool broke =
                    ((std::invoke(std::forward<Callable>(callable),
                                  (glz::get_member(
                                      std::forward<Struct>(value),
                                      glz::get<I>(glz::to_tie(
                                          std::forward<Struct>(value)))))) ==
                      VisitorControlFlow::Break) ||
                     ...);
            }(std::make_index_sequence<N>{});
        }
    }
}

export template <typename Struct>
constexpr size_t structMemberCount = glz::reflect<Struct>::size;

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
    const auto visitor = [&]<typename T>(T &member) {
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
    const auto voidVisitor = [&](auto &member) {
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

export namespace tests {
void reflection() { testForEachStructMember(); }
} // namespace tests
