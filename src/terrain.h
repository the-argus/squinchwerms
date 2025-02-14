#pragma once
#include <okay/allocators/allocator.h>
#include <okay/opt.h>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>

namespace werm {
class Terrain
{
    enum class TerrainType : u8
    {
        Empty,
        Dirt,
        Food,
    };

    struct TerrainEntry
    {
        TerrainType type;
    };

    struct Chunk
    {
        Mesh mesh;
        bool dirty = false;
    };

  public:
    struct Coord
    {
        u64 x;
        u64 y;
    };

    /// Create a terrain into an allocator, return a reference to it or error
    /// fill_amount is how many rows should be filled with dirt
    static ok::opt_t<Terrain &> make_with(ok::allocator_t &ally, Coord size,
                                          u64 fill_amount) noexcept;

    [[nodiscard]] TerrainEntry get(const Coord &coord) const noexcept;
    void set(const Coord &coord, TerrainEntry value) noexcept;
    void draw() const noexcept;

  private:
    static constexpr u64 chunksize = 16;

    struct M
    {
        u64 width;
        u64 height;
        u64 width_chunks;
        u64 height_chunks;
        ok::slice_t<TerrainEntry> data;
        ok::slice_t<Chunk> chunks;
        Material chunk_mat;
    } m;
    Terrain(M members) : m(members) {}
};
} // namespace werm
