#ifndef SHADER_H_
#define SHADER_H_

#include <cglm/cglm.h>
#include "graphics.h"

enum ShaderPrograms {
	TERRAIN_SHADER = 0,
	SHADER_COUNT
};


struct Shader { // TODO expose individual shader program components to save GPU program mem.
	GLuint id;
	GLenum type;
};

struct ShaderProgram {
	GLuint id;
};

extern struct ShaderProgram Shaders[SHADER_COUNT];

void shader_init_registry(void);
void shader_destroy_registry(void);

struct ShaderProgram shader_create(const char *vertexPath, const char *fragmentPath);
void shader_destroy(struct ShaderProgram program);
void shader_bind(struct ShaderProgram program);
void shader_uniform_mat4(struct ShaderProgram program, const char *name, mat4 transformation);

#endif
