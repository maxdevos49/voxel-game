#include <stdlib.h>
#include <string.h>

#include <m4xdev/types.h>
#include <m4xdev/logging.h>
#include <m4xdev/performance.h>

#include "../graphics/graphics.h"
#include "../graphics/glerror.h"
#include "../block-model.h"

#include "chunk.h"

#define CHUNK_AREA	 (CHUNK_WIDTH * CHUNK_WIDTH)
#define CHUNK_VOLUME (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT)

#define CHUNK_SECTION_COUNT	 (CHUNK_HEIGHT / CHUNK_WIDTH)
#define CHUNK_SECTION_VOLUME (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH)

struct ChunkSection {
	u8 data[CHUNK_SECTION_VOLUME];
};

struct Chunk {
	i32 flags;
	i32 x, z;

	struct ChunkSection *sections[CHUNK_SECTION_COUNT];

	u8 heightmap[CHUNK_AREA];

	struct {
		size_t element_count;
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
	} gl;
};

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

static err_t chunk_allocate(struct Chunk **chunkout) {
	assert(chunkout != NULL);
	*chunkout = NULL;

	struct Chunk *chunk = malloc(sizeof(struct Chunk));
	if (chunk == NULL) {
		perror("malloc");
		return -1;
	}

	memset(chunk, 0, sizeof(*chunk));

	*chunkout = chunk;
	chunk = NULL;

	return 0;
}

static void chunksection_deallocate(struct ChunkSection *section) {
	assert(section != NULL);

	free(section);
	section = NULL;
}

static void chunk_deallocate(struct Chunk *chunk) {
	assert(chunk != NULL);

	for (int i = 0; i < CHUNK_SECTION_COUNT; i++) {
		struct ChunkSection *section = chunk->sections[i];
		if (chunk->sections[i] != NULL) {
			chunksection_deallocate(section);
			chunk->sections[i] = NULL;
		}
	}

	free(chunk);
	chunk = NULL;
}

static inline u64 chunksection_index(i32 blockY) {
	assert(blockY >= 0 && blockY < CHUNK_HEIGHT);

	return (u64)blockY >> 4;
}

static inline u64 chunksection_block_index(i32 blockX, i32 blockY, i32 blockZ) {
	assert(blockX >= 0 && blockX < CHUNK_WIDTH);
	assert(blockY >= 0 && blockY < CHUNK_HEIGHT);
	assert(blockZ >= 0 && blockZ < CHUNK_WIDTH);

	i32 sectionY = blockY & 15;
	return ((u64)sectionY << 8) | ((u64)blockZ << 4) | (u64)blockX;
}

static inline void chunk_update_heightmap(struct Chunk *chunk, i32 blockX, i32 blockY, i32 blockZ) {
	assert(chunk != NULL);
	assert(blockX >= 0 && blockX < CHUNK_WIDTH);
	assert(blockY >= 0 && blockY < CHUNK_HEIGHT);
	assert(blockZ >= 0 && blockZ < CHUNK_WIDTH);

	u64 heightmap_index = ((u64)blockZ << 4) | (u64)blockX;

	if (chunk->heightmap[heightmap_index] < blockY) {
		chunk->heightmap[heightmap_index] = blockY;
	}
}

static err_t chunk_put_block(struct Chunk *chunk, i32 blockX, i32 blockY, i32 blockZ, u8 block) {
	assert(chunk != NULL);
	assert(blockX >= 0 && blockX < CHUNK_WIDTH);
	assert(blockY >= 0 && blockY < CHUNK_HEIGHT);
	assert(blockZ >= 0 && blockZ < CHUNK_HEIGHT);

	u64 section_index = chunksection_index(blockY);
	struct ChunkSection *section = chunk->sections[section_index];
	if (section == NULL && block == AIR_BLOCK) {
		return 0;
	} else if (section == NULL) {
		if (chunksection_allocate(&section) != 0) {
			return -1;
		}

		chunk->sections[section_index] = section;
	}
	assert(chunk->sections[section_index] != NULL);

	u64 block_index = chunksection_block_index(blockX, blockY & 15, blockZ);

	u8 old_block = section->data[block_index];
	if (old_block != block) {
		section->data[block_index] = block;
		chunk_update_heightmap(chunk, blockX, blockY, blockZ);
		chunk_set_flag(chunk, (CHUNK_DIRTY_FLAG | CHUNK_MESH_FLAG));
	}

	return 0;
}

