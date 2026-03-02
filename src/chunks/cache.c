#include <stdio.h>
#include <string.h>

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

err_t chunkcache_get(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk **chunkout) {
	if (cache == NULL) return -1;
	if (chunkout == NULL) return -1;

	*chunkout = NULL;

	u64 index = 0;
	if (!get_node_index(cache, chunkX, chunkZ, &index)) {
		return 0;
	}

	struct ChunkNode *node = &cache->data[index];
	assert(node != NULL);

	detach_node(cache, node);
	attach_head_node(cache, node);

	*chunkout = node->data;

	return 0;
}

err_t chunkcache_remove(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk **chunkout) {
	if (cache == NULL) return -1;
	if (chunkout == NULL) return -1;

	*chunkout = NULL;

	u64 index = 0;
	if (!get_node_index(cache, chunkX, chunkZ, &index)) {
		return 0;
	}

	struct ChunkNode *node = &cache->data[index];
	assert(node != NULL);

	*chunkout = node->data;

	detach_node(cache, node);

	u64 next_index = (index + 1) & (cache->capacity - 1);
	node->state = cache->data[next_index].state == HM_FREE ? HM_FREE : HM_TOMBSTONE;

	node->key = (struct ChunkKey){0};
	node->data = NULL;

	cache->size--;

	return 0;
}

err_t chunkcache_add(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk *chunkin, struct Chunk **evictedout) {
	if (cache == NULL) return -1;
	if (chunkin == NULL) return -1;
	if (evictedout == NULL) return -1;

	*evictedout = NULL;

	u64 existing_index = 0;
	if (get_node_index(cache, chunkX, chunkZ, &existing_index)) {
		return -2; // Cache entry already exists.
	}

	if (cache->size + 1 > cache->capacity) {
		assert(cache->tail != NULL);
		chunkcache_remove(cache, cache->tail->key.x, cache->tail->key.z, evictedout);
	}

	u64 preferred_index = hash_key(chunkX, chunkZ, cache->capacity);
	u64 index = preferred_index;

	struct ChunkNode *node = NULL;
	do {
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

	return 0;
}
