#ifndef STATE_H
#define STATE_H 1

#include <stdbool.h>
#include "camera.h"
#include "graphics/texture-atlas.h"

#ifndef DEFAULT_VIEW_RADIUS
#define DEFAULT_VIEW_RADIUS 64
#endif
struct AppState {
	struct {
		u64 radius;
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
