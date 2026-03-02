#ifndef CHUNK_ITERATOR
#define CHUNK_ITERATOR

#include <m4xdev/types.h>
#include "manager.h"

struct ChunkIterator {
	u16 radius;
	struct ChunkCoord origin;

	i32 x, z;
	i32 dx, dz;

	size_t length;
	size_t index;
};

struct ChunkIterator chunkiterator_create(struct ChunkCoord origin, u16 radius);
err_t chunkiterator_next(struct ChunkIterator *iterator, struct ChunkCoord *nextout);

#endif
