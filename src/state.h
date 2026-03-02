#ifndef STATE_H
#define STATE_H 1

#include <stdbool.h>
#include "camera.h"
#include "chunks/manager.h"


#define CHUNK_RADIUS (32)

struct AppState {
	struct {
		bool wireframe;
		bool vsync;
		struct Camera camera;

	} renderer;

	struct ChunkManager *chunk_manager;
};

#endif
