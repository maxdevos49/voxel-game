#ifndef CHUNK_CACHE
#define CHUNK_CACHE

#include <m4xdev/types.h>

#include "chunk.h"

struct ChunkCache;

err_t chunkcache_create(size_t capacity, struct ChunkCache **cacheout);
void chunkcache_destroy(struct ChunkCache *cache);

struct Chunk *chunkcache_get(struct ChunkCache *cache, i32 chunkX, i32 chunkZ);
struct Chunk *chunkcache_add(struct ChunkCache *cache, i32 chunkX, i32 chunkZ, struct Chunk *chunkin);
struct Chunk *chunkcache_remove(struct ChunkCache *cache, i32 chunkX, i32 chunkZ);
struct Chunk *chunkcache_remove_oldest(struct ChunkCache *cache);

#endif