static u8 chunk_get_block(struct Chunk *chunk, i32 blockX, i32 blockY, i32 blockZ) {
	assert(chunk != NULL);
	assert(blockX >= 0 && blockX < CHUNK_WIDTH);
	assert(blockY >= 0 && blockY < CHUNK_HEIGHT);
	assert(blockZ >= 0 && blockZ < CHUNK_HEIGHT);

	u64 section_index = chunksection_index(blockY);
	struct ChunkSection *section = chunk->sections[section_index];
	if (section == NULL) return AIR_BLOCK;

	u64 block_index = chunksection_block_index(blockX, blockY, blockZ);
	return section->data[block_index];
}

err_t chunk_generate(i32 chunkX, i32 chunkZ, struct Chunk **chunkout) {
	assert(chunkout != NULL);
	*chunkout = NULL;

	struct Perf p = {0};
	perf_start(&p);

	struct Chunk *chunk = NULL;
	if (chunk_allocate(&chunk) != 0) {
		return -1;
	}

	for (i32 x = 0; x < CHUNK_WIDTH; x++) {
		for (i32 z = 0; z < CHUNK_WIDTH; z++) {
			float noise = glm_perlin_vec2(
				(vec2){
					((((float)chunkX * (float)CHUNK_WIDTH) + (float)x + 0.5f) * TERRAIN_AMPLITUDE),
					((((float)chunkZ * (float)CHUNK_WIDTH) + (float)z + 0.5f) * TERRAIN_AMPLITUDE)});

			i32 elevation = (i32)((noise * 0.5f + 0.5f) * TERRAIN_PEAK);

			for (i32 y = 0; y < CHUNK_HEIGHT; y++) {
				if (y > elevation) {
					chunk_put_block(chunk, x, y, z, AIR_BLOCK);
				} else if (y == elevation) {
					chunk_put_block(chunk, x, y, z, GRASS_BLOCK);
				} else if (elevation - y < 6) {
					chunk_put_block(chunk, x, y, z, DIRT_BLOCK);
				} else {
					chunk_put_block(chunk, x, y, z, STONE_BLOCK);
				}
			}
		}
	}

	*chunkout = chunk;
	chunk = NULL;

	perf_report(&p, "Chunk Generation");
	return 0;
}

void chunk_destroy(struct Chunk *chunk) {
	assert(chunk != NULL);

	if (chunk->gl.vao != 0) {
		GL_CALL(glDeleteBuffers(1, &chunk->gl.ebo));
		chunk->gl.ebo = 0;

		GL_CALL(glDeleteBuffers(1, &chunk->gl.vbo));
		chunk->gl.vbo = 0;

		GL_CALL(glDeleteVertexArrays(1, &chunk->gl.vao));
		chunk->gl.vao = 0;
	}

	chunk_deallocate(chunk);
	chunk = NULL;
}

void chunk_set_flag(struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	chunk->flags |= flag;
}

void chunk_unset_flag(struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	chunk->flags &= ~flag;
}

bool chunk_get_flag(struct Chunk *chunk, i32 flag) {
	assert(chunk != NULL);

	return chunk->flags & flag;
}

// Worst case face exposure in a chunk is a alternating checkerboard pattern
// which is half the volume with 6 faces per block visible
#define VERTEX_CAPACITY ((CHUNK_VOLUME / 2) * 6 * 4)
#define INDEX_CAPACITY	((CHUNK_VOLUME / 2) * 6 * 6)

static struct ChunkMesh {
	struct {
		struct Vertex data[VERTEX_CAPACITY];
		size_t length;
	} vertex_array;

