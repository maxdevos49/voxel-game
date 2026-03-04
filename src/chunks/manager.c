#include <m4xdev/logging.h>

#include "cache.h"
#include "chunk.h"
#include "manager.h"
#include "iterator.h"
#include "../graphics/shader.h"

struct ChunkManager {
	u16 radius;
	struct ChunkCache *cache;
};

static inline u64 nextpow2(u64 v) { // TODO move to a utility file.
	if (v <= 0) return 0;

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 3;
	v |= v >> 4;
	v |= v >> 16;
	v |= v >> 32;
	v++;

	return v;
}

err_t chunkmanager_create(u16 radius, struct ChunkManager **managerout) {
	if (managerout == NULL) return -1;

	*managerout = NULL;

	struct ChunkManager *manager = malloc(sizeof(struct ChunkManager));
	if (manager == NULL) {
		perror("malloc");
		return -2;
	}

	manager->radius = radius;
	manager->cache = NULL;

	size_t chunk_count = CHUNK_VIEW_AREA(radius) * 1.25f;
	size_t cache_size = nextpow2(chunk_count);

	struct ChunkCache *cache = NULL;
	if (chunkcache_create(cache_size, &cache) != 0) {
		free(manager);
		manager = NULL;
		return -3;
	}

	manager->cache = cache;

	*managerout = manager;
	return 0;
}

static inline struct ChunkCoord chunkcoord_from_blockpos(double blockX, double blockZ) { // TODO move to chunk.h
	return (struct ChunkCoord){
		.x = (i32)(blockX / CHUNK_WIDTH),
		.z = (i32)(blockZ / CHUNK_WIDTH)};
}

err_t chunkmanager_update(struct ChunkManager *manager, double blockX, double blockZ) {
	if (manager == NULL) return -1;

	size_t generate_limit = 5; // TODO move out to a define(state.h?)
	size_t mesh_limit = 5;	   // TODO move out to a define(state.h?)

	struct ChunkCoord origin = chunkcoord_from_blockpos(blockX, blockZ);
	struct ChunkIterator it = chunkiterator_create(origin, manager->radius);

	struct ChunkCoord coord;
	while (chunkiterator_next(&it, &coord) == 0) {
		struct Chunk *chunk = chunkcache_get(manager->cache, coord.x, coord.z);
		if (chunk == NULL) {
			if (generate_limit > 0) {
				LOG_DEBUG("Generating Chunk X: %d Z: %d", coord.x, coord.z);

				if (chunk_generate(coord.x, coord.z, &chunk) != 0) {
					return -3;
				}

				struct Chunk *evicted = chunkcache_add(manager->cache, coord.x, coord.z, chunk);
				if (evicted != NULL) {
					chunk_destroy(evicted);
					evicted = NULL;
				}

				struct Chunk *north = chunkcache_get(manager->cache, coord.x, coord.z - 1);
				if (north != NULL) chunk_set_flag(north, CHUNK_MESH_FLAG);

				struct Chunk *east = chunkcache_get(manager->cache, coord.x + 1, coord.z);
				if (east != NULL) chunk_set_flag(east, CHUNK_MESH_FLAG);

				struct Chunk *south = chunkcache_get(manager->cache, coord.x, coord.z + 1);
				if (south != NULL) chunk_set_flag(south, CHUNK_MESH_FLAG);

				struct Chunk *west = chunkcache_get(manager->cache, coord.x - 1, coord.z);
				if (west != NULL) chunk_set_flag(west, CHUNK_MESH_FLAG);

				generate_limit--;
			}
		} else {
			if (chunk_get_flag(chunk, CHUNK_MESH_FLAG) && mesh_limit > 0) {
				struct Chunk *north = chunkcache_get(manager->cache, coord.x, coord.z - 1);
				struct Chunk *east = chunkcache_get(manager->cache, coord.x + 1, coord.z);
				struct Chunk *south = chunkcache_get(manager->cache, coord.x, coord.z + 1);
				struct Chunk *west = chunkcache_get(manager->cache, coord.x - 1, coord.z);

				if (mesh_chunk(chunk, north, east, south, west) != 0) {
					return -4;
				}

				mesh_limit--;
			}
		}
	}

	return 0;
}

err_t chunkmanager_render(struct ChunkManager *manager, double blockX, double blockZ) {
	if (manager == NULL) return -1;

	struct ChunkCoord origin = chunkcoord_from_blockpos(blockX, blockZ);

	struct ChunkCoord coord;
	struct ChunkIterator it = chunkiterator_create(origin, manager->radius);
	while (chunkiterator_next(&it, &coord) == 0) {
		struct Chunk *chunk = chunkcache_get(manager->cache, coord.x, coord.z);
		if (chunk == NULL || !chunk_get_flag(chunk, CHUNK_RENDERABLE_FLAG)) {
			continue;
		}

		mat4 model;
		glm_mat4_identity(model);

		vec3 chunk_position = {coord.x * CHUNK_WIDTH, 0, coord.z * CHUNK_WIDTH};
		glm_translate(model, chunk_position);

		shader_uniform_mat4(Shaders[TERRAIN_SHADER], "model", model);

		render_chunk(chunk);
	}

	return 0;
}
