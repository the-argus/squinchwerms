#include "terrain.h"
#include <okay/macros/foreach.h>
#include <okay/opt.h>
#include <raymath.h>

namespace werm {
ok::opt_t<Terrain &> Terrain::make_with(ok::allocator_t &ally, Coord size,
                                        u64 fill_amount) noexcept
{
    auto maybe_mem = allo::alloc_one<Terrain>(ally);
    if (!maybe_mem.okay())
        return {};

    Terrain &terrain = maybe_mem.release();

    // try to allocate 16 byte aligned terrain entries, theyre only a single
    // byte aligned by default
    auto maybe_data_mem =
        allo::alloc<TerrainEntry, decltype(ally), 16>(ally, size.x * size.y);

    if (!maybe_data_mem.okay())
        return {};

    u64 chunks_vertical = (size.y % chunksize != 0) + (size.y / chunksize);
    u64 chunks_horizontal = (size.x % chunksize != 0) + (size.x / chunksize);

    auto maybe_chunks =
        allo::alloc<Chunk>(ally, chunks_vertical * chunks_horizontal);

    if (!maybe_chunks.okay())
        return {};

    terrain = M{
        .width = size.x,
        .height = size.y,
        .width_chunks = chunks_horizontal,
        .height_chunks = chunks_vertical,
        .data = maybe_data_mem.release(),
        .chunks = maybe_chunks.release(),
        .chunk_mat = LoadMaterialDefault(),
    };

    for (auto &entry : terrain.m.data) {
        entry = {.type = TerrainType::Empty};
    }

    for (auto &mesh : terrain.m.chunks) {
        mesh = {
            .mesh = GenMeshPlane(chunksize, chunksize, 1, 1),
            .dirty = false,
        };
    }

    auto unload_gpu_resources_from_terrain = [](void *data) {
        auto *t = (Terrain *)data;
        UnloadMaterial(t->m.chunk_mat);
        for (auto &chunk : t->m.chunks) {
            UnloadMesh(chunk.mesh);
        }
    };

    if (!ally.register_destruction_callback(unload_gpu_resources_from_terrain,
                                            &terrain)
             .okay()) {
        unload_gpu_resources_from_terrain(&terrain);
        return {};
    }

    return terrain;
}

auto Terrain::get(const Coord &coord) const noexcept -> TerrainEntry
{
    assert(coord.x < m.width && coord.y < m.height);
    return m.data.data()[coord.x + (m.width * coord.y)];
}

void Terrain::set(const Coord &coord, TerrainEntry value) noexcept
{
    assert(coord.x < m.width && coord.y < m.height);
    m.data.data()[coord.x + (m.width * coord.y)] = value;
}

void Terrain::draw() const noexcept
{
    Matrix transform = MatrixIdentity();
    ok_foreach(const auto &chunk, m.chunks)
    {
        DrawMesh(chunk.mesh, m.chunkMaterial, transform);
    }
}
} // namespace werm
