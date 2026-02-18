module;

#include "macros.h"
#include <type_traits>

export module res;

import uninitialized_storage;
import opt;
import aliases;

export namespace lib {

	template <typename T>
		concept ErrorEnum = requires {
			requires std::is_enum_v<T>;
			requires T::Success == 0;
		};

template <SimpleType T, ErrorEnum E>
class Res
{
  private:

    UninitializedStorage<T> m_value;
    E m_error;

    template <typename U>
    inline static constexpr bool not_tag_or_error =
        (!std::is_same_v<lib::in_place_t, std::remove_cvref_t<U>> and (not std::is_same_v<E, std::remove_cvref_t<U>>);
    

  public:
    template <typename U, typename R> friend class Res;

    using value_type = T;

    constexpr Res() = default;
    constexpr Res(Res &&) = default;
    constexpr Res &operator=(Res &&) = default;
    constexpr ~Res() = default;

    // construct in_place
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit Res(in_place_t, Args &&...args) NOEXCEPT
        : m_hasValue(true),
          m_value(in_place, std::forward<Args>(args)...)
    {
    }

    // convert from a success value, if type is convertible to the success value
    template <typename Other = T>
    constexpr Res(Other &&other) NOEXCEPT
        requires(!is_instance_c<Other, lib::Res> and not_tag_or_error<Other> and
                 std::is_constructible_v<T, decltype(other)> and
                 std::is_convertible_v<decltype(other), T>)
        : m_hasValue(E::Success), m_value(in_place, std::forward<Other>(other))
    {
    }

    // construct from a success value, if success value is constructible from type
    template <typename OtherT>
    constexpr Res(OtherT &&other) NOEXCEPT
        requires(!is_instance_c<OtherT, lib::Res> and not_tag_or_error<OtherT> and std::is_constructible_v<T, decltype(other)> and
                 !std::is_convertible_v<decltype(other), T>)
        : m_hasValue(true), m_value(in_place, std::forward<OtherT>(other))
    {
    }

	// res is never copyable, unwrap before copying
    constexpr Res(const Res &other) NOEXCEPT = delete;
    constexpr Res &operator=(const Res &) NOEXCEPT = delete;

    constexpr bool isError() const NOEXCEPT { return m_error != E::Success; }
    constexpr bool isSuccess() const NOEXCEPT { return m_error == E::Success; }

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
};
}
