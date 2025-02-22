#pragma once
#include <utility>
// NOTE: the above fixes problem in allocator.h header, missing std::forward
#include <okay/allocators/allocator.h>
#include <okay/opt.h>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>

namespace werm {

struct amount_filled_tag
{};

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

    Terrain(amount_filled_tag, Coord size, u64 rows_filled_with_dirt) noexcept;

    [[nodiscard]] TerrainEntry get(const Coord &coord) const noexcept;
    void set(const Coord &coord, TerrainEntry value) noexcept;
    void draw() const noexcept;

  private:
    static constexpr u64 chunkSize = 16;

    struct M
    {
        u64 width;
        u64 height;
        u64 widthChunks;
        u64 heightChunks;
        ok::slice_t<TerrainEntry> data;
        ok::slice_t<Chunk> chunks;
        Material chunkMaterial;
    } m;
    Terrain(M members) : m(members) {}
};
} // namespace werm
