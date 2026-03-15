#include <stdio.h>

#include "chunk.h"
#include "manager.h"
#include "../block-model.h"

#include "world.h"

struct World {
	struct ChunkManager *chunk_manager;
};

err_t world_create(u16 radius, struct World **selfout) {
	assert(selfout != NULL);

	struct ChunkManager *chunk_manager = NULL;
	if (chunkmanager_create(radius, &chunk_manager) != 0) {
		return -1;
	}

	struct World *world = malloc(sizeof(struct World));
	if (world == NULL) {
		perror("malloc");

		chunkmanager_destroy(chunk_manager);
		return -1;
	}

	world->chunk_manager = chunk_manager;

	*selfout = world;

	return 0;
}

void world_destroy(struct World *self) {
	assert(self != NULL);

	chunkmanager_destroy(self->chunk_manager);

	free(self);
	self = NULL;
}

u16 world_get_block(const struct World *self, i32 worldX, i32 worldY, i32 worldZ) {
	assert(self != NULL);

	if (worldY < CHUNK_MIN_HEIGHT || worldY >= CHUNK_MAX_HEIGHT) {
		return AIR_BLOCK;
	}

	struct ChunkPos pos = chunk_position_at(worldX, worldZ);
	struct Chunk *chunk = chunkmanager_get(self->chunk_manager, pos);
	if (chunk == NULL) {
		return AIR_BLOCK;
	}

	struct BlockPos local = chunk_block_position_at(worldX, worldY, worldZ);
	return chunk_get_block(chunk, local);
}

err_t world_set_block(const struct World *world, i32 worldX, i32 worldY, i32 worldZ, u16 block) {
	assert(world != NULL);

	if (worldY < CHUNK_MIN_HEIGHT) {
		return SET_BLOCK_TO_LOW;
	} else if (worldY >= CHUNK_MAX_HEIGHT) {
		return SET_BLOCK_TO_HIGH;
	}

	struct ChunkPos pos = chunk_position_at(worldX, worldZ);
	struct Chunk *chunk = chunkmanager_get(world->chunk_manager, pos);
	if (chunk == NULL) {
		return SET_BLOCK_UNLOADED;
	}

	struct BlockPos local = chunk_block_position_at(worldX, worldY, worldZ);

	chunk_set_block(chunk, local, block);

	if (local.z == 0) {
		struct Chunk *north_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x, pos.z - 1});
		if (north_neighbor != NULL) chunk_set_flag(north_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.z == 0 && local.x == CHUNK_WIDTH - 1) {
		struct Chunk *north_east_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x + 1, pos.z - 1});
		if (north_east_neighbor != NULL) chunk_set_flag(north_east_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.x == CHUNK_WIDTH - 1) {
		struct Chunk *east_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x + 1, pos.z});
		if (east_neighbor != NULL) chunk_set_flag(east_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.z == CHUNK_WIDTH - 1 && local.x == CHUNK_WIDTH - 1) {
		struct Chunk *south_east_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x + 1, pos.z + 1});
		if (south_east_neighbor != NULL) chunk_set_flag(south_east_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.z == CHUNK_WIDTH - 1) {
		struct Chunk *south_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x, pos.z + 1});
		if (south_neighbor != NULL) chunk_set_flag(south_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.z == CHUNK_WIDTH - 1 && local.x == 0) {
		struct Chunk *south_west_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x - 1, pos.z + 1});
		if (south_west_neighbor != NULL) chunk_set_flag(south_west_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.x == 0) {
		struct Chunk *west_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x - 1, pos.z});
		if (west_neighbor != NULL) chunk_set_flag(west_neighbor, CHUNK_MESH_FLAG);
	}

	if (local.z == 0 && local.x == 0) {
		struct Chunk *north_west_neighbor = chunkmanager_get(world->chunk_manager, (struct ChunkPos){pos.x - 1, pos.z - 1});
		if (north_west_neighbor != NULL) chunk_set_flag(north_west_neighbor, CHUNK_MESH_FLAG);
	}

	return SET_BLOCK_OK;
}

void world_preload(struct World *self, double blockX, double blockZ) {
	assert(self != NULL);

	chunkmanager_update(self->chunk_manager, self, blockX, blockZ, true);
	chunkmanager_update(self->chunk_manager, self, blockX, blockZ, true); // twice for complete meshing
}

void world_update(struct World *self, double blockX, double blockZ) {
	assert(self != NULL);

	chunkmanager_update(self->chunk_manager, self, blockX, blockZ, false);
}

void world_render(struct World *self, double blockX, double blockZ) {
	assert(self != NULL);

	chunkmanager_render(self->chunk_manager, blockX, blockZ);
}
