#ifndef VAO_H
#define VAO_H 1

#include "graphics.h"
#include "glbuffer.h"

struct VertexArraysObject {
	GLuint id;
};

struct VertexArraysObject vao_create(void);
void vao_destroy(struct VertexArraysObject self);

void vao_bind(struct VertexArraysObject self);
void vao_unbind(void);
void vao_attr(struct VertexArraysObject self, struct GLBuffer vbo, GLuint index, GLenum type, GLint size, GLsizei stride, size_t offset);

#endif
