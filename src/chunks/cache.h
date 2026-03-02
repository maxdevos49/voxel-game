#ifndef CHUNK_CACHE
#define CHUNK_CACHE

#include <m4xdev/types.h>

#include "chunk.h"

struct ChunkCache;

err_t chunkcache_create(size_t capacity, struct ChunkCache **cacheout);
void ChunkCache_destroy(struct ChunkCache *cache); //TODO implement

err_t chunkcache_get(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk **chunkout);
err_t chunkcache_add(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk *chunkin, struct Chunk **evictedout);
err_t chunkcache_remove(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk **chunkout);

#endif
