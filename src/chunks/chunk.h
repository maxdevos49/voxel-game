#ifndef CHUNK_H
#define CHUNK_H

#include <m4xdev/types.h>

#include "../block-model.h"
#include "../graphics/graphics.h"

#define CHUNK_WIDTH	 16
#define CHUNK_HEIGHT 128
#define CHUNK_VOLUME (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT)

#define CHUNK_BLOCK_INDEX(x, y, z) ((y) + CHUNK_HEIGHT * ((z) + CHUNK_WIDTH * (x)))

struct Chunk {
	i32 chunkX, chunkZ;

	struct {
		GLuint vao;
		GLuint vbo;
		GLuint ebo;

		size_t element_count; // Can vertex count be derived from this? I think yes?

	} gl;

	enum BlockType data[CHUNK_VOLUME];
};

i32 generate_chunk(i32 chunkX, i32 chunkZ, struct Chunk **out);
i32 destroy_chunk(struct Chunk *chunk);

// Rendering

i32 mesh_chunk(
	struct Chunk *chunk,
	struct Chunk *north_chunk,
	struct Chunk *east_chunk,
	struct Chunk *south_chunk,
	struct Chunk *west_chunk);
i32 render_chunk(const struct Chunk *chunk);

#endif
