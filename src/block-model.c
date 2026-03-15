#include <m4xdev/types.h>
#include "block-model.h"

struct BlockModel BlockModels[BLOCK_COUNT];

static void compute_face_quad(
	struct BlockModel *model,
	enum Face face,
	float x0, float y0, float z0,
	float x1, float y1, float z1) {
	struct Quad *quad = &model->computed[face];


	u32 data = (0 & 0x3) | (model->faces[face] << 2);
	switch (face) {
	case TOP_FACE:
		// -Y
		quad->v0 = (struct Vertex){x0, y1, z1, data};
		quad->v1 = (struct Vertex){x1, y1, z1, data};
		quad->v2 = (struct Vertex){x1, y1, z0, data};
		quad->v3 = (struct Vertex){x0, y1, z0, data};
		break;

	case BOTTOM_FACE:
		// +Y
		quad->v0 = (struct Vertex){x0, y0, z0, data};
		quad->v1 = (struct Vertex){x1, y0, z0, data};
		quad->v2 = (struct Vertex){x1, y0, z1, data};
		quad->v3 = (struct Vertex){x0, y0, z1, data};
		break;

	case NORTH_FACE:
		// -Z
		quad->v0 = (struct Vertex){x1, y0, z0, data};
		quad->v1 = (struct Vertex){x0, y0, z0, data};
		quad->v2 = (struct Vertex){x0, y1, z0, data};
		quad->v3 = (struct Vertex){x1, y1, z0, data};
		break;

	case SOUTH_FACE:
		// +Z
		quad->v0 = (struct Vertex){x0, y0, z1, data};
		quad->v1 = (struct Vertex){x1, y0, z1, data};
		quad->v2 = (struct Vertex){x1, y1, z1, data};
		quad->v3 = (struct Vertex){x0, y1, z1, data};
		break;

	case EAST_FACE:
		// +X
		quad->v0 = (struct Vertex){x1, y0, z1, data};
		quad->v1 = (struct Vertex){x1, y0, z0, data};
		quad->v2 = (struct Vertex){x1, y1, z0, data};
		quad->v3 = (struct Vertex){x1, y1, z1, data};
		break;

	case WEST_FACE:
		// -X
		quad->v0 = (struct Vertex){x0, y0, z0, data};
		quad->v1 = (struct Vertex){x0, y0, z1, data};
		quad->v2 = (struct Vertex){x0, y1, z1, data};
		quad->v3 = (struct Vertex){x0, y1, z0, data};
		break;

	default:
		break;
	}
}

static void compute_block_models(void) {
	for (u32 i = 0; i < BLOCK_COUNT; i++) {
		struct BlockModel *model = &BlockModels[i];

		float x0 = model->from[0] / 16.0f;
		float y0 = model->from[1] / 16.0f;
		float z0 = model->from[2] / 16.0f;

		float x1 = model->to[0] / 16.0f;
		float y1 = model->to[1] / 16.0f;
		float z1 = model->to[2] / 16.0f;

		for (u32 face = 0; face < FACE_COUNT; face++) {
			compute_face_quad(
				model,
				face,
				x0, y0, z0,
				x1, y1, z1);
		}
	}
}

void blocks_init_registry(void) {
	BlockModels[AIR_BLOCK] = (struct BlockModel){
		.from = {0, 0, 0},
		.to = {16, 16, 16}};

	BlockModels[STONE_BLOCK] = (struct BlockModel){
		.from = {0, 0, 0},
		.to = {16, 16, 16},
		.faces = {
			STONE_TEXTURE,
			STONE_TEXTURE,
			STONE_TEXTURE,
			STONE_TEXTURE,
			STONE_TEXTURE,
			STONE_TEXTURE,
		}};

	BlockModels[DIRT_BLOCK] = (struct BlockModel){
		.from = {0, 0, 0},
		.to = {16, 16, 16},
		.faces = {
			DIRT_TEXTURE,
			DIRT_TEXTURE,
			DIRT_TEXTURE,
			DIRT_TEXTURE,
			DIRT_TEXTURE,
			DIRT_TEXTURE,
		}};

	BlockModels[GRASS_BLOCK] = (struct BlockModel){
		.from = {0, 0, 0},
		.to = {16, 16, 16},
		.faces = {
			[TOP_FACE] = GRASS_TEXTURE,
			[BOTTOM_FACE] = DIRT_TEXTURE,
			[NORTH_FACE] = GRASS_SIDE_TEXTURE,
			[EAST_FACE] = GRASS_SIDE_TEXTURE,
			[SOUTH_FACE] = GRASS_SIDE_TEXTURE,
			[WEST_FACE] = GRASS_SIDE_TEXTURE,
		}};

	BlockModels[WATER_BLOCK] = (struct BlockModel){
		.from = {0, 0, 0},
		.to = {16, 15, 16},
		.faces = {
			WATER_TEXTURE,
			WATER_TEXTURE,
			WATER_TEXTURE,
			WATER_TEXTURE,
			WATER_TEXTURE,
			WATER_TEXTURE,
		}};

	compute_block_models();
}
