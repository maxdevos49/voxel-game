#include <m4xdev/types.h>
#include <stdio.h>

#include "iterator.h"
#include "manager.h"

struct ChunkIterator chunkiterator_create(struct ChunkCoord origin, u16 radius) {
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
	i32 x = it->x;
	i32 z = it->z;

	// Check for turn
	if (x == z || (x < 0 && x == -z) || (x > 0 && x == 1 - z)) {
		i32 tmp = it->dx;
		it->dx = -it->dz;
		it->dz = tmp;
	}

	// Move
	it->x += it->dx;
	it->z += it->dz;
	it->index++;
}

err_t chunkiterator_next(struct ChunkIterator *it, struct ChunkCoord *nextout) {
	if (it == NULL) return -1;
	if (nextout == NULL) return -1;

	i32 r = (i32)it->radius;

	while (it->index < it->length) {
		i32 x = it->x;
		i32 z = it->z;

		if (x >= -r && x <= r &&
			z >= -r && z <= r) {
			struct ChunkCoord coord;
			coord.x = it->origin.x + x;
			coord.z = it->origin.z + z;

			*nextout = coord;

			iterator_process(it);
			return 0;
		}

		iterator_process(it);
	}

	return 1;// Done
}
