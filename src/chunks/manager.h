#ifndef CHUNKS_MANAGER
#define CHUNKS_MANAGER

#include <m4xdev/types.h>

#include "chunk.h"

#define CHUNK_VIEW_AREA(radius) ((radius * 2 + 1) * (radius * 2 + 1))

struct ChunkManager;

err_t chunkmanager_create(u16 radius, struct ChunkManager **selfout);
void chunkmanager_destroy(struct ChunkManager *self);

void chunkmanager_view_radius_set(struct ChunkManager *self, u64 radius);

struct Chunk *chunkmanager_get(const struct ChunkManager *self, struct ChunkPos pos);

err_t chunkmanager_preload(struct ChunkManager *self, double blockX, double blockZ);
err_t chunkmanager_update(struct ChunkManager *self, double blockX, double blockZ);
err_t chunkmanager_render(struct ChunkManager *self, double blockX, double blockZ);

#endif
