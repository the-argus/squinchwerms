module;

#include "macros.h"

#include <concepts>
#include <memory> // for construct_at
#include <type_traits>
#include <utility>

export module allocator;

import aliases;
import slice;
import opt;
import res;

export class Allocator;

export namespace alloc {
enum class Error : u8
{
    Success = 0,
    OOM,
    Snsupported,
    Usage,
    CouldntExpandInPlace,
    PlatformFailure,
};

inline constexpr u64 default_align = alignof(u64) * 2;

struct Request
{
    u64 numBytes;
    u64 alignment = alloc::default_align;
};

struct ReallocateRequest
{
    Bytes memory;
    // minimum size of the memory after reallocating. arraylist may set this to
    // current size + sizeof(T) when appending. Although it is not the optimal
    // size increase, it is the minimum needed to continue without an error.
    u64 newSizeBytes;
    // the optimal new size after reallocation. for an arraylist this would be
    // current size * growth_factor. ignored if shrinking or if zero
    u64 preferredSizeBytes = 0;
    u64 alignment = alloc::default_align;
    bool inPlaceOrElseFail = false;

    [[nodiscard]] constexpr bool is_valid() const NOEXCEPT
    {
        return !memory.isEmpty() &&
               // no attempt to... free the memory?
               (newSizeBytes != 0) &&
               // preferred should be zero OR ( (we're growing OR staying the
               // same size) + preferred is greater than required )
               (preferredSizeBytes == 0 || (newSizeBytes >= memory.size() &&
                                            preferredSizeBytes > newSizeBytes));
    }

    [[nodiscard]] constexpr size_t calculatePreferredSize() const noexcept
    {
        return preferredSizeBytes == 0 ? newSizeBytes : preferredSizeBytes;
    }
};
} // namespace alloc

export class Allocator
{
  public:
    [[nodiscard]] constexpr Res<Bytes, alloc::Error>
    allocate(const alloc::Request &request) NOEXCEPT
    {
        // one way for request to be invalid
        if (request.numBytes == 0) [[unlikely]] {
            w_assert(false, "Attempt to allocate 0 bytes from allocator.");
            return makeNullSlice<u8>(); // alloc::error::unsupported;
        }
        return impl_allocate(request);
    }

    constexpr void deallocate(void *memory, size_t size_hint = 0) NOEXCEPT
    {
        if (memory) [[likely]]
            impl_deallocate(memory, size_hint);
    }

    [[nodiscard]] constexpr Res<Bytes, alloc::Error>
    reallocate(const alloc::ReallocateRequest &options) NOEXCEPT
    {
        if (!options.is_valid()) [[unlikely]] {
            w_assert(false, "invalid ReallocateRequest");
            return makeNullSlice<u8>();
        }
        return impl_reallocate(options);
    }

    struct RestorePoint
    {
        friend class Allocator;

        RestorePoint(const RestorePoint &) = delete;
        RestorePoint &operator=(const RestorePoint &) = delete;
        RestorePoint(RestorePoint &&) = delete;
        RestorePoint &operator==(RestorePoint &&) = delete;

        constexpr ~RestorePoint()
        {
            m_allocator.impl_arenaRestoreScope(m_handle);
        }

        RestorePoint() = delete;

      private:
        constexpr RestorePoint(Allocator &allocator)
            : m_allocator(allocator), m_handle(allocator.impl_arenaNewScope())
        {
        }

        void *m_handle;
        Allocator &m_allocator;
    };

    template <typename T>
        requires std::is_invocable_r_v<void, T> and
                 std::is_move_constructible_v<T> and
                 std::is_trivially_destructible_v<T>
    constexpr void pushDestructor(T &&destructorCallableObject) NOEXCEPT
    {
        struct Destructor : public DestructorBase
        {
            T callable;

            Destructor(T &&callableParam) : callable(std::move(callableParam))
            {
                this->destructorFunction = [](DestructorBase &self) {
                    static_cast<Destructor *>(&self)->callable();
                };
            }
        };

        Res maybeDestructor =
            this->make<Destructor>(std::move(destructorCallableObject));
        if (not maybeDestructor) [[unlikely]]
            return maybeDestructor.error();
        this->impl_arenaPushDestructor(maybeDestructor.unwrap());
    }

    [[nodiscard]] constexpr RestorePoint beginScope() NOEXCEPT
    {
        return RestorePoint(*this);
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> and
                 std::is_trivially_destructible_v<T>
    [[nodiscard]] constexpr Res<T &, alloc::Error> make(Args &&...args) NOEXCEPT
    {
        auto allocationResult = allocate(alloc::Request{
            .numBytes = sizeof(T),
            .alignment = alignof(T),
        });
        if (allocationResult.isError()) [[unlikely]]
            return allocationResult.error();

        u8 *objectStart =
            allocationResult.unwrap().uncheckedAddressOfFirstItem();

        w_assert(u64(objectStart) % alignof(T) == 0,
                 "Misaligned memory produced by allocator");

        T *made = reinterpret_cast<T *>(objectStart);

        std::construct_at(made, std::forward<Args>(args)...);

        return *made;
    }

  protected:
    struct DestructorBase
    {
        Opt<DestructorBase &> prev;
        void (*destructorFunction)(DestructorBase &self);
    };

    constexpr virtual void
    impl_arenaPushDestructor(DestructorBase &entry) NOEXCEPT = 0;

    [[nodiscard]] constexpr virtual Res<Bytes, alloc::Error>
    impl_allocate(const alloc::Request &) NOEXCEPT = 0;

    [[nodiscard]] constexpr virtual void *impl_arenaNewScope() NOEXCEPT = 0;

    constexpr virtual void impl_arenaRestoreScope(void *handle) NOEXCEPT = 0;

    constexpr virtual void impl_deallocate(void *memory,
                                           size_t size_hint) NOEXCEPT = 0;

    [[nodiscard]] constexpr virtual Res<Bytes, alloc::Error>
    impl_reallocate(const alloc::ReallocateRequest &options) NOEXCEPT = 0;
};

export template <typename T>
concept AllocatorType = requires {
    requires std::is_base_of_v<Allocator, T>;
    requires std::is_convertible_v<T &, Allocator &>;
};
