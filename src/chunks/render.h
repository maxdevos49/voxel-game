#ifndef CHUNK_RENDER_H
#define CHUNK_RENDER_H

#include <m4xdev/types.h>
#include "chunk.h"

void chunkrenderer_mesh(
	struct Chunk *chunk,
	const struct Chunk *north_chunk,
	const struct Chunk *north_east_chunk,
	const struct Chunk *east_chunk,
	const struct Chunk *south_east_chunk,
	const struct Chunk *south_chunk,
	const struct Chunk *south_west_chunk,
	const struct Chunk *west_chunk,
	const struct Chunk *north_west_chunk);

void chunkrenderer_cleanup(struct Chunk *chunk);

void chunkrenderer_render(const struct Chunk *chunk);

#endif
