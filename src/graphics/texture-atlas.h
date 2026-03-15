#ifndef GL_TEXTURE_ATLAS_H
#define GL_TEXTURE_ATLAS_H

#include <glad/gl.h>
#include <m4xdev/types.h>

struct TextureAtlas {
	GLuint id;
	GLint slot;
	u32 tile_width;
	u32 tile_height;
	u32 tile_count;
};

err_t textureatlas_create(GLint slot, const char *path, GLsizei tile_width, GLsizei tile_height, GLsizei tile_count, struct TextureAtlas *atlasout);
void textureatlas_destroy(struct TextureAtlas *atlas);

void textureatlas_bind(const struct TextureAtlas *atlas);

#endif
