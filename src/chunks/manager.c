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

	size_t generate_limit = 100; // TODO move out to a define(state.h?)
	size_t mesh_limit = 100; // TODO move out to a define(state.h?)

	struct ChunkCoord origin = chunkcoord_from_blockpos(blockX, blockZ);
	struct ChunkIterator it = chunkiterator_create(origin, manager->radius);

	struct ChunkCoord coord;
	while (chunkiterator_next(&it, &coord) == 0) {
		struct Chunk *chunk = NULL;
		if (chunkcache_get(manager->cache, coord.x, coord.z, &chunk) != 0) {
			return -2;
		}

		if (chunk == NULL) {
			if (generate_limit > 0) {
				LOG_DEBUG("Generating Chunk X: %d Z: %d", coord.x, coord.z);

				if (generate_chunk(coord.x, coord.z, &chunk) != 0) {
					return -3;
				}

				struct Chunk *evicted = NULL;
				if (chunkcache_add(manager->cache, coord.x, coord.z, chunk, &evicted) != 0) {
					return -3;
				}

				if (evicted != NULL) {
					destroy_chunk(evicted);
					evicted = NULL;
				}

				// TODO mark neighbors dirty/remesh

				generate_limit--;
			}
		} else {
			if (chunk->gl.vao == 0 && mesh_limit > 0) {
				struct Chunk *north_chunk = NULL; // TODO fetch surrounding chunks for meshing.
				struct Chunk *east_chunk = NULL;
				struct Chunk *south_chunk = NULL;
				struct Chunk *west_chunk = NULL;

				if (mesh_chunk(chunk, north_chunk, east_chunk, south_chunk, west_chunk) != 0) {
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
		struct Chunk *chunk = NULL;
		if (chunkcache_get(manager->cache, coord.x, coord.z, &chunk) != 0) {
			return -2;
		}

		if (chunk == NULL || chunk->gl.vao == 0) {
			continue;
		}

		// LOG_DEBUG("Rendering Chunk X: %d Z: %d", coord.x, coord.z);

		mat4 model;
		glm_mat4_identity(model);

		vec3 chunk_position = {chunk->chunkX * CHUNK_WIDTH, 0, chunk->chunkZ * CHUNK_WIDTH};
		glm_translate(model, chunk_position);

		shader_uniform_mat4(Shaders[TERRAIN_SHADER], "model", model);

		render_chunk(chunk);
	}

	return 0;
}
