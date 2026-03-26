#include <cglm/noise.h>
#include <m4xdev/performance.h>

#include "chunk.h"
#include "generator.h"

#include "../block-model.h"

#define PROFILE_PERFORMANCE false

#define SCALE		1000.6f
#define PERSISTANCE 0.4f
#define LACUNARITY	3.5f
#define OCTAVES		4
#define SEED		(u64)8675309

static inline float noise_sample2d(u8 octaves, float persistance, float lacunarity, i64 offsetX, i64 offsetY) {
	float amplitude = 1.0f;
	float frequency = 1.0f;

	float noise = 0.0f;
	for (u8 i = 0; i < octaves; i++) {
		float sampleX = (float)offsetX / SCALE * frequency;
		float sampleY = (float)offsetY / SCALE * frequency;

		noise += glm_perlin_vec2((vec2){sampleX, sampleY}) * amplitude;

		amplitude *= persistance;
		frequency *= lacunarity;
	}

	return noise;
}

err_t chunk_generate_flat(i64 chunkX, i64 chunkZ, struct Chunk **chunkout) {
	assert(chunkout != NULL);
	*chunkout = NULL;

	struct Chunk *chunk = NULL;
	if (chunk_allocate(chunkX, chunkZ, &chunk) != 0) {
		return -1;
	}

	i64 ground_level = 40;

	for (i64 x = 0; x < CHUNK_WIDTH; x++) {
		for (i64 z = 0; z < CHUNK_WIDTH; z++) {
			for (i64 y = 0; y <= ground_level; y++) {
				if (y == ground_level) {
					chunk_set_block(chunk, (struct BlockPos){x, y, z}, GRASS_BLOCK);
				} else if (ground_level - y < 5) {
					chunk_set_block(chunk, (struct BlockPos){x, y, z}, DIRT_BLOCK);
				} else {
					chunk_set_block(chunk, (struct BlockPos){x, y, z}, STONE_BLOCK);
				}
			}
		}
	}

	*chunkout = chunk;
	return 0;
}

err_t chunk_generate(i64 chunkX, i64 chunkZ, struct Chunk **chunkout) {
	assert(chunkout != NULL);
	*chunkout = NULL;

#if PROFILE_PERFORMANCE
	struct Perf p = {0};
	perf_start(&p);
#endif

	struct Chunk *chunk = NULL;
	if (chunk_allocate(chunkX, chunkZ, &chunk) != 0) {
		return -1;
	}

	for (i64 x = 0; x < CHUNK_WIDTH; x++) {
		for (i64 z = 0; z < CHUNK_WIDTH; z++) {
			float noise = noise_sample2d(
							  OCTAVES,
							  PERSISTANCE,
							  LACUNARITY,
							  chunkX * CHUNK_WIDTH + x,
							  chunkZ * CHUNK_WIDTH + z) *
					100 +
				60;

			i64 sea_level = 42;
			i64 elevation = (i64)MAX(MIN(noise, CHUNK_MAX_HEIGHT - 1), 0.0f);
			i64 tree_line = 100;

			for (i64 y = MAX(elevation, sea_level); y >= 0; y--) {
				struct BlockPos local = {x, y, z};

				if (y == 0) {
					chunk_set_block(chunk, local, BEDROCK_BLOCK);
				} else if (elevation < sea_level + 4 && y <= elevation && elevation - y < 4) {
					chunk_set_block(chunk, local, SAND_BLOCK);
				} else if (elevation < sea_level && y > elevation) {
					chunk_set_block(chunk, local, WATER_BLOCK);
				} else if (y < elevation && elevation - y < 6 && elevation < tree_line) {
					chunk_set_block(chunk, local, DIRT_BLOCK);
				} else if (y == elevation && elevation < tree_line) {
					chunk_set_block(chunk, local, GRASS_BLOCK);
				} else {
					chunk_set_block(chunk, local, STONE_BLOCK);
				}
			}
		}
	}

	*chunkout = chunk;
	chunk = NULL;

#if PROFILE_PERFORMANCE
	perf_report(&p, "Chunk Generation");
#endif

	return 0;
}
