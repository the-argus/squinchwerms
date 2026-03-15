module;

#include "macros.h"
#include <memory> // addressof
#include <type_traits>
#include <utility>

export module res;

import uninitialized_storage;
import is_instance;
import opt;
import aliases;

export template <typename T, typename E> class Res;

export template <typename T>
concept ErrorEnum = requires {
    requires std::is_enum_v<T>;
    requires(u64(T::Success) == 0);
};

export template <SimpleType T, ErrorEnum E> class Res<T, E>
{
  private:
    static_assert(!std::is_constructible_v<T, E>);

    UninitializedStorage<T> m_value;
    E m_error;

    template <typename U>
    inline static constexpr bool not_tag_or_error =
        (not std::is_same_v<in_place_t, std::remove_cvref_t<U>> and
         not std::is_same_v<E, std::remove_cvref_t<U>>);

  public:
    template <typename U, typename R> friend class Res;

    using value_type = T;

    constexpr Res() = delete;
    constexpr Res(Res &&) = default;
    constexpr Res &operator=(Res &&) = default;
    constexpr ~Res() = default;

    constexpr Res(E errorCode) : m_error(errorCode)
    {
        if (errorCode == E::Success) [[unlikely]] {
            w_abort("Attempt to construct res with success error code");
        }
    }

    // construct in_place
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit Res(in_place_t, Args &&...args) NOEXCEPT
        : m_error(E::Success),
          m_value(in_place, std::forward<Args>(args)...)
    {
    }

    // convert from a success value, if type is convertible to the success value
    template <typename Other = T>
    constexpr Res(Other &&other) NOEXCEPT
        requires(!IsInstance<Other, ::Res> and not_tag_or_error<Other> and
                 std::is_constructible_v<T, decltype(other)> and
                 std::is_convertible_v<decltype(other), T>)
        : m_error(E::Success), m_value(in_place, std::forward<Other>(other))
    {
    }

    // construct from a success value, if success value is constructible from
    // type
    template <typename OtherT>
    explicit constexpr Res(OtherT &&other) NOEXCEPT
        requires(!IsInstance<OtherT, ::Res> and not_tag_or_error<OtherT> and
                 std::is_constructible_v<T, decltype(other)> and
                 !std::is_convertible_v<decltype(other), T>)
        : m_error(E::Success), m_value(in_place, std::forward<OtherT>(other))
    {
    }

    // res is never copyable, unwrap before copying
    constexpr Res(const Res &other) NOEXCEPT = delete;
    constexpr Res &operator=(const Res &) NOEXCEPT = delete;

    constexpr bool isError() const NOEXCEPT { return m_error != E::Success; }
    constexpr bool isSuccess() const NOEXCEPT { return m_error == E::Success; }

    constexpr E error() const NOEXCEPT { return m_error; }

    constexpr T *operator->() & NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return std::addressof(this->m_value.value);
    }

    constexpr const T *operator->() const &NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return std::addressof(this->m_value.value);
    }

    constexpr const T *operator->() && = delete;

    constexpr T &unwrap() & NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return this->m_value.value;
    }

    constexpr const T &unwrap() const &NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return this->m_value.value;
    }

    constexpr T &&unwrap() && NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return std::move(this->m_value.value);
    }

    template <typename Callable>
        requires std::is_invocable_v<Callable, const T &>
    constexpr auto map(Callable &&callable) const &NOEXCEPT
    {
        using TransformedType = std::invoke_result_t<Callable, const T &>;
        using ReturnType = Res<TransformedType, E>;

        if (isError()) {
            return ReturnType(this->error());
        } else {
            return ReturnType(std::invoke(std::forward<Callable>(callable),
                                          this->m_value.value));
        }
    }

    template <typename Callable>
        requires std::is_invocable_v<Callable, T &>
    constexpr auto map(Callable &&callable) & NOEXCEPT
    {
        using TransformedType = std::invoke_result_t<Callable, T &>;
        using ReturnType = Res<TransformedType, E>;

        if (isError()) {
            return ReturnType(this->error());
        } else {
            return ReturnType(std::invoke(std::forward<Callable>(callable),
                                          this->m_value.value));
        }
    }

    template <typename Callable>
        requires std::is_invocable_v<Callable, T &>
    constexpr auto map(Callable &&callable) && NOEXCEPT
    {
        using TransformedType = std::invoke_result_t<Callable, T &&>;
        using ReturnType = Res<TransformedType, E>;

        if (isError()) {
            return ReturnType(this->error());
        } else {
            return ReturnType(std::invoke(std::forward<Callable>(callable),
                                          std::move(this->m_value.value)));
        }
    }
};

export template <Reference T, ErrorEnum E> class Res<T, E>
{
  private:
    // ambiguous types
    static_assert(!std::is_convertible_v<T, E &>);
    static_assert(!std::is_convertible_v<T, const E &>);
    static_assert(!std::is_convertible_v<T, E &&>);
    static_assert(!std::is_convertible_v<E &, T>);
    static_assert(!std::is_convertible_v<const E &, T>);
    static_assert(!std::is_convertible_v<E &&, T>);

    using Underlying = std::remove_reference_t<T>;
    using Pointer = std::add_pointer_t<Underlying>;

    Pointer m_value;
    E m_error;

  public:
    template <typename U, typename R> friend class Res;

    using value_type = T;

    constexpr Res(T reference) NOEXCEPT : m_error(E::Success),
                                          m_value(std::addressof(reference))
    {
    }

    constexpr Res(E errorCode) : m_error(errorCode)
    {
        if (errorCode == E::Success) [[unlikely]] {
            w_abort("Attempt to construct res with success error code");
        }
    }

    constexpr Res(const Res &other) NOEXCEPT = delete;
    constexpr Res &operator=(const Res &) NOEXCEPT = delete;
    constexpr Res(Res &&) = default;
    constexpr Res &operator=(Res &&) = default;
    constexpr ~Res() = default;

    constexpr bool isError() const NOEXCEPT { return m_error != E::Success; }
    constexpr bool isSuccess() const NOEXCEPT { return m_error == E::Success; }

    constexpr E error() const NOEXCEPT { return m_error; }

    constexpr Pointer operator->() const NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return m_value;
    }

    constexpr T unwrap() NOEXCEPT
    {
        w_assert(this->isSuccess(), "attempt to dereference error Res");
        return *m_value;
    }

    template <typename Callable>
        requires std::is_invocable_v<Callable, T>
    constexpr auto map(Callable &&callable) const NOEXCEPT
    {
        using TransformedType = std::invoke_result_t<Callable, T>;
        using ReturnType = Res<TransformedType, E>;

        if (isError()) {
            return ReturnType(this->error());
        } else {
            return ReturnType(std::invoke(std::forward<Callable>(callable),
                                          this->m_value.value));
        }
    }
};

export template <IsInstance<Res> T>
[[nodiscard]] constexpr bool isSuccess(const T &res)
{
    return res.isSuccess();
}

export template <ErrorEnum T>
[[nodiscard]] constexpr bool isSuccess(const T &error)
{
    return error == T::Success;
}

export template <IsInstance<Res> T>
[[nodiscard]] constexpr bool isError(const T &res)
{
    return res.isError();
}

export template <ErrorEnum T>
[[nodiscard]] constexpr bool isError(const T &error)
{
    return error != T::Success;
}
