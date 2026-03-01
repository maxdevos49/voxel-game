#include <stdlib.h>
#include <string.h>

#include <m4xdev/types.h>
#include <m4xdev/logging.h>
#include <m4xdev/performance.h>

#include "graphics/graphics.h"
#include "graphics/glerror.h"
#include "block-model.h"
#include "chunk.h"

#define TERRAIN_AMPLITUDE (1.0f / 20.0f)
#define TERRAIN_PEAK	  (100.0f)

i32 generate_chunk(i32 chunkX, i32 chunkZ, struct Chunk **out) {
	if (out == NULL) {
		return -1;
	}

	struct Chunk *chunk = malloc(sizeof(struct Chunk));
	if (chunk == NULL) {
		perror("malloc");
		return -2;
	}

	struct Perf p = {0};
	perf_start(&p);

	chunk->chunkX = chunkX;
	chunk->chunkZ = chunkZ;

	chunk->gl.vao = 0;
	chunk->gl.vbo = 0;
	chunk->gl.ebo = 0;

	// srand(time(NULL)); // Seed for predictability

	for (i32 x = 0; x < CHUNK_WIDTH; x++) {
		for (i32 z = 0; z < CHUNK_WIDTH; z++) {
			float noise = glm_perlin_vec2(
				(vec2){
					((((float)chunkX * (float)CHUNK_WIDTH) + (float)x + 0.5f) * TERRAIN_AMPLITUDE),
					((((float)chunkZ * (float)CHUNK_WIDTH) + (float)z + 0.5f) * TERRAIN_AMPLITUDE)});

			i32 elevation = (i32)((noise * 0.5f + 0.5f) * TERRAIN_PEAK);

			for (i32 y = 0; y < CHUNK_HEIGHT; y++) {
				u32 index = CHUNK_BLOCK_INDEX(x, y, z);
				if (y > elevation) {
					chunk->data[index] = AIR_BLOCK;
				} else if (y == elevation) {
					chunk->data[index] = GRASS_BLOCK;
				} else if (elevation - y < 6) {
					chunk->data[index] = DIRT_BLOCK;
				} else {
					chunk->data[index] = STONE_BLOCK;
				}

				// int type = rand() % BLOCK_COUNT;

				// u32 index = CHUNK_INDEX(x, y, z);
				// chunk->data[index] = type;
			}
		}
	}

	perf_report(&p, "Chunk Generation");

	*out = chunk;
	return 0;
}

i32 destroy_chunk(struct Chunk *chunk) {
	if (chunk == NULL) {
		return -1;
	}

	if (chunk->gl.vao != 0) {
		GL_CALL(glDeleteBuffers(1, &chunk->gl.ebo));
		chunk->gl.ebo = 0;

		GL_CALL(glDeleteBuffers(1, &chunk->gl.vbo));
		chunk->gl.vbo = 0;

		GL_CALL(glDeleteVertexArrays(1, &chunk->gl.vao));
		chunk->gl.vao = 0;
	}

	free(chunk);
	chunk = NULL;

	return 0;
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
				enum BlockType target = chunk->data[CHUNK_BLOCK_INDEX(x, y, z)];

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
					if (z == (CHUNK_WIDTH - 1) && south_chunk && south_chunk->data[CHUNK_BLOCK_INDEX(x, y, 0)] == AIR_BLOCK) {
						push_face(x, y, z, target, SOUTH_FACE, &chunk_mesh);
					}

					// Target Block: East Face - exposed to east chunk
					if (x == (CHUNK_WIDTH - 1) && east_chunk && east_chunk->data[CHUNK_BLOCK_INDEX(0, y, z)] == AIR_BLOCK) {
						push_face(x, y, z, target, EAST_FACE, &chunk_mesh);
					}

					// Target Block: North Face - exposed to north chunk
					if (z == 0 && north_chunk && north_chunk->data[CHUNK_BLOCK_INDEX(x, y, CHUNK_WIDTH - 1)] == AIR_BLOCK) {
						push_face(x, y, z, target, NORTH_FACE, &chunk_mesh);
					}

					// Target Block: West Face - exposed to west chunk
					if (x == 0 && west_chunk && west_chunk->data[CHUNK_BLOCK_INDEX(CHUNK_WIDTH - 1, y, z)] == AIR_BLOCK) {
						push_face(x, y, z, target, WEST_FACE, &chunk_mesh);
					}

					continue;
				}

				// Bottom Block: Top Face
				if (y - 1 >= 0) {
					enum BlockType bottom = chunk->data[CHUNK_BLOCK_INDEX(x, y - 1, z)];
					if (bottom != AIR_BLOCK) {
						push_face(x, y - 1, z, bottom, TOP_FACE, &chunk_mesh);
					}
				}

				// Top Block: Bottom Face
				if (y + 1 < CHUNK_HEIGHT) {
					enum BlockType top = chunk->data[CHUNK_BLOCK_INDEX(x, y + 1, z)];
					if (top != AIR_BLOCK) {
						push_face(x, y + 1, z, top, BOTTOM_FACE, &chunk_mesh);
					}
				}

				// North Block: South Face
				if (z - 1 >= 0) {
					enum BlockType north = chunk->data[CHUNK_BLOCK_INDEX(x, y, z - 1)];
					if (north != AIR_BLOCK) {
						push_face(x, y, z - 1, north, SOUTH_FACE, &chunk_mesh);
					}
				}

				// East Block: West Face
				if (x + 1 < CHUNK_WIDTH) {
					enum BlockType east = chunk->data[CHUNK_BLOCK_INDEX(x + 1, y, z)];
					if (east != AIR_BLOCK) {
						push_face(x + 1, y, z, east, WEST_FACE, &chunk_mesh);
					}
				}

				// South Block: North Face
				if (z + 1 < CHUNK_WIDTH) {
					enum BlockType south = chunk->data[CHUNK_BLOCK_INDEX(x, y, z + 1)];
					if (south != AIR_BLOCK) {
						push_face(x, y, z + 1, south, NORTH_FACE, &chunk_mesh);
					}
				}

				// West Block: East Face
				if (x - 1 >= 0) {
					enum BlockType west_block = chunk->data[CHUNK_BLOCK_INDEX(x - 1, y, z)];
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

	perf_report(&p, "Chunk Meshing");
	return 0;
}

i32 render_chunk(const struct Chunk *chunk) {
	if (chunk == NULL) {
		return -1;
	}

	if (chunk->gl.vao == 0) {
		return -1;
	}

	GL_CALL(glBindVertexArray(chunk->gl.vao));
	GL_CALL(glDrawElements(GL_TRIANGLES, chunk->gl.element_count, GL_UNSIGNED_INT, 0));
	GL_CALL(glBindVertexArray(0));

	return 0;
}
