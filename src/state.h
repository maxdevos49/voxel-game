#ifndef STATE_H
#define STATE_H 1

#include <stdbool.h>
#include "camera.h"
#include "graphics/texture-atlas.h"

#define CHUNK_RADIUS (32)

struct AppState {
	struct {
		bool wireframe;
		bool vsync;
		bool facecull;
		bool antialias;
		struct Camera camera;

	} renderer;

	struct World *world;
	struct TextureAtlas tile_atlas;
};

#endif
