#ifndef BUFFER_H
#define BUFFER_H 1

#include <stdbool.h>
#include "graphics.h"

struct GLBuffer {
	GLuint id;
	GLenum type; 	// GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
	GLenum usage; 	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW
};

struct GLBuffer glbuffer_create(GLenum type, GLenum usage);
void glbuffer_destroy(struct GLBuffer self);
void glbuffer_bind(struct GLBuffer self);
void glbuffer_data(struct GLBuffer self, GLsizeiptr size, const void* data);

#endif
