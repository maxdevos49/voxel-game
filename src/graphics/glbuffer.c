#include <assert.h>
#include "glerror.h"
#include "glbuffer.h"

struct GLBuffer glbuffer_create(GLenum type, GLenum usage) {
	struct GLBuffer buffer = {
		.type = type,
		.usage = usage};

	GL_CALL(glGenBuffers(1, &buffer.id));

	return buffer;
}

void glbuffer_destroy(struct GLBuffer self) {
	assert(self.id != 0);

	GL_CALL(glDeleteBuffers(1, &self.id));
	self.id = 0;
}

void glbuffer_bind(struct GLBuffer self) {
	assert(self.id != 0);

	GL_CALL(glBindBuffer(self.type, self.id));
}

void glbuffer_data(struct GLBuffer self, GLsizeiptr size, const void *data) {
	assert(self.id != 0);

	glbuffer_bind(self);
	GL_CALL(glBufferData(self.type, size, data, self.usage));
}