	struct {
		u32 data[INDEX_CAPACITY];
		size_t length;
	} index_array;
} chunk_mesh = {0};

static void push_face(float x, float y, float z, enum BlockType block, enum Face face, struct ChunkMesh *mesh) {
	struct BlockModel model = BlockModels[block];

	struct Quad quad = model.computed[face];

	size_t vertex_offset = mesh->vertex_array.length;
	mesh->vertex_array.data[vertex_offset] = (struct Vertex){
		quad.v0.x + x,
		quad.v0.y + y,
		quad.v0.z + z,
		quad.v0.u,
		quad.v0.v};

	mesh->vertex_array.data[vertex_offset + 1] = (struct Vertex){
		quad.v1.x + x,
		quad.v1.y + y,
		quad.v1.z + z,
		quad.v1.u,
		quad.v1.v};

	mesh->vertex_array.data[vertex_offset + 2] = (struct Vertex){
		quad.v2.x + x,
		quad.v2.y + y,
		quad.v2.z + z,
		quad.v2.u,
		quad.v2.v};

	mesh->vertex_array.data[vertex_offset + 3] = (struct Vertex){
		quad.v3.x + x,
		quad.v3.y + y,
		quad.v3.z + z,
		quad.v3.u,
		quad.v3.v};

	mesh->vertex_array.length += 4;

	u32 index_offset = mesh->index_array.length;

	mesh->index_array.data[index_offset + 0] = vertex_offset + 0;
	mesh->index_array.data[index_offset + 1] = vertex_offset + 1;
	mesh->index_array.data[index_offset + 2] = vertex_offset + 2;
	mesh->index_array.data[index_offset + 3] = vertex_offset + 2;
	mesh->index_array.data[index_offset + 4] = vertex_offset + 3;
	mesh->index_array.data[index_offset + 5] = vertex_offset + 0;

	mesh->index_array.length += 6;
}

