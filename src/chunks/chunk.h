#ifndef CHUNK_H
#define CHUNK_H

#include <assert.h>
#include <m4xdev/types.h>

#define CHUNK_WIDTH		 16
#define CHUNK_WIDTH_BITS 4
#define CHUNK_WIDTH_MASK (CHUNK_WIDTH - 1)
#define CHUNK_MIN_HEIGHT 0
#define CHUNK_MAX_HEIGHT 256

#define CHUNK_AREA	 (CHUNK_WIDTH * CHUNK_WIDTH)
#define CHUNK_VOLUME (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_MAX_HEIGHT)

#define CHUNK_DIRTY_FLAG	  (1 << 0) /* Chunk is not saved to disk */
#define CHUNK_MESH_FLAG		  (1 << 1) /* Chunk needs reprocessed or meshed */
#define CHUNK_RENDERABLE_FLAG (1 << 2) /* Chunk can be rendered */

struct ChunkPos {
	i32 x;
	i32 z;
};

static inline struct ChunkPos chunk_position_at(i32 worldX, i32 worldZ) {
	return (struct ChunkPos){
		.x = worldX >> CHUNK_WIDTH_BITS,
		.z = worldZ >> CHUNK_WIDTH_BITS};
}

struct BlockPos {
	i32 x;
	i32 y;
	i32 z;
};

static inline struct BlockPos chunk_block_position_at(i32 worldX, i32 worldY, i32 worldZ) {
	assert(worldY >= CHUNK_MIN_HEIGHT && worldY < CHUNK_MAX_HEIGHT);

	return (struct BlockPos){
		.x = worldX & CHUNK_WIDTH_MASK,
		.y = worldY,
		.z = worldZ & CHUNK_WIDTH_MASK};
}

struct Chunk;

err_t chunk_allocate(i32 chunkX, i32 chunkZ, struct Chunk **chunkout);
void chunk_destroy(struct Chunk *chunk);

void chunk_set_block(struct Chunk *chunk, struct BlockPos local, u16 block);
u16 chunk_get_block(const struct Chunk *chunk, struct BlockPos local);
i32 chunk_hightmap_at(const struct Chunk *chunk, struct BlockPos local);

void chunk_set_flag(struct Chunk *chunk, i32 flag);
void chunk_unset_flag(struct Chunk *chunk, i32 flag);
bool chunk_get_flag(const struct Chunk *chunk, i32 flag);

#endif
