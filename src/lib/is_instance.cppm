module;

#include <type_traits>

export module is_instance;

template <class, template <typename...> typename>
struct IsInstanceMeta : public std::false_type
{};

template <class... Y, template <typename...> typename U>
struct IsInstanceMeta<U<Y...>, U> : public std::true_type
{};

export template <typename T, template <typename...> typename temp>
concept IsInstance = IsInstanceMeta<std::remove_cv_t<T>, temp>::value;
