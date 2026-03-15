#include <m4xdev/logging.h>

#include "chunk.h"
#include "manager.h"

#include "iterator.h"
#include "cache.h"
#include "generator.h"
#include "render.h"

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

err_t chunkmanager_create(u16 radius, struct ChunkManager **selfout) {
	if (selfout == NULL) return -1;

	*selfout = NULL;

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

	*selfout = manager;
	return 0;
}

void chunkmanager_destroy(struct ChunkManager *self) {
	assert(self != NULL);

	struct Chunk *chunk = NULL;
	while ((chunk = chunkcache_remove_oldest(self->cache)) != NULL) {
		chunkrenderer_cleanup(chunk);
		chunk_destroy(chunk);
	}

	chunkcache_destroy(self->cache);
	self->cache = NULL;

	free(self);
}

struct Chunk *chunkmanager_get(const struct ChunkManager *self, struct ChunkPos pos) {
	assert(self != NULL);

	return chunkcache_get(self->cache, pos.x, pos.z);
}

err_t chunkmanager_update(struct ChunkManager *self, struct World *world, double blockX, double blockZ, bool load_all) {
	assert(self != NULL);
	assert(world != NULL);
	(void)world;

	size_t generate_limit = 4;
	size_t mesh_limit = 4;

	struct ChunkPos origin = chunk_position_at((i32)blockX, (i32)blockZ);
	struct ChunkIterator it = chunkiterator_create(origin, self->radius);

	struct ChunkPos pos;
	while (chunkiterator_next(&it, &pos)) {
		struct Chunk *chunk = chunkcache_get(self->cache, pos.x, pos.z);
		if (chunk == NULL) {
			if (generate_limit > 0 || load_all) {
				if (chunk_generate(pos.x, pos.z, &chunk) != 0) {
					return -3;
				}

				struct Chunk *evicted = chunkcache_add(self->cache, pos.x, pos.z, chunk);
				if (evicted != NULL) {
					chunkrenderer_cleanup(evicted);
					chunk_destroy(evicted);
					evicted = NULL;
				}

				struct Chunk *north = chunkcache_get(self->cache, pos.x, pos.z - 1);
				if (north != NULL) chunk_set_flag(north, CHUNK_MESH_FLAG);

				struct Chunk *north_east = chunkcache_get(self->cache, pos.x + 1, pos.z - 1);
				if (north_east != NULL) chunk_set_flag(north_east, CHUNK_MESH_FLAG);

				struct Chunk *east = chunkcache_get(self->cache, pos.x + 1, pos.z);
				if (east != NULL) chunk_set_flag(east, CHUNK_MESH_FLAG);

				struct Chunk *south_east = chunkcache_get(self->cache, pos.x + 1, pos.z + 1);
				if (south_east != NULL) chunk_set_flag(south_east, CHUNK_MESH_FLAG);

				struct Chunk *south = chunkcache_get(self->cache, pos.x, pos.z + 1);
				if (south != NULL) chunk_set_flag(south, CHUNK_MESH_FLAG);

				struct Chunk *south_west = chunkcache_get(self->cache, pos.x - 1, pos.z + 1);
				if (south_west != NULL) chunk_set_flag(south_west, CHUNK_MESH_FLAG);

				struct Chunk *west = chunkcache_get(self->cache, pos.x - 1, pos.z);
				if (west != NULL) chunk_set_flag(west, CHUNK_MESH_FLAG);

				struct Chunk *north_west = chunkcache_get(self->cache, pos.x - 1, pos.z - 1);
				if (north_west != NULL) chunk_set_flag(north_west, CHUNK_MESH_FLAG);

				generate_limit--;
			}
		} else {
			if (chunk_get_flag(chunk, CHUNK_MESH_FLAG) && (mesh_limit > 0 || load_all)) {
				const struct Chunk *north_chunk = chunkcache_get(self->cache, pos.x, pos.z - 1);
				const struct Chunk *north_east_chunk = chunkcache_get(self->cache, pos.x + 1, pos.z - 1);
				const struct Chunk *east_chunk = chunkcache_get(self->cache, pos.x + 1, pos.z);
				const struct Chunk *south_east_chunk = chunkcache_get(self->cache, pos.x + 1, pos.z + 1);
				const struct Chunk *south_chunk = chunkcache_get(self->cache, pos.x, pos.z + 1);
				const struct Chunk *south_west_chunk = chunkcache_get(self->cache, pos.x - 1, pos.z + 1);
				const struct Chunk *west_chunk = chunkcache_get(self->cache, pos.x - 1, pos.z);
				const struct Chunk *north_west_chunk = chunkcache_get(self->cache, pos.x - 1, pos.z - 1);

				chunkrenderer_mesh(
					chunk,
					north_chunk,
					north_east_chunk,
					east_chunk,
					south_east_chunk,
					south_chunk,
					south_west_chunk,
					west_chunk,
					north_west_chunk);

				mesh_limit--;
			}
		}
	}

	return 0;
}

err_t chunkmanager_render(struct ChunkManager *self, double blockX, double blockZ) {
	if (self == NULL) return -1;

	struct ChunkPos origin = chunk_position_at(blockX, blockZ);

	struct ChunkPos coord;
	struct ChunkIterator it = chunkiterator_create(origin, self->radius);
	while (chunkiterator_next(&it, &coord)) {
		struct Chunk *chunk = chunkcache_get(self->cache, coord.x, coord.z);
		if (chunk == NULL) continue;

		chunkrenderer_render(chunk);
	}

	return 0;
}
