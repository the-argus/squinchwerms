module;

#include <type_traits>

#include <fmt/format.h>

#include "macros.h"

export module slice;

import aliases;
import uninitialized_storage;
import arraylike_container;
import is_instance;

export struct SubSliceOptions
{
    u64 start = 0;
    u64 length;
};

// forward decls
export template <typename Viewed> class Slice;

// library implementation wants to be able to construct Slices from data + size,
// particularly in stdmem and allocators
export namespace unsafe {
template <typename Viewed>
Slice<Viewed> rawSlice(Viewed &data, u64 size) NOEXCEPT;
}

/// Create a Slice with no elements and no data
export template <typename Viewed>
constexpr Slice<Viewed> makeNullSlice() NOEXCEPT;

/// A non-owning reference to a section of a contiguously allocated array of
/// type T. Intended to be passed around like a pointer.
/// The data pointed at by a Slice can be expected to be "initialized"-
/// unless the viewed type is trivially constructible, in which case the type
/// provides no guarantees and that is also true of a Slice of the type.
export template <typename Viewed> class Slice
{
    static_assert(!std::is_reference_v<Viewed>,
                  "Cannot create a Slice of references.");

  private:
    u64 m_elements;
    Viewed *m_data;

    constexpr Slice(Viewed &data, u64 size) NOEXCEPT : m_data(&data),
                                                       m_elements(size)
    {
    }

    inline static UninitializedStorage<Viewed> nullSliceMemory;
    inline static auto *nullSliceMemoryStart = &nullSliceMemory.value;

    struct NullSliceTag
    {};

    constexpr Slice(NullSliceTag) NOEXCEPT : m_data(nullSliceMemoryStart),
                                             m_elements(0)
    {
    }

  public:
    Slice(const Slice &) = default;
    Slice &operator=(const Slice &) = default;
    Slice(Slice &&) = default;
    Slice &operator=(Slice &&) = default;
    ~Slice() = default;

    // friends with all other types of Slices
    template <typename T> friend class Slice;

    using value_type = Viewed;

    [[nodiscard]] constexpr value_type *addressOfFirstItem() const NOEXCEPT
    {
        return &first();
    }

    /// Guaranteed to never return nullptr, it is defined behavior to
    /// dereference this pointer but it is only defined behavior to write to it
    /// if the slice is not empty.
    [[nodiscard]] constexpr value_type *
    uncheckedAddressOfFirstItem() const NOEXCEPT
    {
        w_assert(m_data,
                 "Attempted to call unchecked_address_of_first_item() but the "
                 "Slice points to no valid data.");
        return m_data;
    }

    [[nodiscard]] constexpr u64 size() const NOEXCEPT { return m_elements; }

    [[nodiscard]] constexpr u64 sizeInBytes() const NOEXCEPT
    {
        return m_elements * sizeof(Viewed);
    }

    [[nodiscard]] constexpr u64 sizeInBits() const NOEXCEPT
    {
        return m_elements * sizeof(Viewed) * 8;
    }

    [[nodiscard]] constexpr bool isEmpty() const NOEXCEPT
    {
        return m_elements == 0;
    }

    [[nodiscard]] constexpr value_type &first() const NOEXCEPT
    {
        if (this->isEmpty()) [[unlikely]] {
            w_abort("Attempt to get first() item from empty Slice.");
        }
        return m_data[0];
    }

    [[nodiscard]] constexpr value_type &last() NOEXCEPT
    {
        if (this->isEmpty()) [[unlikely]] {
            w_abort("Attempt to get last() item from empty Slice.");
        }
        return m_data[m_elements - 1];
    }

    template <StdArraylikeContainerOf<Viewed> U>
    constexpr Slice(U &other) NOEXCEPT
        requires(!std::is_const_v<Viewed> && !IsInstance<U, Slice> &&
                 !std::is_const_v<U>)
        : m_elements(other.size()), m_data(other.data())
    {
    }

    // if we are const, then a const or nonconst reference to a container of
    // const or nonconst items is always valid.
    // NOTE: although this seems to take a nonconst reference, because it is
    // templated it can also take nonconst references.
    template <
        StdArraylikeContainerOfConstOrNonconst<std::remove_const_t<Viewed>> U>
    constexpr Slice(U &other) NOEXCEPT
        requires(std::is_const_v<Viewed> && !IsInstance<U, Slice>)
        : m_elements(other.size()), m_data(other.data())
    {
    }

    // mark rvalue reference overloads for array-to-Slice constructors as
    // deleted, so we dont create a Slice of something that's about to be
    // destroyed
    template <
        StdArraylikeContainerOfConstOrNonconst<std::remove_const_t<Viewed>> U>
    constexpr Slice(const U &&other)
        requires(!IsInstance<U, ::Slice>)
    = delete;

    template <
        StdArraylikeContainerOfConstOrNonconst<std::remove_const_t<Viewed>> U>
    constexpr Slice(U &&other)
        requires(!IsInstance<U, Slice>)
    = delete;

    // c array converting constructors
    template <u64 size>
    constexpr Slice(Viewed (&array)[size]) NOEXCEPT : m_data(array),
                                                      m_elements(size)
    {
    }

    // NOTE: no need for type deduction to work here as the above c array
    // converting constructor is what should provide deduction. you only
    // coerce to const if you explicitly provide type info like
    // int i[] = {1, 2, 3 4};
    // Slice<const int> s(i);
    template <typename nonconst_value_type, u64 size>
        requires(std::is_const_v<value_type> &&
                 std::is_same_v<const nonconst_value_type, value_type>)
    constexpr Slice(nonconst_value_type (&array)[size]) NOEXCEPT
        : m_data(array),
          m_elements(size)
    {
    }

    // can convert to const version of self
    constexpr operator Slice<const value_type>() const NOEXCEPT
    {
        return Slice<const value_type>(*m_data, m_elements);
    }

    [[nodiscard]] constexpr bool
    isAliasFor(const Slice<const value_type> &other) NOEXCEPT
        requires(!std::is_const_v<value_type>)
    {
        return m_elements == other.m_elements && m_data == other.m_data;
    };

    [[nodiscard]] constexpr bool isAliasFor(const Slice &other) NOEXCEPT
    {
        return m_elements == other.m_elements && m_data == other.m_data;
    };

    [[nodiscard]] constexpr value_type &operator[](u64 idx) const NOEXCEPT
    {
        if (idx >= m_elements) [[unlikely]] {
            w_abort("Out of bounds access into Slice.");
        }
        return m_data[idx];
    }

    [[nodiscard]] constexpr value_type &uncheckedAccess(u64 idx) const NOEXCEPT
    {
        w_assert(idx < m_elements, "Out of bounds access into Slice.");
        return m_data[idx];
    }

    [[nodiscard]] constexpr Slice
    subslice(const SubSliceOptions &options) const NOEXCEPT
    {
        if (options.start >= m_elements) [[unlikely]] {
            w_abort("Attempt to create subslice but the starting value is "
                    "out of bounds.");
        }
        if (options.start + options.length > m_elements) [[unlikely]] {
            w_abort("Attempt to create subslice but the ending value is out "
                    "of bounds.");
        }

        return Slice(*(m_data + options.start), options.length);
    }

    /// Creates a subslice which does not contain the first `num_to_drop` items
    /// from this slice.
    [[nodiscard]] constexpr Slice drop(const u64 num_to_drop) const NOEXCEPT
    {
        if (num_to_drop > this->size()) [[unlikely]] {
            w_abort("Attempt to drop more items from a slice than it holds");
        }

        return Slice(*(m_data + num_to_drop), this->size() - num_to_drop);
    }

    Slice() = delete;

    constexpr operator bool() { return not isEmpty(); }

    template <typename T>
    friend Slice<T> unsafe::rawSlice(T &data, u64 size) NOEXCEPT;
    template <typename T> friend constexpr Slice<T> makeNullSlice() NOEXCEPT;

    friend struct fmt::formatter<Slice>;
};

