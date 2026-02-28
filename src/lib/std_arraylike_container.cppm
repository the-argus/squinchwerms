module;

#include <concepts>
#include <type_traits>

export module arraylike_container;

import aliases;

template <typename T>
concept pointer = requires { requires std::is_pointer_v<T>; };
template <typename T>
concept const_pointer = requires {
    requires pointer<T>;
    requires std::is_const_v<std::remove_pointer_t<T>>;
};
template <typename T>
concept nonconst_pointer = requires {
    requires pointer<T>;
    requires !std::is_const_v<std::remove_pointer_t<T>>;
};

template <typename T, typename target_t>
concept pointer_to = requires {
    requires pointer<T>;
    requires std::is_same_v<std::remove_pointer_t<T>, target_t>;
};

template <typename T, typename target_t>
concept nonconst_or_const_pointer_to = requires {
    requires pointer<T>;
    requires std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>,
                            std::remove_cv_t<target_t>>;
};

export template <typename T>
concept StdArraylikeContainer = requires(const T &c, T &nc) {
    {
        c.data()
    } -> const_pointer; // owning container: const should always mean
                        // const interior
    {
        nc.data()
    } -> pointer; // allowed to be const, too (std::array<const int>)
    { c.size() } -> std::same_as<u64>;
    { nc.size() } -> std::same_as<u64>;
    { std::declval<T &&>().size() } -> std::same_as<u64>;
};

export template <typename T, typename Contents>
concept StdArraylikeContainerOf =
    requires(const T &c, std::remove_const_t<T> &nc) {
        requires StdArraylikeContainer<T>;
        { c.data() } -> std::same_as<const Contents *>;
        { nc.data() } -> std::same_as<Contents *>;
    };

export template <typename T, typename Contents>
concept StdArraylikeContainerOfConstOrNonconst =
    requires(const T &c, std::remove_const_t<T> &nc) {
        requires StdArraylikeContainer<T>;
        requires std::is_same_v<
            std::remove_const_t<std::remove_pointer_t<decltype(c.data())>>,
            Contents>;
        requires std::is_same_v<
            std::remove_const_t<std::remove_pointer_t<decltype(nc.data())>>,
            Contents>;
    };
