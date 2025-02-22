#pragma once
#include <utility>

#include "natural_log/natural_log.h"
#include <okay/allocators/allocator.h>
#include <okay/macros/foreach.h>
#include <okay/opt.h>
#include <okay/short_arithmetic_types.h>

namespace werm {
template <typename T> struct LinkedPool
{
    // this pool expects to be wiped by an allocator clear and doesnt do
    // destruction
    static_assert(std::is_trivially_destructible_v<T>);

    struct Item
    {
        T self;
        bool free = false;
        friend struct LinkedPool;

      private:
        ok::opt_t<Item &> next;
    };

    struct FindResult
    {
        ok::opt_t<Item &> found;
        ok::opt_t<Item &> previous;
    };

    ok::opt_t<Item &> head;
    ok::allocator_t &backingAllocator;

    template <typename... Args>
    [[nodiscard]] constexpr ok::alloc::result_t<T &>
    make(Args &&...args) noexcept
    {
        static_assert(ok::is_std_constructible_v<T, Args...>);
        constexpr auto freePredicate = [](Item &i) { return i.free; };
        FindResult findResult = find(freePredicate);
        if (!findResult.found) {
            auto stat = allocMore();
            if (!stat.okay())
                return stat.err();
        }
        findResult = find(freePredicate);
        Item &free = findResult.found.value();
        free.next = ok::nullopt;
        free.free = false;
        new (&free.self) T(std::forward<Args>(args)...);
        return free.self;
    }

    constexpr void destroyAndReclaim(T &item) noexcept
    {
        FindResult matching =
            find([item](Item &i) { return &i.self == &item; });
        if (!matching.found) {
            LN_WARN("Attempt to destroy and reclaim item which could not be "
                    "found in LinkedPool");
            return;
        }
        Item &found = matching.found.value();
        found.free = true;
        // set to zeroes
        ok::memfill(ok::reinterpret_as_bytes(ok::slice_from_one(found.self)),
                    0);
    }

    // TODO: just make this a range i guess
    template <typename callable_t>
    constexpr void forEach(const callable_t &callable)
    {
        auto item = head;
        while (item) {
            Item &value = item.value();
            if (!value.free) {
                callable(value);
            }
            item = value;
        }
    }

  private:
    constexpr ok::status_t<ok::alloc::error> allocMore()
    {
        auto res = backingAllocator.allocate({
            .num_bytes = sizeof(Item) * 20,
            .alignment = alignof(Item),
        });
        if (!res.okay())
            return res.err();

        ok::bytes_t bytes = res.release().as_bytes();
        const auto num_items = bytes.size() / sizeof(Item);
        bytes = bytes.subslice({.start = 0, .length = num_items});
        auto new_items =
            ok::raw_slice(*reinterpret_cast<Item *>(bytes.data()), num_items);
        for (u64 i = 0; i < new_items.size(); ++i) {
            Item &item = new_items[i];
            item.free = true;
            if (i + 1 != new_items.size()) {
                item.next = new_items[i + 1];
            } else {
                item.next = ok::nullopt;
            }
        }

        return ok::alloc::error::okay;
    }

    template <typename callable_t>
    [[nodiscard]] constexpr FindResult
    find(const callable_t &predicate) noexcept
    {
        static_assert(
            ok::is_std_invocable_r_v<bool, const callable_t &, Item &>);
        FindResult out;
        auto item = head;
        while (item) {
            Item &value = item.value();
            if (predicate(value)) {
                out.found = item;
                return out;
            }
            out.previous = item;
            item = value.next;
        }
        out.previous = ok::nullopt;
        return out;
    }
};
} // namespace werm
