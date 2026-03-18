#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <m4xdev/types.h>

#include "chunk.h"
#include "cache.h"

static_assert(CHUNK_WIDTH == 16, "Chunk cache assumes chunk width of 16");

#define KEY_STATE_FREE		0
#define KEY_STATE_TOMBSTONE 1
#define KEY_STATE_OCCUPIED	2

#define KEY_STATE_BITS		 2
#define KEY_STATE_MASK		 ((1UL << KEY_STATE_BITS) - 1)
#define KEY_CHUNK_POS_Z_BITS 28
#define KEY_CHUNK_POS_Z_MASK ((1UL << KEY_CHUNK_POS_Z_BITS) - 1)
#define KEY_CHUNK_POS_X_BITS 28
#define KEY_CHUNK_POS_X_MASK ((1UL << KEY_CHUNK_POS_X_BITS) - 1)

static inline u64 chunkcache_key(i64 chunkX, i64 chunkZ) {
	assert(chunkX >= CHUNK_POS_MIN_X && chunkX <= CHUNK_POS_MAX_X);
	assert(chunkZ >= CHUNK_POS_MIN_Z && chunkZ <= CHUNK_POS_MAX_Z);

	return ((u64)chunkX & KEY_CHUNK_POS_X_MASK) << (KEY_CHUNK_POS_X_BITS + KEY_STATE_BITS) |
		((u64)chunkZ & KEY_CHUNK_POS_Z_MASK) << KEY_STATE_BITS;
}

#define INVALID_INDEX USHRT_MAX

struct ChunkNode {
	u64 key;
	u64 next, prev;
	struct Chunk *data;
};

struct ChunkCache {
	u64 head, tail;
	size_t capacity;
	u64 mask;
	size_t highwater;
	size_t size;
	struct ChunkNode data[];
};

err_t chunkcache_create(size_t capacity, size_t highwater, struct ChunkCache **cacheout) {
	if ((capacity & (capacity - 1)) != 0) return -1;				 // Capacity must be power of 2
	if (highwater > capacity || highwater < capacity / 2) return -1; // highwater must be less then capacity but greater then half capacity.
	if (cacheout == NULL) return -1;

	*cacheout = NULL;

	struct ChunkCache *cache = malloc(sizeof(struct ChunkCache) + sizeof(struct ChunkNode) * capacity);
	if (cache == NULL) {
		perror("malloc");
		return -2;
	}

	for (u64 i = 0; i < capacity; i++) {
		struct ChunkNode *node = &cache->data[i];
		node->key = KEY_STATE_FREE;
		node->prev = INVALID_INDEX;
		node->next = INVALID_INDEX;
		node->data = NULL;
	}

	cache->head = INVALID_INDEX;
	cache->tail = INVALID_INDEX;
	cache->capacity = capacity;
	cache->mask = capacity - 1;
	cache->highwater = highwater;
	cache->size = 0;

	*cacheout = cache;

	return 0;
}

void chunkcache_destroy(struct ChunkCache *cache) {
	assert(cache != NULL);
	assert(cache->size == 0);

	free(cache);
	cache = NULL;
}

/**
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */
static inline u64 key_preferred_index(u64 key, size_t mask) {
	const u8 *bytes = (const u8 *)&key;
	u64 hash = 1469598103934665603ULL;	// offset basis
	const u64 prime = 1099511628211ULL; // FNV prime

	for (size_t i = 0; i < sizeof(key); i++) {
		hash ^= (u64)bytes[i];
		hash *= prime;
	}

	return hash & mask;
}

static inline void detach_index(struct ChunkCache *cache, u64 index) {
	assert(cache != NULL);

	struct ChunkNode *node = &cache->data[index];

	if (node->prev != INVALID_INDEX) {
		cache->data[node->prev].next = node->next;
	} else {
		cache->head = node->next;
	}

	if (node->next != INVALID_INDEX) {
		cache->data[node->next].prev = node->prev;
	} else {
		cache->tail = node->prev;
	}

	node->prev = INVALID_INDEX;
	node->next = INVALID_INDEX;
}

static inline void attach_index_at_head(struct ChunkCache *cache, u64 index) {
	assert(cache != NULL);

	struct ChunkNode *node = &cache->data[index];

	node->prev = INVALID_INDEX;
	node->next = cache->head;

	if (cache->head != INVALID_INDEX) {
		cache->data[cache->head].prev = index;
	}

	cache->head = index;

	if (cache->tail == INVALID_INDEX) {
		cache->tail = index;
	}
}

