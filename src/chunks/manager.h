#ifndef CHUNKS_MANAGER
#define CHUNKS_MANAGER

#include <m4xdev/types.h>

#define CHUNK_VIEW_AREA(radius) ((radius * 2 + 1) * (radius * 2 + 1))

struct ChunkCoord {
	i32 x;
	i32 z;
};

struct ChunkManager;

err_t chunkmanager_create(u16 radius, struct ChunkManager **managerout);
void chunkmanager_destroy(struct ChunkManager *manager);

err_t chunkmanager_update(struct ChunkManager *manager, double blockX, double blockZ);
err_t chunkmanager_render(struct ChunkManager *manager, double blockX, double blockZ);

// Ideas
// void chunkmanager_set_simulation_radius(struct ChunkManager *manager, u8 radius);
// u8 chunkmanager_get_simulation_radius(struct ChunkManager *manager);

// void chunkmanager_set_render_radius(struct ChunkManager *manager, u8 radius);
// u8 chunkmanager_get_render_radius(struct ChunkManager *manager);

#endif