i32 mesh_chunk(
	struct Chunk *chunk,
	struct Chunk *north_chunk,
	struct Chunk *east_chunk,
	struct Chunk *south_chunk,
	struct Chunk *west_chunk) {
	if (chunk == NULL) {
		return -1;
	}

	struct Perf p = {0};
	perf_start(&p);

	chunk_mesh.vertex_array.length = 0;
	chunk_mesh.index_array.length = 0;

	for (i32 x = 0; x < CHUNK_WIDTH; x++) {
		for (i32 z = 0; z < CHUNK_WIDTH; z++) {
			for (i32 y = 0; y < CHUNK_HEIGHT; y++) {
				enum BlockType target = chunk_get_block(chunk, x, y, z);

				if (target != AIR_BLOCK) {
					// Target Block: Bottom Face - Bottom of world
					if (y == 0) {
						push_face(x, y, z, target, BOTTOM_FACE, &chunk_mesh);
					}

					// Target Block: Top Face - Top of world
					if (y == (CHUNK_HEIGHT - 1)) {
						push_face(x, y, z, target, TOP_FACE, &chunk_mesh);
					}

					// Target Block: South Face - exposed to south chunk
					if (z == (CHUNK_WIDTH - 1) && south_chunk && chunk_get_block(south_chunk, x, y, 0) == AIR_BLOCK) {
						push_face(x, y, z, target, SOUTH_FACE, &chunk_mesh);
					}

					// Target Block: East Face - exposed to east chunk
					if (x == (CHUNK_WIDTH - 1) && east_chunk && chunk_get_block(east_chunk, 0, y, z) == AIR_BLOCK) {
						push_face(x, y, z, target, EAST_FACE, &chunk_mesh);
					}

					// Target Block: North Face - exposed to north chunk
					if (z == 0 && north_chunk && chunk_get_block(north_chunk, x, y, CHUNK_WIDTH - 1) == AIR_BLOCK) {
						push_face(x, y, z, target, NORTH_FACE, &chunk_mesh);
					}

					// Target Block: West Face - exposed to west chunk
					if (x == 0 && west_chunk && chunk_get_block(west_chunk, CHUNK_WIDTH - 1, y, z) == AIR_BLOCK) {
						push_face(x, y, z, target, WEST_FACE, &chunk_mesh);
					}

					continue;
				}

				// Bottom Block: Top Face
				if (y - 1 >= 0) {
					enum BlockType bottom = chunk_get_block(chunk, x, y - 1, z);
					if (bottom != AIR_BLOCK) {
						push_face(x, y - 1, z, bottom, TOP_FACE, &chunk_mesh);
					}
				}

				// Top Block: Bottom Face
				if (y + 1 < CHUNK_HEIGHT) {
					enum BlockType top = chunk_get_block(chunk, x, y + 1, z);
					if (top != AIR_BLOCK) {
						push_face(x, y + 1, z, top, BOTTOM_FACE, &chunk_mesh);
					}
				}

				// North Block: South Face
				if (z - 1 >= 0) {
					enum BlockType north = chunk_get_block(chunk, x, y, z - 1);
					if (north != AIR_BLOCK) {
						push_face(x, y, z - 1, north, SOUTH_FACE, &chunk_mesh);
					}
				}

				// East Block: West Face
				if (x + 1 < CHUNK_WIDTH) {
					enum BlockType east = chunk_get_block(chunk, x + 1, y, z);
					if (east != AIR_BLOCK) {
						push_face(x + 1, y, z, east, WEST_FACE, &chunk_mesh);
					}
				}

				// South Block: North Face
				if (z + 1 < CHUNK_WIDTH) {
					enum BlockType south = chunk_get_block(chunk, x, y, z + 1);
					if (south != AIR_BLOCK) {
						push_face(x, y, z + 1, south, NORTH_FACE, &chunk_mesh);
					}
				}

				// West Block: East Face
				if (x - 1 >= 0) {
					enum BlockType west_block = chunk_get_block(chunk, x - 1, y, z);
					if (west_block != AIR_BLOCK) {
						push_face(x - 1, y, z, west_block, EAST_FACE, &chunk_mesh);
					}
				}
			}
		}
	}

	chunk->gl.element_count = chunk_mesh.index_array.length;

	if (chunk->gl.vao == 0) {
		GL_CALL(glGenVertexArrays(1, &chunk->gl.vao));
		GL_CALL(glBindVertexArray(chunk->gl.vao));

		GL_CALL(glGenBuffers(1, &chunk->gl.vbo));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, chunk->gl.vbo));

		size_t stride = 5 * sizeof(float);

		// Vertex
		GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)0));
		GL_CALL(glEnableVertexAttribArray(0));

		// Texture
		GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(3 * sizeof(float))));
		GL_CALL(glEnableVertexAttribArray(1));

		GL_CALL(glGenBuffers(1, &chunk->gl.ebo));
	}

	GL_CALL(glBindVertexArray(chunk->gl.vao));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, chunk->gl.vbo));
	GL_CALL(glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(struct Vertex) * chunk_mesh.vertex_array.length,
		chunk_mesh.vertex_array.data,
		GL_STATIC_DRAW));

	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->gl.ebo));
	GL_CALL(glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(u32) * chunk_mesh.index_array.length,
		chunk_mesh.index_array.data,
		GL_STATIC_DRAW));

	GL_CALL(glBindVertexArray(0));

	chunk_unset_flag(chunk, CHUNK_MESH_FLAG);
	chunk_set_flag(chunk, CHUNK_RENDERABLE_FLAG);

	perf_report(&p, "Chunk Meshing");
	return 0;
}

i32 render_chunk(struct Chunk *chunk) {
	assert(chunk != NULL);

	if (!chunk_get_flag(chunk, CHUNK_RENDERABLE_FLAG)) {
		return -1;
	}

	GL_CALL(glBindVertexArray(chunk->gl.vao));
	GL_CALL(glDrawElements(GL_TRIANGLES, chunk->gl.element_count, GL_UNSIGNED_INT, 0));
	GL_CALL(glBindVertexArray(0));

	return 0;
}
