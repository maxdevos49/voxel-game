#ifndef CHUNK_H
#define CHUNK_H

#include <m4xdev/types.h>

// ===========================
// CHUNK CONSTANTS/TYPES
// ===========================

#define CHUNK_WIDTH	 16
#define CHUNK_HEIGHT 128

#define CHUNK_DIRTY_FLAG	  (1 << 0) /* Chunk is not saved to disk */
#define CHUNK_MESH_FLAG		  (1 << 1) /* Chunk needs reprocessed or meshed */
#define CHUNK_RENDERABLE_FLAG (1 << 2) /* Chunk can be rendered */

struct Chunk;

err_t chunk_generate(i32 chunkX, i32 chunkZ, struct Chunk **chunkout);
void chunk_destroy(struct Chunk *chunk);

void chunk_set_flag(struct Chunk *chunk, i32 flag);
void chunk_unset_flag(struct Chunk *chunk, i32 flag);
bool chunk_get_flag(struct Chunk *chunk, i32 flag);

// ===========================
// CHUNK RENDERING
// ===========================

i32 mesh_chunk(
	struct Chunk *chunk,
	struct Chunk *north_chunk,
	struct Chunk *east_chunk,
	struct Chunk *south_chunk,
	struct Chunk *west_chunk);
i32 render_chunk(struct Chunk *chunk);

// ===========================
// TERRAIN GENERATION
// ===========================

#define TERRAIN_AMPLITUDE (1.0f / 20.0f)
#define TERRAIN_PEAK	  (100.0f)

#endif
