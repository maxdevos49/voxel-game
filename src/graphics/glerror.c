#include <stdio.h>
#include "glerror.h"

#ifndef LOGGING_H
#define CLR_RESET "\033[0m"
#define CLR_ERROR "\033[31m"
#endif

void glerror_log_(const char *file, int line, const char *function) {
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		const char *message = NULL;

		switch (error) {
		case GL_INVALID_ENUM:
			message = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			message = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			message = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			message = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			message = "GL_OUT_OF_MEMORY";
			break;
		default:
			message = "GL_UNKNOWN";
		}

		fprintf(stderr, CLR_ERROR "[GLERROR]" CLR_RESET " (%s:%d %s) %s(%d)\n", file, line, function, message, error);
	}
}
