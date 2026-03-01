#ifndef BLOCK_H
#define BLOCK_H 1

#include <cglm/cglm.h>

enum BlockType {
	STONE_BLOCK,
	DIRT_BLOCK,
	GRASS_BLOCK,
	AIR_BLOCK,
	BLOCK_COUNT,
};

enum BlockTexture {
	STONE_TEXTURE,
	DIRT_TEXTURE,
	GRASS_TEXTURE,
	GRASS_SIDE_TEXTURE,
	MISSING_TEXTURE,
	BLOCK_TEXTURE_COUNT
};

enum Face {
	TOP_FACE,
	BOTTOM_FACE,
	NORTH_FACE,
	EAST_FACE,
	SOUTH_FACE,
	WEST_FACE,
	FACE_COUNT
};

struct Vertex {
	float x;
	float y;
	float z;
	float u;
	float v;
};

// Take some inspiration from: https://minecraft.wiki/w/Model
struct BlockModel {
	ivec3 from;
	ivec3 to;
	enum BlockTexture faces[FACE_COUNT];

	// Do not manually assign
	struct Quad {
		struct Vertex v0;
		struct Vertex v1;
		struct Vertex v2;
		struct Vertex v3;
	} computed[FACE_COUNT];
};

extern struct BlockModel BlockModels[BLOCK_COUNT - 1]; // -1 to not include air block

void blocks_init_registry(void);

#endif