template <typename Viewed, u64 size> Slice(Viewed (&)[size]) -> Slice<Viewed>;

template <typename T, bool is_const>
using conditionally_const_t = std::conditional_t<is_const, const T, T>;

template <typename T>
    requires StdArraylikeContainer<std::remove_cvref_t<T>>
Slice(T) -> Slice<conditionally_const_t<
    std::remove_cvref_t<decltype(*std::declval<T>().data())>,
    std::is_const_v<typename std::remove_cvref_t<T>::value_type>>>;

export using Bytes = Slice<u8>;

/// Make a slice of only a part of a contiguous stdlib container
/// from: index to start subslice from
/// to: index to end subslice at (must be greater than `from` and less than
/// container.size())
export template <StdArraylikeContainer Container>
[[nodiscard]] constexpr auto subslice(Container &container,
                                      const SubSliceOptions &options) NOEXCEPT
    -> Slice<std::remove_reference_t<decltype(*container.data())>>
{
    if (options.start >= container.size()) [[unlikely]] {
        w_abort("Attempt to get a subslice of a container but the starting "
                "value is out of range.");
    }
    if (options.start + options.length > container.size()) [[unlikely]] {
        w_abort("Attempt to get a subslice of a container but the ending "
                "value is out of range.");
    }

    return unsafe::rawSlice(*(container.data() + options.start),
                            options.length);
}

