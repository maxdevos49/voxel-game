#include <stb_image.h>
#include <m4xdev/logging.h>
#include <string.h>

#include "graphics.h"
#include "glerror.h"

#include "texture-atlas.h"

err_t textureatlas_create(GLint slot, const char *path, GLsizei tile_width, GLsizei tile_height, GLsizei tile_count, struct TextureAtlas *atlasout) {
	assert(tile_width > 0);
	assert(tile_height > 0);
	assert(atlasout != NULL);

	GLint max_slots = 0;
	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_slots));
	if (slot < 0 || slot >= max_slots) {
		LOG_ERROR("Unsupported slot: %d", slot);
		return -1;
	}

	if (path == NULL) {
		LOG_ERROR("NULL path\n");
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	u8 *data = stbi_load(path, &width, &height, &channels, 0);
	if (data == NULL) {
		LOG_ERROR("Failed to load: %s\n", path);
		return -1;
	}

	if (channels != 4) {
		stbi_image_free(data);
		data = NULL;

		LOG_ERROR("Unsupported channel count: %d", channels);
		return -1;
	}

	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));

	GLuint id = 0;
	GL_CALL(glGenTextures(1, &id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, id));

	// Allocate data
	GL_CALL(glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_RGBA8,
		tile_width,
		tile_height,
		tile_count,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL));

	size_t tile_size = tile_width * tile_height * 4;
	u8 *tile_buffer = malloc(tile_size);
	if (tile_buffer == NULL) {
		return -1;
	}

	// Load layer data
	for (GLsizei layer = 0; layer < tile_count; layer++) {
		for (GLsizei y = 0; y < tile_height; y++) {
			memcpy(
				tile_buffer + y * tile_width * 4,
				data + (y * width + layer * tile_width) * 4,
				tile_width * 4);
		}

		GL_CALL(glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, layer,
			tile_width,
			tile_height,
			1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			tile_buffer));
	}

	free(tile_buffer);
	tile_buffer = NULL;

	stbi_image_free(data);
	data = NULL;

	// Anti aliasing
	GLfloat value, max_anisotropy = 16.0f;
	GL_CALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value));
	value = (value > max_anisotropy) ? max_anisotropy : value;
	GL_CALL(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, value));

	// Mipmaping
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));
	GL_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 16.0));

	// Wrapping
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT));

	atlasout->id = id;
	atlasout->slot = slot;
	atlasout->tile_width = tile_width;
	atlasout->tile_height = tile_height;
	atlasout->tile_count = tile_count;

	GL_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

	return 0;
}

void textureatlas_destroy(struct TextureAtlas *atlas) {
	assert(atlas != NULL);
	assert(atlas->id != 0);

	GL_CALL(glDeleteTextures(1, &atlas->id));

	atlas->id = 0;
	atlas->slot = 0;
	atlas->tile_width = 0;
	atlas->tile_height = 0;
	atlas->tile_count = 0;

	atlas = NULL;
}

void textureatlas_bind(const struct TextureAtlas *atlas) {
	assert(atlas != NULL);
	GL_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->id));
}
