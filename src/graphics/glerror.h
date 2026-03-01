#ifndef GLERROR_H
#define GLERROR_H

#include <stdbool.h>
#include "graphics.h"

void glerror_log_(const char *file, int line, const char *function);

#ifdef DEBUG
    #define GL_CALL(x) do { glGetError(); x; glerror_log_(__FILE__, __LINE__, __func__); } while(0)
#else
    #define GL_CALL(x) x
#endif

#endif