export template <typename Viewed>
[[nodiscard]] constexpr auto subslice(const Slice<Viewed> &slice,
                                      const SubSliceOptions &options) NOEXCEPT
{
    return slice.subslice(options);
}

export namespace unsafe {
/// Construct a Slice from a starting item and a number of items. Generally a
/// bad idea, but useful when interfacing with things like c-style strings.
template <typename Viewed>
[[nodiscard]] Slice<Viewed> rawSlice(Viewed &data, u64 size) NOEXCEPT
{
    return Slice<Viewed>(data, size);
}
} // namespace unsafe

export template <typename Viewed>
[[nodiscard]] constexpr Slice<Viewed> makeNullSlice() NOEXCEPT
{
    return Slice<Viewed>(typename Slice<Viewed>::NullSliceTag{});
}

export template <typename T>
[[nodiscard]] constexpr Slice<T> sliceFromOne(T &item) NOEXCEPT
{
    return unsafe::rawSlice(item, 1);
}

export template <typename T>
[[nodiscard]] constexpr bool memoverlaps(Slice<T> a, Slice<T> b) NOEXCEPT
{
    return a.uncheckedAddressOfFirstItem() <
               (b.uncheckedAddressOfFirstItem() + b.size()) &&
           b.uncheckedAddressOfFirstItem() <
               (a.uncheckedAddressOfFirstItem() + a.size());
}

export template <typename T> struct MemcopyOptions
{
    Slice<T> to;
    Slice<T> from;
};

export template <typename T> struct MemcontainsOptions
{
    Slice<T> outer;
    Slice<T> inner;
};

export template <typename T>
[[nodiscard]] constexpr bool
memcontains(const MemcontainsOptions<T> &options) NOEXCEPT
{
    if (options.outer.isEmpty()) [[unlikely]] {
        return false;
    }
    return options.outer.uncheckedAddressOfFirstItem() <=
               options.inner.uncheckedAddressOfFirstItem() &&
           options.outer.uncheckedAddressOfFirstItem() + options.outer.size() >=
               options.inner.uncheckedAddressOfFirstItem() +
                   options.inner.size();
}

export template <typename T>
constexpr Slice<T> memcopy(const MemcopyOptions<T> &options) NOEXCEPT
{
    static_assert(std::is_trivially_copyable_v<T>,
                  "Cannot memcopy non-trivially copyable type.");

    if (options.from.isEmpty()) {
        return options.to.subslice({.length = 0});
    }

    if (options.to.size() < options.from.size() ||
        memoverlaps(options.to, options.from)) [[unlikely]] {
        w_abort("Attempt to memcopy but the memory given either overlaps or "
                "has a smaller destination than source.");
    }

    ::memcpy(options.to.uncheckedAddressOfFirstItem(),
             options.from.uncheckedAddressOfFirstItem(),
             options.from.size() * sizeof(T));

    return unsafe::rawSlice(*options.to.uncheckedAddressOfFirstItem(),
                            options.from.size());
}

