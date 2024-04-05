#pragma once
#include <allo.h>
#include <raylib.h>
#include <ziglike/zigstdint.h>

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
    static zl::opt<Terrain &> make_with(allo::abstract_allocator_t &ally,
                                        Coord size, u64 fill_amount) noexcept;

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
        zl::slice<TerrainEntry> data;
        zl::slice<Chunk> chunks;
        Material chunk_mat;
    } m;
    Terrain(M members) : m(members) {}
};
} // namespace werm
