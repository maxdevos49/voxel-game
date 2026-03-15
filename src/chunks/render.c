#include <m4xdev/types.h>
#include <m4xdev/performance.h>
#include <string.h>

#include "chunk.h"
#include "internal.h"
#include "render.h"

#include "../graphics/shader.h"
#include "../block-model.h"
#include "../graphics/glerror.h"

#define PROFILE_PERFORMANCE false

#define AO_DEFAULT 3

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

static inline u8 vertex_ao(bool side1, bool side2, bool corner) {
	if (side1 && side2) {
		return 0;
	}

	return 3 - (side1 + side2 + corner);
}

static inline bool vertex_ao_flip(u8 v0, u8 v1, u8 v2, u8 v3) {
	return v0 + v3 > v1 + v2;
}

static void push_face(struct ChunkMesh *mesh, struct BlockPos local, enum BlockType block, enum Face face, u8 aoV0, u8 aoV1, u8 aoV2, u8 aoV3) {
	assert(mesh != NULL);

	struct BlockModel model = BlockModels[block];

	struct Quad quad = model.computed[face];

	size_t vertex_offset = mesh->vertex_array.length;
	mesh->vertex_array.data[vertex_offset] = (struct Vertex){
		quad.v0.x + local.x,
		quad.v0.y + local.y,
		quad.v0.z + local.z,
		quad.v0.data | aoV0,
	};

	mesh->vertex_array.data[vertex_offset + 1] = (struct Vertex){
		quad.v1.x + local.x,
		quad.v1.y + local.y,
		quad.v1.z + local.z,
		quad.v1.data | aoV1,
	};

	mesh->vertex_array.data[vertex_offset + 2] = (struct Vertex){
		quad.v2.x + local.x,
		quad.v2.y + local.y,
		quad.v2.z + local.z,
		quad.v2.data | aoV2,
	};

	mesh->vertex_array.data[vertex_offset + 3] = (struct Vertex){
		quad.v3.x + local.x,
		quad.v3.y + local.y,
		quad.v3.z + local.z,
		quad.v3.data | aoV3,
	};

	mesh->vertex_array.length += 4;

	u32 index_offset = mesh->index_array.length;

	if (vertex_ao_flip(aoV0, aoV1, aoV2, aoV3)) {
		mesh->index_array.data[index_offset + 0] = vertex_offset + 0;
		mesh->index_array.data[index_offset + 1] = vertex_offset + 1;
		mesh->index_array.data[index_offset + 2] = vertex_offset + 3;
		mesh->index_array.data[index_offset + 3] = vertex_offset + 1;
		mesh->index_array.data[index_offset + 4] = vertex_offset + 2;
		mesh->index_array.data[index_offset + 5] = vertex_offset + 3;
	} else {
		mesh->index_array.data[index_offset + 0] = vertex_offset + 0;
		mesh->index_array.data[index_offset + 1] = vertex_offset + 1;
		mesh->index_array.data[index_offset + 2] = vertex_offset + 2;
		mesh->index_array.data[index_offset + 3] = vertex_offset + 2;
		mesh->index_array.data[index_offset + 4] = vertex_offset + 3;
		mesh->index_array.data[index_offset + 5] = vertex_offset + 0;
	}

	mesh->index_array.length += 6;
}

typedef const struct Chunk *chunkneighbors_t[3][3]; //zx

static inline u16 chunkneighbors_get_block(chunkneighbors_t neighbors, i32 localX, i32 localY, i32 localZ) {
	i32 chunkX = (localX >> CHUNK_WIDTH_BITS) + 1;
	i32 chunkZ = (localZ >> CHUNK_WIDTH_BITS) + 1;

	const struct Chunk *chunk = neighbors[chunkZ][chunkX];
	if (chunk == NULL || localY < CHUNK_MIN_HEIGHT || localY >= CHUNK_MAX_HEIGHT) return AIR_BLOCK;

	i32 x = localX & CHUNK_WIDTH_MASK;
	i32 y = localY;
	i32 z = localZ & CHUNK_WIDTH_MASK;

	return chunk_get_block(chunk, (struct BlockPos){x, y, z});
}

typedef u16 blockneighbors_t[3][3][3]; //zxy

static inline u16 blockneighbors_get_block(blockneighbors_t neighbors, i32 offsetX, i32 offsetY, i32 offsetZ) {
	assert(offsetX >= -1 && offsetX <= 1);
	assert(offsetY >= -1 && offsetY <= 1);
	assert(offsetZ >= -1 && offsetZ <= 1);

	return neighbors[offsetZ + 1][offsetX + 1][offsetY + 1];
}

