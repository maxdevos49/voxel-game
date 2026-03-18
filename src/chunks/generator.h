#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include <m4xdev/types.h>

#include "chunk.h"

err_t chunk_generate(i64 chunkX, i64 chunkZ, struct Chunk **chunkout);
err_t chunk_generate_flat(i64 chunkX, i64 chunkZ, struct Chunk **chunkout);

#endif
