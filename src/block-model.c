#include <m4xdev/types.h>

#include "block-model.h"

struct BlockModel BlockModels[BLOCK_COUNT - 1];

static void compute_face_quad( // TODO this is trash and does not respect winding order. Nor does the epsilon texture thing work
	struct BlockModel *model,
	enum Face face,
	float x0, float y0, float z0,
	float x1, float y1, float z1) {
	struct Quad *quad = &model->computed[face];

	switch (face) {
	case TOP_FACE:
		// -Y
		quad->v0 = (struct Vertex){x0, y1, z1, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x1, y1, z1, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x1, y1, z0, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x0, y1, z0, 0.0f, 1.0f};
		break;

	case BOTTOM_FACE:
		// +Y
		quad->v0 = (struct Vertex){x0, y0, z0, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x1, y0, z0, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x1, y0, z1, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x0, y0, z1, 0.0f, 1.0f};
		break;

	case NORTH_FACE:
		// -Z
		quad->v0 = (struct Vertex){x1, y0, z0, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x0, y0, z0, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x0, y1, z0, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x1, y1, z0, 0.0f, 1.0f};
		break;

	case SOUTH_FACE:
		// +Z
		quad->v0 = (struct Vertex){x0, y0, z1, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x1, y0, z1, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x1, y1, z1, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x0, y1, z1, 0.0f, 1.0f};
		break;

	case EAST_FACE:
		// +X
		quad->v0 = (struct Vertex){x1, y0, z1, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x1, y0, z0, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x1, y1, z0, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x1, y1, z1, 0.0f, 1.0f};
		break;

	case WEST_FACE:
		// -X
		quad->v0 = (struct Vertex){x0, y0, z0, 0.0f, 0.0f};
		quad->v1 = (struct Vertex){x0, y0, z1, 1.0f, 0.0f};
		quad->v2 = (struct Vertex){x0, y1, z1, 1.0f, 1.0f};
		quad->v3 = (struct Vertex){x0, y1, z0, 0.0f, 1.0f};
		break;

	default:
		break;
	}

	float tile_width = 1.0f / BLOCK_TEXTURE_COUNT;
	int tex = model->faces[face];
	float u_min = tex * tile_width;
	float u_max = u_min + tile_width;

	float v_min = 0.0f;
	float v_max = 1.0f;

	float epsilon = 0.5f / 400.0f;

	u_min += epsilon;
	u_max -= epsilon;

	quad->v0.u = u_min;
	quad->v0.v = v_min;

	quad->v1.u = u_max;
	quad->v1.v = v_min;

	quad->v2.u = u_max;
	quad->v2.v = v_max;

	quad->v3.u = u_min;
	quad->v3.v = v_max;
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

	compute_block_models();
}
