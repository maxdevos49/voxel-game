#ifndef WORLD_H
#define WORLD_H

#include <m4xdev/types.h>

struct World;

err_t world_create(u16 radius, struct World **worldout);
void world_destroy(struct World *world);

u16 world_get_block(const struct World *world, i64 worldX, i64 worldY, i64 worldZ);

#define SET_BLOCK_OK	   0
#define SET_BLOCK_TO_LOW   -1
#define SET_BLOCK_TO_HIGH  -2
#define SET_BLOCK_UNLOADED -3

err_t world_set_block(const struct World *world, i64 worldX, i64 worldY, i64 worldZ, u16 block);

void world_preload(struct World *world, double blockX, double blockZ);
void world_update(struct World *world, double blockX, double blockZ);
void world_render(struct World *world, double blockX, double blockZ);

void world_view_radius_set(struct World *world, u64 radius);

#endif
