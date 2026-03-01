#ifndef STATE_H
#define STATE_H 1

#include <stdbool.h>

#include "chunk.h"
#include "camera.h"

#define CHUNK_DIAMETER (64)
#define CHUNK_COUNT (CHUNK_DIAMETER * CHUNK_DIAMETER)

struct AppState {
	struct {
		bool wireframe;
		bool vsync;
		struct Camera camera;

	} renderer;

	struct Chunk *chunks[CHUNK_COUNT];
};

#endif
