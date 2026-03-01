#include <assert.h>
#include <stdlib.h>
#include "glerror.h"
#include "shader.h"
#include <m4xdev/types.h>
#include <m4xdev/logging.h>

#include "../file/io.h"

static int shader_load_source(const char *path, char **dataptr) {
	assert(path != NULL);
	assert(dataptr != NULL);

	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		perror("fopen");
		return -1;
	}

	size_t size = 0;
	char *data = NULL;
	if (readall(file, &data, &size) != READALL_OK) {
		return -1;
	}

	*dataptr = data;

	return 0;
}

static int shader_compile(GLenum type, const char *source, GLuint *shaderId) {
	assert(source != NULL);
	assert(shaderId != NULL);

	GLuint id = 0;
	GL_CALL(id = glCreateShader(type));
	assert(id != 0);

	GL_CALL(glShaderSource(id, 1, &source, NULL));
	GL_CALL(glCompileShader(id));

	GLint status = GL_FALSE;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &status));

	if (status == GL_FALSE) {
		GLint size = 0;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &size));

		GLchar log[size];
		GL_CALL(glGetShaderInfoLog(id, size, NULL, log));
		GL_CALL(glDeleteShader(id));

		LOG_ERROR("%s", log);
		return -1;
	}

	*shaderId = id;

	return 0;
}

static int shader_link_program(GLuint vertexShaderId, GLuint fragmentShaderId, GLuint *programId) {
	assert(vertexShaderId != 0);
	assert(fragmentShaderId != 0);
	assert(programId != NULL);

	GLuint id = 0;
	GL_CALL(id = glCreateProgram());
	assert(id != 0);

	GL_CALL(glAttachShader(id, vertexShaderId));
	GL_CALL(glAttachShader(id, fragmentShaderId));
	GL_CALL(glLinkProgram(id));

	GLint status = GL_FALSE;
	GL_CALL(glGetProgramiv(id, GL_LINK_STATUS, &status));

	if (status == GL_FALSE) {
		GLint size = 0;
		GL_CALL(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &size));

		GLchar log[size];
		GL_CALL(glGetProgramInfoLog(id, size, NULL, log));
		GL_CALL(glDeleteProgram(id));

		LOG_ERROR("%s", log);
		return -1;
	}

	*programId = id;

	return 0;
}

struct ShaderProgram shader_create(const char *vertexPath, const char *fragmentPath) {
	assert(vertexPath != NULL);
	assert(fragmentPath != NULL);

	char *vertexSource = NULL;
	if (shader_load_source(vertexPath, &vertexSource) != 0) {
		LOG_FATAL("Failed to load vertex shader (%s)", vertexPath);
	}

	GLuint vertexId = 0;
	if (shader_compile(GL_VERTEX_SHADER, vertexSource, &vertexId) != 0) {
		LOG_FATAL("Failed to compile vertex shader (%s)", vertexPath);
	}

	free(vertexSource);
	vertexSource = NULL;

	char *fragmentSource = NULL;
	if (shader_load_source(fragmentPath, &fragmentSource) != 0) {
		LOG_FATAL("Failed to load fragment shader (%s)", fragmentSource);
	}

	GLuint fragmentId = 0;
	if (shader_compile(GL_FRAGMENT_SHADER, fragmentSource, &fragmentId) != 0) {
		LOG_FATAL("Failed to compile fragment shader (%s)", fragmentPath);
	}

	free(fragmentSource);
	fragmentSource = NULL;

	GLuint programId = 0;
	if (shader_link_program(vertexId, fragmentId, &programId) != 0) {
		LOG_FATAL("Failed to link shader program (%s, %s)", vertexPath, fragmentPath);
	}

	GL_CALL(glDeleteShader(vertexId));
	vertexId = 0;

	GL_CALL(glDeleteShader(fragmentId));
	fragmentId = 0;

	struct ShaderProgram program = {0};
	program.id = programId;
	return program;
}

void shader_destroy(struct ShaderProgram program) {
	assert(program.id != 0);
	GL_CALL(glDeleteProgram(program.id));
	program.id = 0;
}

void shader_bind(struct ShaderProgram program) {
	assert(program.id != 0);
	GL_CALL(glUseProgram(program.id));
}

void shader_uniform_mat4(struct ShaderProgram program, const char *name, mat4 transformation) {
	assert(program.id != 0);
	assert(name != NULL);

	// TODO cache uniform locations for shaders somehow. Maybe I add all uniform
	// locations to a registry just like the shaders.
	GLint location = 0;
	GL_CALL(location = glGetUniformLocation(program.id, name));
	assert(location != -1); // Unknown uniform check.

	GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat *)transformation));
}

//
// TODO this probably belongs outside this file
//

struct ShaderProgram Shaders[SHADER_COUNT] = {0};

void shader_init_registry(void) {
	Shaders[TERRAIN_SHADER] = shader_create("./resources/shaders/terrain.vert", "./resources/shaders/terrain.frag");
}

void shader_destroy_registry(void) {
	shader_destroy(Shaders[TERRAIN_SHADER]);
}
