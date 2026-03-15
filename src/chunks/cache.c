#include <stdio.h>
#include <string.h>
#include <m4xdev/types.h>

#include "chunk.h"
#include "cache.h"

#define HM_FREE		 1
#define HM_TOMBSTONE 2
#define HM_OCCUPIED	 3

struct ChunkKey {
	i32 x;
	i32 z;
};

struct ChunkNode {
	u64 state;

	struct ChunkKey key;
	struct Chunk *data;

	struct ChunkNode *next;
	struct ChunkNode *prev;
};

struct ChunkCache {
	struct ChunkNode *head;
	struct ChunkNode *tail;

	size_t capacity;
	size_t size;
	struct ChunkNode data[];
};

err_t chunkcache_create(size_t capacity, struct ChunkCache **cacheout) {
	if ((capacity & (capacity - 1)) != 0) return -1; // Capacity must be power of 2
	if (cacheout == NULL) return -1;

	*cacheout = NULL;

	struct ChunkCache *cache = malloc(sizeof(struct ChunkCache) + sizeof(struct ChunkNode) * capacity);
	if (cache == NULL) {
		perror("malloc");
		return -2;
	}

	for (u32 i = 0; i < capacity; i++) {
		struct ChunkNode *node = &cache->data[i];
		node->state = HM_FREE;
		node->key = (struct ChunkKey){0};
		node->data = NULL;
		node->prev = NULL;
		node->next = NULL;
	}

	cache->head = NULL;
	cache->tail = NULL;
	cache->capacity = capacity;
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
static inline u64 hash_key(i32 chunkX, i32 chunkZ, size_t capacity) {
	struct ChunkKey key = {chunkX, chunkZ};

	const u8 *bytes = (const u8 *)&key;
	u64 hash = 1469598103934665603ULL;	// offset basis
	const u64 prime = 1099511628211ULL; // FNV prime

	for (size_t i = 0; i < sizeof(struct ChunkKey); i++) {
		hash ^= (u64)bytes[i];
		hash *= prime;
	}

	return hash & (capacity - 1);
}

static inline void detach_node(struct ChunkCache *cache, struct ChunkNode *node) {
	assert(cache != NULL);
	assert(node != NULL);

	if (cache->head == node) {
		cache->head = node->next;
	}

	if (cache->tail == node) {
		cache->tail = node->prev;
	}

	if (node->prev != NULL) {
		node->prev->next = node->next;
	}

	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	node->prev = NULL;
	node->next = NULL;
}

static inline void attach_head_node(struct ChunkCache *cache, struct ChunkNode *node) {
	assert(cache != NULL);
	assert(node != NULL);

	node->next = cache->head;

	if (cache->head != NULL) {
		cache->head->prev = node;
	}

	cache->head = node;

	if (cache->tail == NULL) {
		cache->tail = node;
	}
}

static inline bool get_node_index(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, u64 *indexout) {
	assert(cache != NULL);
	assert(indexout != NULL);

	u64 preferred_index = hash_key(chunkX, chunkZ, cache->capacity);
	u64 index = preferred_index;

	do {
		struct ChunkNode *node = &cache->data[index];
		if (node->state == HM_FREE) break;

		if (node->state == HM_OCCUPIED && node->key.x == chunkX && node->key.z == chunkZ) {
			*indexout = index;
			return true;
		}

		index++;
		index &= (cache->capacity - 1);
	} while (index != preferred_index);

	return false;
}

struct Chunk *chunkcache_get(struct ChunkCache *cache, i32 chunkX, i32 chunkZ) {
	assert(cache != NULL);

	u64 index = 0;
	if (!get_node_index(cache, chunkX, chunkZ, &index)) {
		return NULL;
	}

	assert(index >= 0 && index < cache->capacity);
	struct ChunkNode *node = &cache->data[index];

	detach_node(cache, node);
	attach_head_node(cache, node);

	return node->data;
}

struct Chunk *chunkcache_remove(struct ChunkCache *cache, i32 chunkX, i32 chunkZ) {
	assert(cache != NULL);

	u64 index = 0;
	if (!get_node_index(cache, chunkX, chunkZ, &index)) {
		return NULL;
	}

	struct ChunkNode *node = &cache->data[index];
	struct Chunk *chunk = node->data;

	detach_node(cache, node);
	u64 next_index = (index + 1) & (cache->capacity - 1);
	assert(next_index >= 0 && next_index < cache->capacity);
	node->state = cache->data[next_index].state == HM_FREE ? HM_FREE : HM_TOMBSTONE;
	node->key = (struct ChunkKey){0};
	node->data = NULL;

	cache->size--;

	return chunk;
}

struct Chunk *chunkcache_add(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk *chunkin) {
	assert(cache != NULL);
	assert(chunkin != NULL);

	u64 existing_index = 0;
	if (get_node_index(cache, chunkX, chunkZ, &existing_index)) {
		return NULL; // Cache entry already exists.
	}

	assert(existing_index >= 0 && existing_index < cache->capacity);

	struct Chunk *evicted = NULL;
	if (cache->size + 1 > cache->capacity) {
		assert(cache->tail != NULL);
		evicted = chunkcache_remove(cache, cache->tail->key.x, cache->tail->key.z);
	}

	u64 preferred_index = hash_key(chunkX, chunkZ, cache->capacity);
	u64 index = preferred_index;

	struct ChunkNode *node = NULL;
	do {
		assert(index >= 0 && index < cache->capacity);

		node = &cache->data[index];
		if (node->state == HM_FREE || node->state == HM_TOMBSTONE) break;

		index++;
		index &= (cache->capacity - 1);
	} while (index != preferred_index);

	assert(node != NULL);

	node->state = HM_OCCUPIED;
	node->key = (struct ChunkKey){chunkX, chunkZ};
	node->data = chunkin;

	attach_head_node(cache, node);

	cache->size++;

	return evicted;
}

struct Chunk *chunkcache_remove_oldest(struct ChunkCache *cache) {
	assert(cache != NULL);

	if (cache->tail == NULL) {
		return NULL;
	}

	struct ChunkNode *node = cache->tail;
	return chunkcache_remove(cache, node->key.x, node->key.z);
}
