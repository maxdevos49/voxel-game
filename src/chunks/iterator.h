#ifndef CHUNK_ITERATOR
#define CHUNK_ITERATOR

#include <m4xdev/types.h>

#include "chunk.h"

struct ChunkIterator {
	u16 radius;
	struct ChunkPos origin;

	i32 x, z;
	i32 dx, dz;

	size_t length;
	size_t index;
};

struct ChunkIterator chunkiterator_create(struct ChunkPos origin, u16 radius);
bool chunkiterator_next(struct ChunkIterator *iterator, struct ChunkPos *nextout);

#endif
