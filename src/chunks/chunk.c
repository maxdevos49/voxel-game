#include <string.h>
#include <m4xdev/types.h>

#include "chunk.h"
#include "internal.h"

#include "../block-model.h"

//==============================================================================
// CHUNK SECTION FUNCTIONS (Private)
//==============================================================================

static err_t chunksection_allocate(struct ChunkSection **sectionout) {
	assert(sectionout != NULL);
	*sectionout = NULL;

	struct ChunkSection *section = malloc(sizeof(struct ChunkSection));
	if (section == NULL) {
		perror("malloc");
		return -1;
	}

	memset(section, AIR_BLOCK, sizeof(*section));

	*sectionout = section;
	section = NULL;

	return 0;
}

static void chunksection_deallocate(struct ChunkSection *section) {
	assert(section != NULL);

	free(section);
	section = NULL;
}

//==============================================================================
// CHUNK FUNCTIONS
//==============================================================================

static inline void chunk_update_heightmap(struct Chunk *chunk, struct BlockPos local, u16 block) {
	assert(chunk != NULL);
	u64 heightmap_index = (local.z << 4) | local.x;
	i64 current_height = chunk->heightmap[heightmap_index];
	if (local.y > current_height && block != AIR_BLOCK) {
		chunk->heightmap[heightmap_index] = local.y;
	} else if (local.y == current_height && block == AIR_BLOCK) {
		i64 y = local.y;
		while (--y > 0 && chunk_get_block(chunk, (struct BlockPos){local.x, y, local.z}) == AIR_BLOCK);
		chunk->heightmap[heightmap_index] = y;
	}
}

err_t chunk_allocate(i64 chunkX, i64 chunkZ, struct Chunk **chunkout) {
	assert(chunkout != NULL);
	*chunkout = NULL;

	struct Chunk *chunk = malloc(sizeof(struct Chunk));
	if (chunk == NULL) {
		perror("malloc");
		return -1;
	}

	memset(chunk, 0, sizeof(*chunk));

	chunk->x = chunkX;
	chunk->z = chunkZ;

	*chunkout = chunk;
	chunk = NULL;

	return 0;
}

void chunk_destroy(struct Chunk *chunk) {
	assert(chunk != NULL);
	assert(chunk->gl.vao == 0);

	for (int i = 0; i < CHUNK_SECTION_COUNT; i++) {
		struct ChunkSection *section = chunk->sections[i];
		if (chunk->sections[i] != NULL) {
			chunksection_deallocate(section);
			chunk->sections[i] = NULL;
		}
	}

	chunk = NULL;
}

void chunk_set_block(struct Chunk *chunk, struct BlockPos local, u16 block) {
	assert(chunk != NULL);

	struct ChunkSection *section = chunk->sections[local.y >> CHUNK_SECTION_BITS];
	if (section == NULL && block == AIR_BLOCK) {
		return;
	} else if (section == NULL) {
		if (chunksection_allocate(&section) != 0) {
			return;
		}

		chunk->sections[local.y >> CHUNK_SECTION_BITS] = section;
	}

	i64 block_index = local.z << 8 | local.x << 4 | (local.y & CHUNK_SECTION_MASK);

	u16 old_block = section->data[block_index];
	if (old_block != block) {
		section->data[block_index] = block;
		chunk_update_heightmap(chunk, local, block);
		chunk_set_flag(chunk, (CHUNK_DIRTY_FLAG | CHUNK_MESH_FLAG));
	}
}

u16 chunk_get_block(const struct Chunk *chunk, struct BlockPos local) {
	assert(chunk != NULL);

	struct ChunkSection *section = chunk->sections[local.y >> CHUNK_SECTION_BITS];
	if (section == NULL) return AIR_BLOCK;

	return section->data[local.z << 8 | local.x << 4 | (local.y & CHUNK_SECTION_MASK)];
}

i64 chunk_hightmap_at(const struct Chunk *chunk, struct BlockPos local) {
	assert(chunk != NULL);

	return chunk->heightmap[(local.z << 4) | local.x];
}

bool chunk_get_flag(const struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	return chunk->flags & flag;
}

void chunk_set_flag(struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	chunk->flags |= flag;
}

void chunk_unset_flag(struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	chunk->flags &= ~flag;
}
