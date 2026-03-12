module;

#include "macros.h"

#include <algorithm> // for std::max
#include <cstring>
#include <memory> // for std::align

export module arena;

import allocator;
import slice;
import res;
import opt;
import aliases;

export class Arena : public Allocator
{
  public:
    constexpr explicit Arena(Bytes static_buffer) NOEXCEPT;
    constexpr explicit Arena(Allocator &backing_allocator) NOEXCEPT;

    Arena(Arena &&other) = delete;
    Arena &operator=(Arena &&other) = delete;

    Arena &operator=(const Arena &) = delete;
    Arena(const Arena &) = delete;

    constexpr ~Arena() NOEXCEPT { destroy(); }

    constexpr void clear() NOEXCEPT;

  protected:
    [[nodiscard]] constexpr Res<Bytes, alloc::Error>
    impl_allocate(const alloc::Request &) NOEXCEPT final;

    [[nodiscard]] constexpr void *impl_arenaNewScope() NOEXCEPT override;
    constexpr void impl_arenaRestoreScope(void *handle) NOEXCEPT override;

    constexpr void
    impl_arenaPushDestructor(DestructorBase &entry) NOEXCEPT override;

    constexpr void impl_deallocate(void *memory,
                                   size_t size_hint) NOEXCEPT final
    {
        // deallocating with an arena is a no-op
        return;
    }

    constexpr Res<Bytes, alloc::Error>
    impl_reallocate(const alloc::ReallocateRequest &options) NOEXCEPT final
    {
        // just allocate a new block with the new size and do a copy
        Res allocation = this->allocate(alloc::Request{
            .numBytes = options.calculatePreferredSize(),
            .alignment = options.alignment,
        });

        if (!allocation.isSuccess()) [[unlikely]]
            return allocation;

        Bytes newmem = allocation.unwrap();

        // ignore result of memcopy
        auto &&_ =
            memcopy(MemcopyOptions{.to = newmem, .from = options.memory});

        // freeing the old allocation is not possible with arena
        return newmem;
    }

  private:
    enum class DestructorListClearMode
    {
        ClearAll,
        StopAfterCurrentScope,
    };

    constexpr void destroy() NOEXCEPT;
    constexpr void callAllDestructors(DestructorListClearMode mode) NOEXCEPT;

    Bytes m_memory;
    u64 m_firstAvailableByteIndex;
    Opt<Allocator &> m_backing;
    Opt<DestructorBase &> m_lastPushedDestructor;
};

constexpr Arena::Arena(Bytes staticBuffer) NOEXCEPT
    : m_memory(staticBuffer),
      m_firstAvailableByteIndex(0)
{
}

constexpr Arena::Arena(Allocator &backing_allocator) NOEXCEPT
    : m_memory(makeNullSlice<u8>()),
      m_firstAvailableByteIndex(0),
      m_backing(backing_allocator)
{
}

constexpr void Arena::destroy() NOEXCEPT
{
    callAllDestructors(DestructorListClearMode::ClearAll);
    if (m_backing && !m_memory.isEmpty()) {
        m_backing.unwrap().deallocate(m_memory.uncheckedAddressOfFirstItem());
    }
}

constexpr void Arena::callAllDestructors(DestructorListClearMode mode) NOEXCEPT
{
    Opt<DestructorBase &> node = m_lastPushedDestructor;
    while (node) {
        DestructorBase &noderef = node.unwrap();

        if (mode == DestructorListClearMode::StopAfterCurrentScope &&
            noderef.destructorFunction == nullptr) [[unlikely]] {
            m_lastPushedDestructor = noderef.prev;
            return;
        }

        noderef.destructorFunction(noderef);
        node = noderef.prev;
    }
    m_lastPushedDestructor.reset();
}