static inline u64 get_key_index(struct ChunkCache *cache, u64 key) {
	assert(cache != NULL);

	u64 preferred_index = key_preferred_index(key, cache->mask);
	u64 index = preferred_index;

	do {
		struct ChunkNode *node = &cache->data[index];

		u64 state = node->key & KEY_STATE_MASK;
		if (state == KEY_STATE_FREE) break;

		u64 current_key = node->key & ~KEY_STATE_MASK;
		if (state == KEY_STATE_OCCUPIED && current_key == key) {
			return index;
		}

		index = (index + 1) & cache->mask;
	} while (index != preferred_index);

	return INVALID_INDEX;
}

struct Chunk *chunkcache_get(struct ChunkCache *cache, i64 chunkX, i64 chunkZ) {
	assert(cache != NULL);
	assert(chunkX >= CHUNK_POS_MIN_X && chunkX <= CHUNK_POS_MAX_X);
	assert(chunkZ >= CHUNK_POS_MIN_Z && chunkZ <= CHUNK_POS_MAX_Z);

	u64 index = get_key_index(cache, chunkcache_key(chunkX, chunkZ));
	if (index == INVALID_INDEX) {
		return NULL;
	}

	struct ChunkNode *node = &cache->data[index];

	detach_index(cache, index);
	attach_index_at_head(cache, index);

	return node->data;
}

static inline struct Chunk *chunkcache_remove_index(struct ChunkCache *cache, u64 index) {
	assert(cache != NULL);
	assert(index != INVALID_INDEX);

	struct ChunkNode *node = &cache->data[index];
	struct Chunk *chunk = node->data;

	detach_index(cache, index);

	u64 next_index = (index + 1) & cache->mask;

	node->data = NULL;
	if ((cache->data[next_index].key & KEY_STATE_MASK) == KEY_STATE_FREE) {
		node->key = KEY_STATE_FREE;

		size_t i = index;
		while (true) {
			size_t prev = (i - 1) & cache->mask;
			if ((cache->data[prev].key & KEY_STATE_MASK) != KEY_STATE_TOMBSTONE) break;
			cache->data[prev].key = KEY_STATE_FREE;
			i = prev;
		}
	} else {
		node->key = KEY_STATE_TOMBSTONE;
	}

	cache->size--;

	return chunk;
}

static inline struct Chunk *chunkcache_remove_key(struct ChunkCache *cache, u64 key) {
	assert(cache != NULL);

	u64 index = get_key_index(cache, key);
	if (index == INVALID_INDEX) {
		return NULL;
	}

	return chunkcache_remove_index(cache, index);
}

struct Chunk *chunkcache_remove(struct ChunkCache *cache, i64 chunkX, i64 chunkZ) {
	return chunkcache_remove_key(cache, chunkcache_key(chunkX, chunkZ));
}

struct Chunk *chunkcache_add(struct ChunkCache *cache, i64 chunkX, i64 chunkZ, struct Chunk *chunkin) {
	assert(cache != NULL);
	assert(chunkX >= CHUNK_POS_MIN_X && chunkX <= CHUNK_POS_MAX_X);
	assert(chunkZ >= CHUNK_POS_MIN_Z && chunkZ <= CHUNK_POS_MAX_Z);
	assert(chunkin != NULL);

	u64 key = chunkcache_key(chunkX, chunkZ);

	u64 existing_index = get_key_index(cache, key);
	if (existing_index != INVALID_INDEX) {
		return NULL; // Cache entry already exists.
	}

	struct Chunk *evicted = NULL;
	if (cache->size + 1 > cache->highwater) {
		evicted = chunkcache_remove_index(cache, cache->tail);
	}

	u64 preferred_index = key_preferred_index(key, cache->mask);
	u64 index = preferred_index;

	struct ChunkNode *node = NULL;
	do {
		node = &cache->data[index];
		u8 state = node->key & KEY_STATE_MASK;
		if (state == KEY_STATE_FREE || state == KEY_STATE_TOMBSTONE) break;

		index = (index + 1) & cache->mask;
	} while (index != preferred_index);

	node->key = (key | KEY_STATE_OCCUPIED);
	node->data = chunkin;

	attach_index_at_head(cache, index);

	cache->size++;

	return evicted;
}

struct Chunk *chunkcache_remove_oldest(struct ChunkCache *cache) {
	assert(cache != NULL);

	if (cache->tail == INVALID_INDEX) return NULL;

	return chunkcache_remove_index(cache, cache->tail);
}
