#ifndef CHUNK_INTERNAL
#define CHUNK_INTERNAL

#include <m4xdev/types.h>
#include <glad/gl.h>

#include "chunk.h"

#define CHUNK_SECTION_COUNT	 (CHUNK_MAX_HEIGHT / CHUNK_WIDTH)
#define CHUNK_SECTION_VOLUME (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH)
#define CHUNK_SECTION_BITS	 4
#define CHUNK_SECTION_MASK	 (CHUNK_WIDTH - 1)

struct ChunkGL {
	size_t element_count;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
};

struct ChunkSection {
	u16 data[CHUNK_SECTION_VOLUME]; // zxy
};

struct Chunk {
	i64 x, z;
	i32 flags;
	i32 reserved;

	struct ChunkSection *sections[CHUNK_SECTION_COUNT];

	u8 heightmap[CHUNK_AREA];

	struct ChunkGL gl;
};

#endif
