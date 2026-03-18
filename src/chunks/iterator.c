#include <m4xdev/types.h>

#include "iterator.h"

struct ChunkIterator chunkiterator_create(struct ChunkPos origin, u16 radius) {
	struct ChunkIterator it;

	it.radius = radius;
	it.origin = origin;

	it.x = 0;
	it.z = 0;

	it.dx = 0;
	it.dz = -1;

	size_t diameter = (size_t)radius * 2 + 1;
	it.length = diameter * diameter;
	it.index = 0;

	return it;
}

static inline void iterator_process(struct ChunkIterator *it) {
	i64 x = it->x;
	i64 z = it->z;

	// Check for turn
	if (x == z || (x < 0 && x == -z) || (x > 0 && x == 1 - z)) {
		i64 tmp = it->dx;
		it->dx = -it->dz;
		it->dz = tmp;
	}

	// Move
	it->x += it->dx;
	it->z += it->dz;
	it->index++;
}

bool chunkiterator_next(struct ChunkIterator *it, struct ChunkPos *nextout) {
	assert(it != NULL);
	assert(nextout != NULL);

	i64 r = it->radius;

	while (it->index < it->length) {
		i64 x = it->x;
		i64 z = it->z;

		if (x >= -r && x <= r &&
			z >= -r && z <= r) {
			struct ChunkPos pos;
			pos.x = it->origin.x + x;
			pos.z = it->origin.z + z;

			*nextout = pos;

			iterator_process(it);
			return true;
		}

		iterator_process(it);
	}

	return false; // Done
}