void chunkrenderer_mesh(
	struct Chunk *chunk,
	const struct Chunk *north_chunk,
	const struct Chunk *north_east_chunk,
	const struct Chunk *east_chunk,
	const struct Chunk *south_east_chunk,
	const struct Chunk *south_chunk,
	const struct Chunk *south_west_chunk,
	const struct Chunk *west_chunk,
	const struct Chunk *north_west_chunk) {
	assert(chunk != NULL);

	chunkneighbors_t chunks;
	chunks[0][0] = north_west_chunk;
	chunks[0][1] = north_chunk;
	chunks[0][2] = north_east_chunk;

	chunks[1][0] = west_chunk;
	chunks[1][1] = chunk;
	chunks[1][2] = east_chunk;

	chunks[2][0] = south_west_chunk;
	chunks[2][1] = south_chunk;
	chunks[2][2] = south_east_chunk;

#if PROFILE_PERFORMANCE
	struct Perf p = {0};
	perf_start(&p);
#endif

	chunk_mesh.vertex_array.length = 0;
	chunk_mesh.index_array.length = 0;

	blockneighbors_t blocks = {0};

	for (i32 x = 0; x < CHUNK_WIDTH; x++) {
		for (i32 z = 0; z < CHUNK_WIDTH; z++) {
			i32 highest_block = chunk_hightmap_at(chunk, (struct BlockPos){x, 0, z});

			for (i32 y = 0; y <= highest_block; y++) {
				struct BlockPos local = {x, y, z};

				u16 block = chunkneighbors_get_block(chunks, x, y, z);
				if (block == AIR_BLOCK) continue;

				// West Slice blocks
				blocks[0][0][0] = chunkneighbors_get_block(chunks, x - 1, y - 1, z - 1);
				blocks[0][0][1] = chunkneighbors_get_block(chunks, x - 1, y, z - 1);
				blocks[0][0][2] = chunkneighbors_get_block(chunks, x - 1, y + 1, z - 1);

				blocks[0][1][0] = chunkneighbors_get_block(chunks, x, y - 1, z - 1);
				blocks[0][1][1] = chunkneighbors_get_block(chunks, x, y, z - 1);
				blocks[0][1][2] = chunkneighbors_get_block(chunks, x, y + 1, z - 1);

				blocks[0][2][0] = chunkneighbors_get_block(chunks, x + 1, y - 1, z - 1);
				blocks[0][2][1] = chunkneighbors_get_block(chunks, x + 1, y, z - 1);
				blocks[0][2][2] = chunkneighbors_get_block(chunks, x + 1, y + 1, z - 1);

				// Center slice blocks
				blocks[1][0][0] = chunkneighbors_get_block(chunks, x - 1, y - 1, z);
				blocks[1][0][1] = chunkneighbors_get_block(chunks, x - 1, y, z);
				blocks[1][0][2] = chunkneighbors_get_block(chunks, x - 1, y + 1, z);

				blocks[1][1][0] = chunkneighbors_get_block(chunks, x, y - 1, z);
				blocks[1][1][1] = block;
				blocks[1][1][2] = chunkneighbors_get_block(chunks, x, y + 1, z);

				blocks[1][2][0] = chunkneighbors_get_block(chunks, x + 1, y - 1, z);
				blocks[1][2][1] = chunkneighbors_get_block(chunks, x + 1, y, z);
				blocks[1][2][2] = chunkneighbors_get_block(chunks, x + 1, y + 1, z);

				// East slice blocks
				blocks[2][0][0] = chunkneighbors_get_block(chunks, x - 1, y - 1, z + 1);
				blocks[2][0][1] = chunkneighbors_get_block(chunks, x - 1, y, z + 1);
				blocks[2][0][2] = chunkneighbors_get_block(chunks, x - 1, y + 1, z + 1);

				blocks[2][1][0] = chunkneighbors_get_block(chunks, x, y - 1, z + 1);
				blocks[2][1][1] = chunkneighbors_get_block(chunks, x, y, z + 1);
				blocks[2][1][2] = chunkneighbors_get_block(chunks, x, y + 1, z + 1);

				blocks[2][2][0] = chunkneighbors_get_block(chunks, x + 1, y - 1, z + 1);
				blocks[2][2][1] = chunkneighbors_get_block(chunks, x + 1, y, z + 1);
				blocks[2][2][2] = chunkneighbors_get_block(chunks, x + 1, y + 1, z + 1);

				// Top face
				u16 top_block = blockneighbors_get_block(blocks, 0, 1, 0);
				if (top_block == AIR_BLOCK) {
					bool north = blockneighbors_get_block(blocks, 0, 1, -1) != AIR_BLOCK;
					bool north_east = blockneighbors_get_block(blocks, 1, 1, -1) != AIR_BLOCK;
					bool east = blockneighbors_get_block(blocks, 1, 1, 0) != AIR_BLOCK;
					bool south_east = blockneighbors_get_block(blocks, 1, 1, 1) != AIR_BLOCK;
					bool south = blockneighbors_get_block(blocks, 0, 1, 1) != AIR_BLOCK;
					bool south_west = blockneighbors_get_block(blocks, -1, 1, 1) != AIR_BLOCK;
					bool west = blockneighbors_get_block(blocks, -1, 1, 0) != AIR_BLOCK;
					bool north_west = blockneighbors_get_block(blocks, -1, 1, -1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(south, west, south_west);
					u8 aoV1 = vertex_ao(east, south, south_east);
					u8 aoV2 = vertex_ao(north, east, north_east);
					u8 aoV3 = vertex_ao(west, north, north_west);

					push_face(&chunk_mesh, local, block, TOP_FACE, aoV0, aoV1, aoV2, aoV3);
				}

						// Top face
				u16 bottom_block = blockneighbors_get_block(blocks, 0, -1, 0);
				if (bottom_block == AIR_BLOCK) {
					bool north = 		blockneighbors_get_block(blocks, 0, -1, -1) != AIR_BLOCK;
					bool north_east = 	blockneighbors_get_block(blocks, 1, -1, -1) != AIR_BLOCK;
					bool east = 		blockneighbors_get_block(blocks, 1, -1, 0) != AIR_BLOCK;
					bool south_east = 	blockneighbors_get_block(blocks, 1, -1, 1) != AIR_BLOCK;
					bool south = 		blockneighbors_get_block(blocks, 0, -1, 1) != AIR_BLOCK;
					bool south_west = 	blockneighbors_get_block(blocks, -1, -1, 1) != AIR_BLOCK;
					bool west = 		blockneighbors_get_block(blocks, -1, -1, 0) != AIR_BLOCK;
					bool north_west = 	blockneighbors_get_block(blocks, -1, -1, -1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(north, west, north_west);
					u8 aoV1 = vertex_ao(north,east, north_east);
					u8 aoV2 = vertex_ao(south, east, south_east);
					u8 aoV3 = vertex_ao(south, west, south_west);

					push_face(&chunk_mesh, local, block, BOTTOM_FACE, aoV0, aoV1, aoV2, aoV3);
				}

				// North Face
				u16 north_block = blockneighbors_get_block(blocks, 0, 0, -1);
				if (north_block == AIR_BLOCK) {
					bool top = blockneighbors_get_block(blocks, 0, 1, -1) != AIR_BLOCK;
					bool top_east = blockneighbors_get_block(blocks, 1, 1, -1) != AIR_BLOCK;
					bool east = blockneighbors_get_block(blocks, 1, 0, -1) != AIR_BLOCK;
					bool bottom_east = blockneighbors_get_block(blocks, 1, -1, -1) != AIR_BLOCK;
					bool bottom = blockneighbors_get_block(blocks, 0, -1, -1) != AIR_BLOCK;
					bool bottom_west = blockneighbors_get_block(blocks, -1, -1, -1) != AIR_BLOCK;
					bool west = blockneighbors_get_block(blocks, -1, 0, -1) != AIR_BLOCK;
					bool top_west = blockneighbors_get_block(blocks, -1, 1, -1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(bottom, east, bottom_east); // 0
					u8 aoV1 = vertex_ao(bottom, west, bottom_west); // 1
					u8 aoV2 = vertex_ao(top, west, top_west);// 3
					u8 aoV3 = vertex_ao(top, east, top_east);// 2

					push_face(&chunk_mesh, local, block, NORTH_FACE, aoV0, aoV1, aoV2, aoV3);
				}

				// East face
				u16 east_block = blockneighbors_get_block(blocks, 1, 0, 0);
				if (east_block == AIR_BLOCK) {
					bool top = blockneighbors_get_block(blocks, 1, 1, 0) != AIR_BLOCK;
					bool top_north = blockneighbors_get_block(blocks, 1, 1, -1) != AIR_BLOCK;
					bool north = blockneighbors_get_block(blocks, 1, 0, -1) != AIR_BLOCK;
					bool bottom_north = blockneighbors_get_block(blocks, 1, -1, -1) != AIR_BLOCK;
					bool bottom = blockneighbors_get_block(blocks, 1, -1, 0) != AIR_BLOCK;
					bool bottom_south = blockneighbors_get_block(blocks, 1, -1, 1) != AIR_BLOCK;
					bool south = blockneighbors_get_block(blocks, 1, 0, 1) != AIR_BLOCK;
					bool top_south = blockneighbors_get_block(blocks, 1, 1, 1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(south, bottom, bottom_south);
					u8 aoV1 = vertex_ao(north, bottom, bottom_north);
					u8 aoV2 = vertex_ao(north, top, top_north);
					u8 aoV3 = vertex_ao(south, top, top_south);

					push_face(&chunk_mesh, local, block, EAST_FACE, aoV0, aoV1, aoV2, aoV3);
				}

				// South face
				u16 south_block = blockneighbors_get_block(blocks, 0, 0, 1);
				if (south_block == AIR_BLOCK) {
					bool top = blockneighbors_get_block(blocks, 0, 1, 1) != AIR_BLOCK;
					bool top_east = blockneighbors_get_block(blocks, 1, 1, 1) != AIR_BLOCK;
					bool east = blockneighbors_get_block(blocks, 1, 0, 1) != AIR_BLOCK;
					bool bottom_east = blockneighbors_get_block(blocks, 1, -1, 1) != AIR_BLOCK;
					bool bottom = blockneighbors_get_block(blocks, 0, -1, 1) != AIR_BLOCK;
					bool bottom_west = blockneighbors_get_block(blocks, -1, -1, 1) != AIR_BLOCK;
					bool west = blockneighbors_get_block(blocks, -1, 0, 1) != AIR_BLOCK;
					bool top_west = blockneighbors_get_block(blocks, -1, 1, 1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(west, bottom, bottom_west);
					u8 aoV1 = vertex_ao(east, bottom, bottom_east);
					u8 aoV2 = vertex_ao(east, top, top_east);
					u8 aoV3 = vertex_ao(west, top, top_west);

					push_face(&chunk_mesh, local, block, SOUTH_FACE, aoV0, aoV1, aoV2, aoV3);
				}

				// West face
				u16 west_block = blockneighbors_get_block(blocks, -1, 0, 0);
				if (west_block == AIR_BLOCK) {
					bool top = blockneighbors_get_block(blocks, -1, 1, 0) != AIR_BLOCK;
					bool top_north = blockneighbors_get_block(blocks, -1, 1, -1) != AIR_BLOCK;
					bool north = blockneighbors_get_block(blocks, -1, 0, -1) != AIR_BLOCK;
					bool bottom_north = blockneighbors_get_block(blocks, -1, -1, -1) != AIR_BLOCK;
					bool bottom = blockneighbors_get_block(blocks, -1, -1, 0) != AIR_BLOCK;
					bool bottom_south = blockneighbors_get_block(blocks, -1, -1, 1) != AIR_BLOCK;
					bool south = blockneighbors_get_block(blocks, -1, 0, 1) != AIR_BLOCK;
					bool top_south = blockneighbors_get_block(blocks, -1, 1, 1) != AIR_BLOCK;

					u8 aoV0 = vertex_ao(north, bottom, bottom_north);
					u8 aoV1 = vertex_ao(south, bottom, bottom_south);
					u8 aoV2 = vertex_ao(south, top, top_south);
					u8 aoV3 = vertex_ao(north, top, top_north);

					push_face(&chunk_mesh, local, block, WEST_FACE, aoV0, aoV1, aoV2, aoV3);
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

		size_t stride = sizeof(struct Vertex);

		// Vertex
		GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)0));
		GL_CALL(glEnableVertexAttribArray(0));

		// Data (ao + texture)
		GL_CALL(glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, stride, (const void *)offsetof(struct Vertex, data)));
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

#if PROFILE_PERFORMANCE
	perf_report(&p, "Chunk Meshing");
#endif
}

void chunkrenderer_cleanup(struct Chunk *chunk) {
	assert(chunk != NULL);

	if (chunk->gl.vao != 0) {
		GL_CALL(glDeleteBuffers(1, &chunk->gl.ebo));
		chunk->gl.ebo = 0;

		GL_CALL(glDeleteBuffers(1, &chunk->gl.vbo));
		chunk->gl.vbo = 0;

		GL_CALL(glDeleteVertexArrays(1, &chunk->gl.vao));
		chunk->gl.vao = 0;
	}
}

void chunkrenderer_render(const struct Chunk *chunk) {
	assert(chunk != NULL);

	if (!chunk_get_flag(chunk, CHUNK_RENDERABLE_FLAG)) {
		return;
	}

	mat4 transformation;
	glm_mat4_identity(transformation);
	vec3 chunk_position = {chunk->x * CHUNK_WIDTH, 0, chunk->z * CHUNK_WIDTH};
	glm_translate(transformation, chunk_position);

	shader_uniform_mat4(Shaders[TERRAIN_SHADER], "model", transformation);
	GL_CALL(glBindVertexArray(chunk->gl.vao));
	GL_CALL(glDrawElements(GL_TRIANGLES, chunk->gl.element_count, GL_UNSIGNED_INT, 0));
	GL_CALL(glBindVertexArray(0));
}
