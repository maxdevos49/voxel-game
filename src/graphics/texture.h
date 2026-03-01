#ifndef TEXTURE_H
#define TEXTURE_H

#include "graphics.h"

//
// Texture Resources:
// - https://www.youtube.com/watch?v=DuQDx0ZIxa8
// - https://www.youtube.com/watch?v=n4k7ANAFsIQ
// - https://learnopengl.com/Getting-started/Textures
// - https://open.gl/textures
// - https://registry.khronos.org/OpenGL-Refpages/gl4/html/texture.xhtml
//



struct Texture {
	GLuint id;
};

struct Texture texture_create(const char *path);
void texture_destroy(struct Texture self);

void texture_bind(struct Texture self);
void texture_unbind(void);

enum TextureTypes {
	TERRAIN_TEXTURE,
	TEXTURE_COUNT,
};

extern struct Texture Textures[TEXTURE_COUNT];

void texture_init_registry(void);
// void texture_destroy_registry(void);

#endif
