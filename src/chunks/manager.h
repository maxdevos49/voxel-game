#ifndef CHUNKS_MANAGER
#define CHUNKS_MANAGER

#include <m4xdev/types.h>

#include "chunk.h"
#include "world.h"

#define CHUNK_VIEW_AREA(radius) ((radius * 2 + 1) * (radius * 2 + 1))

struct ChunkManager;

err_t chunkmanager_create(u16 radius, struct ChunkManager **selfout);
void chunkmanager_destroy(struct ChunkManager *self);

struct Chunk *chunkmanager_get(const struct ChunkManager *self, struct ChunkPos pos);

err_t chunkmanager_update(struct ChunkManager *self, struct World *world, double blockX, double blockZ, bool load_all);
err_t chunkmanager_render(struct ChunkManager *self, double blockX, double blockZ);

#endif
