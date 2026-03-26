#ifndef CHUNK_CACHE
#define CHUNK_CACHE

#include <m4xdev/types.h>

#include "chunk.h"

struct ChunkCache;

err_t chunkcache_create(size_t capacity, size_t highwater, struct ChunkCache **cacheout);
err_t chunkcache_resize(struct ChunkCache *cache, size_t highwater);
void chunkcache_destroy(struct ChunkCache *cache);

bool chunkcache_drain(struct ChunkCache *cache);

struct Chunk *chunkcache_get(struct ChunkCache *cache, i64 chunkX, i64 chunkZ);
void chunkcache_add(struct ChunkCache *cache, i64 chunkX, i64 chunkZ, struct Chunk *chunkin);
struct Chunk *chunkcache_remove(struct ChunkCache *cache, i64 chunkX, i64 chunkZ);
struct Chunk *chunkcache_remove_oldest(struct ChunkCache *cache);

#endif