[[nodiscard]] constexpr Res<Bytes, alloc::Error>
Arena::impl_allocate(const alloc::Request &request) NOEXCEPT
{
    using namespace alloc;
    constexpr auto extraBookkeepingBytes = 100;
    // handle first-time allocation case
    if (m_memory.isEmpty()) [[unlikely]] {
        if (!m_backing) [[unlikely]]
            return alloc::Error::OOM;

        w_assert(m_firstAvailableByteIndex == 0, "");

        Allocator &backing = m_backing.unwrap();
        const alloc::Request backing_request{
            .numBytes = request.numBytes + extraBookkeepingBytes,
            .alignment = request.alignment,
        };

        Res result = backing.allocate(backing_request);
        if (isSuccess(result)) [[likely]] {
            m_memory = result.unwrap();
            m_firstAvailableByteIndex = 0;
        } else [[unlikely]] {
            return result;
        }
    }

    // handle needs reallocation case
    const auto alignOrReallocInPlace = [&]() -> Res<u8 *, alloc::Error> {
        const auto getAlignedStart = [this] {
            // NOTE: this might return a pointer off the end of the memory,
            // but if so then get_space_remaining() should return 0
            return m_memory.uncheckedAddressOfFirstItem() +
                   m_firstAvailableByteIndex;
        };
        const auto getSpaceRemaining = [this] {
            w_assert(m_firstAvailableByteIndex <= m_memory.size(), "");
            return m_memory.size() - m_firstAvailableByteIndex;
        };

        void *alignedStartVoidptr = getAlignedStart();
        size_t spaceRemainingAfterAlignment = getSpaceRemaining();

        if (!std::align(request.alignment, request.numBytes,
                        alignedStartVoidptr, spaceRemainingAfterAlignment)) {

            if (!m_backing) [[unlikely]]
                return alloc::Error::OOM;

            auto &backing = m_backing.unwrap();

            constexpr auto growth_factor = 2;

            auto maybeNewMemory = backing.reallocate(ReallocateRequest{
                .memory = m_memory,
                .newSizeBytes = std::max(m_memory.size() * growth_factor,
                                         m_memory.size() + request.numBytes +
                                             extraBookkeepingBytes),
                .inPlaceOrElseFail = true,
            });

            if (!isSuccess(maybeNewMemory)) [[unlikely]]
                return maybeNewMemory.error();

            w_assert(maybeNewMemory.unwrap().addressOfFirstItem() ==
                         m_memory.addressOfFirstItem(),
                     "");
            w_assert(alignedStartVoidptr == getAlignedStart(), "");
            w_assert(maybeNewMemory.unwrap().size() > m_memory.size(), "");
            w_assert(m_firstAvailableByteIndex <= m_memory.size(), "");

            m_memory = maybeNewMemory.unwrap();
            spaceRemainingAfterAlignment = getSpaceRemaining();

            w_assert(m_firstAvailableByteIndex < m_memory.size(), "");

            // re-align
#ifndef NDEBUG
            const bool align_succeeded =
#endif
                std::align(request.alignment, request.numBytes,
                           alignedStartVoidptr, spaceRemainingAfterAlignment);
            w_assert(align_succeeded, "");
        }

        u8 *const alignedStart = static_cast<u8 *>(alignedStartVoidptr);

        return alignedStart;
    };

    const auto maybeAlignedStart = alignOrReallocInPlace();

    if (!isSuccess(maybeAlignedStart)) [[unlikely]]
        return maybeAlignedStart.error();

    u8 *const alignedStart = maybeAlignedStart.unwrap();

    u8 *const newAvailableStart = alignedStart + request.numBytes;

    w_assert(m_firstAvailableByteIndex < m_memory.size(), "");

    m_firstAvailableByteIndex =
        newAvailableStart - m_memory.uncheckedAddressOfFirstItem();

    // its okay for m_first_available_byte_index to be *equal* to memory.size()
    // here, that means things are full
    w_assert(m_firstAvailableByteIndex <= m_memory.size(), "");

    ::memset(alignedStart, 0, request.numBytes);
    return unsafe::rawSlice(*alignedStart, request.numBytes);
}

[[nodiscard]] constexpr void *Arena::impl_arenaNewScope() NOEXCEPT
{
    // a null destructor indicates a change in scope
    Res destructor = this->make<DestructorBase>();
    // NOTE: if we have no space for a destructor, then restoring the scope
    // won't work. That's fine because we will OOM after this anyways
    if (isSuccess(destructor)) [[likely]]
        impl_arenaPushDestructor(destructor.unwrap());
    return std::bit_cast<void *>(m_firstAvailableByteIndex);
}

constexpr void Arena::impl_arenaRestoreScope(void *handle) NOEXCEPT
{
    callAllDestructors(DestructorListClearMode::StopAfterCurrentScope);
    w_assert(m_firstAvailableByteIndex < m_memory.size(), "");
    m_firstAvailableByteIndex = std::bit_cast<size_t>(handle);
    w_assert(m_firstAvailableByteIndex < m_memory.size(), "");
}

constexpr void
Arena::impl_arenaPushDestructor(DestructorBase &destructor) NOEXCEPT
{
    destructor.prev = m_lastPushedDestructor;
    m_lastPushedDestructor = destructor;
}

constexpr void Arena::clear() NOEXCEPT
{
    callAllDestructors(DestructorListClearMode::ClearAll);
#ifndef NDEBUG
    memfill(m_memory, 0x69);
#endif
    m_firstAvailableByteIndex = 0;
}
