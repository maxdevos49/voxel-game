#include <m4xdev/logging.h>

#include "chunk.h"
#include "manager.h"

#include "iterator.h"
#include "cache.h"
#include "generator.h"
#include "render.h"

#define MAX_VIEW_RADIUS 64

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
	if (radius > MAX_VIEW_RADIUS) return -1;

	*selfout = NULL;

	struct ChunkManager *manager = malloc(sizeof(struct ChunkManager));
	if (manager == NULL) {
		perror("malloc");
		return -2;
	}

	manager->radius = radius;
	manager->cache = NULL;

	size_t cache_capacity = nextpow2(CHUNK_VIEW_AREA(MAX_VIEW_RADIUS) * 2);
	size_t cache_view_highwater = CHUNK_VIEW_AREA(radius) + CHUNK_RING_COUNT(radius + 1) + CHUNK_RING_COUNT(radius + 2);

	struct ChunkCache *cache = NULL;
	if (chunkcache_create(cache_capacity, cache_view_highwater, &cache) != 0) {
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

void chunkmanager_view_radius_set(struct ChunkManager *self, u64 radius) {
	assert(self != NULL);

	size_t cache_view_highwater = CHUNK_VIEW_AREA(radius) + CHUNK_RING_COUNT(radius + 1) + CHUNK_RING_COUNT(radius + 2);
	if (chunkcache_resize(self->cache, cache_view_highwater) == 0) {
		self->radius = radius;
	}
}

struct Chunk *chunkmanager_get(const struct ChunkManager *self, struct ChunkPos pos) {
	assert(self != NULL);

	return chunkcache_get(self->cache, pos.x, pos.z);
}

err_t chunkmanager_preload(struct ChunkManager *self, double blockX, double blockZ) {
	struct Chunk *chunk = NULL;
	struct ChunkPos pos;
	struct ChunkIterator it;

	struct ChunkPos origin = chunk_position_at(blockX, blockZ);

	it = chunkiterator_create(origin, self->radius);
	while (chunkiterator_next(&it, &pos)) {
		chunk_generate(pos.x, pos.z, &chunk);
		chunkcache_add(self->cache, pos.x, pos.z, chunk);
	}

	it = chunkiterator_create(origin, self->radius);
	while (chunkiterator_next(&it, &pos)) {
		chunk = chunkcache_get(self->cache, pos.x, pos.z);
		if (chunk == NULL) continue;

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
	}

	return 0;
}

err_t chunkmanager_update(struct ChunkManager *self, double blockX, double blockZ) {
	assert(self != NULL);

	size_t generate_limit = 4;
	size_t mesh_limit = 4;

	struct ChunkPos origin = chunk_position_at(blockX, blockZ);
	struct ChunkIterator it = chunkiterator_create(origin, self->radius);

	struct ChunkPos pos;
	while (chunkiterator_next(&it, &pos)) {
		struct Chunk *chunk = chunkcache_get(self->cache, pos.x, pos.z);
		if (chunk == NULL) {
			if (generate_limit > 0) {
				if (chunk_generate(pos.x, pos.z, &chunk) != 0) {
					return -3;
				}

				chunkcache_add(self->cache, pos.x, pos.z, chunk);

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
			if (chunk_get_flag(chunk, CHUNK_MESH_FLAG) && (mesh_limit > 0)) {
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

	struct Chunk *removed = NULL;
	while (chunkcache_drain(self->cache)) {
		removed = chunkcache_remove_oldest(self->cache);
		if (removed != NULL) {
			chunkrenderer_cleanup(removed);
			chunk_destroy(removed);
			removed = NULL;
		}
	}

	return 0;
}

err_t chunkmanager_render(struct ChunkManager *self, double blockX, double blockZ) {
	if (self == NULL) return -1;

	struct ChunkPos origin = chunk_position_at(blockX, blockZ);

	struct ChunkPos coord;
	struct ChunkIterator it = chunkiterator_create(origin, self->radius);
	struct Chunk *chunk = NULL;

	while (chunkiterator_next(&it, &coord)) {
		chunk = chunkcache_get(self->cache, coord.x, coord.z);
		if (chunk == NULL) continue;

		chunkrenderer_render(chunk);
	}

	return 0;
}