export [[nodiscard]] constexpr bool memcompare(Bytes lhs, Bytes rhs) NOEXCEPT
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    if (lhs.uncheckedAddressOfFirstItem() ==
        rhs.uncheckedAddressOfFirstItem()) {
        return true;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs.uncheckedAddressOfFirstItem()[i] !=
            rhs.uncheckedAddressOfFirstItem()[i]) {
            return false;
        }
    }
    return true;
}

export template <typename T, typename... Args>
    requires(std::is_constructible_v<T, Args...> and not std::is_const_v<T> and
             std::is_trivially_destructible_v<T>)
constexpr void memfill(Slice<T> slice, Args &&...args) NOEXCEPT
{
    if constexpr (std::is_same_v<T, u8>) {
        ::memset(slice.uncheckedAddressOfFirstItem(),
                 u8(std::forward<Args>(args)...), slice.size());
    } else {
        for (size_t i = 0; i < slice.size(); ++i) {
            auto &item = slice.uncheckedAccess(i);
            std::construct_at(item, std::forward<Args>(args)...);
        }
    }
}

export template <typename T>
    requires(not std::is_const_v<T>)
[[nodiscard]] constexpr Bytes reinterpretAsBytes(Slice<T> slice) NOEXCEPT
{
    return unsafe::rawSlice(
        *reinterpret_cast<u8 *>(slice.uncheckedAddressOfFirstItem()),
        slice.sizeInBytes());
}

export template <typename T>
    requires(std::is_const_v<T>)
[[nodiscard]] constexpr Slice<const u8>
reinterpretAsBytes(Slice<T> slice) NOEXCEPT
{
    return unsafe::rawSlice(
        *reinterpret_cast<const u8 *>(slice.uncheckedAddressOfFirstItem()),
        slice.sizeInBytes());
}

export template <typename T>
[[nodiscard]] constexpr Slice<T> reinterpretBytesAs(Bytes bytes) NOEXCEPT
{
    return unsafe::rawSlice(
        *reinterpret_cast<T *>(bytes.uncheckedAddressOfFirstItem()),
        bytes.size() / sizeof(T));
}

export template <typename Viewed> struct fmt::formatter<Slice<Viewed>>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            FMT_THROW("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    auto format(const Slice<Viewed> &Slice, format_context &ctx) const
        -> format_context::iterator
    {
        // TODO: use CTTI here to get nice typename before pointer
        return fmt::format_to(
            ctx.out(), "Slice<{:p} -> {}>",
            const_cast<void *>(static_cast<const void *>(Slice.m_data)),
            Slice.m_elements);
    }
};

export template <> struct fmt::formatter<Slice<const char>>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        // uber procedural parsing algorithm to check for either a {} format
        // specifier or a {s} format specifier.
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 's') {
            ++it;
            m_is_string = true;
        }
        return it;
    }

    auto format(const Slice<const char> &slice, format_context &ctx) const
        -> format_context::iterator
    {
        if (!m_is_string) {
            return fmt::format_to(
                ctx.out(), "Slice<{:p} -> {}>",
                const_cast<void *>(static_cast<const void *>(slice.m_data)),
                slice.m_elements);
        } else {
            // TODO: there must be a better (memcpy) way to print a string here,
            // maybe inherit from string view, see
            // https://fmt.dev/latest/api/#formatting-user-defined-types
            auto iter = ctx.out();
            for (u64 i = 0; i < slice.size(); ++i) {
                iter = fmt::format_to(iter, "{}", slice.uncheckedAccess(i));
            }
            return iter;
        }
    }

    bool m_is_string = false;
};
