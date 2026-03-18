#ifndef CHUNK_ITERATOR
#define CHUNK_ITERATOR

#include <m4xdev/types.h>

#include "chunk.h"

struct ChunkIterator {
	i64 radius;
	struct ChunkPos origin;

	i64 x, z;
	i64 dx, dz;

	size_t length;
	size_t index;
};

struct ChunkIterator chunkiterator_create(struct ChunkPos origin, u16 radius);
bool chunkiterator_next(struct ChunkIterator *iterator, struct ChunkPos *nextout);

#endif
