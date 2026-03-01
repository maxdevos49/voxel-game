#include <stdint.h>
#include <assert.h>
#include "glerror.h"
#include "vao.h"

struct VertexArraysObject vao_create(void) {
	struct VertexArraysObject self = {0};
	GL_CALL(glGenVertexArrays(1, &self.id));
	return self;
}

void vao_destroy(struct VertexArraysObject self) {
	assert(self.id != 0);
	GL_CALL(glDeleteVertexArrays(1, &self.id));
	self.id = 0;
}

void vao_bind(struct VertexArraysObject self) {
	assert(self.id != 0);
	GL_CALL(glBindVertexArray(self.id));
}

void vao_unbind(void) {
	GL_CALL(glBindVertexArray(0));
}

// Could this functions signature be reduced by calculating the stride via the size and type? Same with offset via the index and size
void vao_attr(struct VertexArraysObject self, struct GLBuffer vbo, GLuint index, GLenum type, GLint size, GLsizei stride, size_t offset) {
	assert(self.id != 0);
	assert(vbo.id != 0 && vbo.type == GL_ARRAY_BUFFER);
	assert(type == GL_FLOAT); // Time to support glVertexAttribIPointer

	vao_bind(self);
	glbuffer_bind(vbo);

	GL_CALL(glVertexAttribPointer(index, size, type, GL_FALSE, stride, (const void *)offset));
	GL_CALL(glEnableVertexAttribArray(index));
}
