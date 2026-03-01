#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <m4xdev/types.h>
#include <m4xdev/logging.h>

#include "glerror.h"
#include "texture.h"
#include "graphics.h"

struct Texture texture_create(const char *path) {
	assert(path != NULL);

	stbi_set_flip_vertically_on_load(1);
	int width, height, channels;
	u8 *data = stbi_load(path, &width, &height, &channels, 0);
	if (data == NULL) {
		LOG_FATAL("Failed to load texture with path \"%s\"", path);
	}

	assert(channels == 4 && "Expected image to be rgba");

	GLuint id = 0;
	GL_CALL(glGenTextures(1, &id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

	stbi_image_free(data);
	data = NULL;

	return (struct Texture){.id = id};
}

void texture_bind(struct Texture self) {
	GL_CALL(glBindTexture(GL_TEXTURE_2D, self.id));
}

void texture_unbind(void) {
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

struct Texture Textures[TEXTURE_COUNT] = {0};

void texture_init_registry(void) {
	Textures[TERRAIN_TEXTURE] = texture_create("./resources/textures/blocks.png");
}
