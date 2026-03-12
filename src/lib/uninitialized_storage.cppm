module;

#include "macros.h"
#include <cstring>
#include <memory> // addressof
#include <type_traits>
#include <utility>

export module uninitialized_storage;

import aliases;

template <typename T> constexpr void fillObjectWithDebugBytes(T *object)
{
    // #ifndef NDEBUG
    constexpr u8 debugByte = 0b01010101;
    if (not std::is_constant_evaluated()) {
        // memset is not constexpr, involves reinterpreting. allow calls to
        // fillObjectWithDebugBytes to happen in constexpr by doing this
        ::memset(object, debugByte, sizeof(T));
    }
    // #endif
}

struct Empty
{};

export template <typename T> union UninitializedStorage
{
  public:
    using type = T;

    Empty empty;
    T value;

    constexpr UninitializedStorage() NOEXCEPT : empty() {}

    template <typename... args_t>
        requires std::is_constructible_v<T, args_t...>
    constexpr UninitializedStorage(std::in_place_t, args_t &&...args) NOEXCEPT
        : value(std::forward<args_t>(args)...)
    {
    }

    constexpr void fillWithDebugBytes() NOEXCEPT
    {
        fillObjectWithDebugBytes(std::addressof(value));
    }

    constexpr ~UninitializedStorage()
        requires(!std::is_trivially_destructible_v<T>)
    {
        fillWithDebugBytes();
    }

    constexpr ~UninitializedStorage()
        requires(std::is_trivially_destructible_v<T>)
    = default;
};

static_assert(std::is_trivially_destructible_v<UninitializedStorage<int>>);
