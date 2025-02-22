#pragma once
#include <utility>

#include <raylib.h>

#include <okay/allocators/allocator.h>
#include <okay/allocators/arena.h>

#include "physics.h"
#include "linked_pool.h"

namespace werm {

struct Game
{
    ok::allocator_t &backingHeapAllocator;
    ok::arena_t &levelArena;
	LinkedPool<::Mesh> meshes;
	Physics physics;

	constexpr void destroyMesh(::Mesh& mesh) {
		UnloadMesh(mesh);
		meshes.destroyAndReclaim(mesh);
	}
};

} // namespace werm
