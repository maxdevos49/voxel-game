//
// Author: Maxwell DeVos
// Defines: LOG_LEVEL
// Requires: -Wno-gnu-zero-variadic-macro-arguments
//

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#define LOG_LEVEL_TRACE 0 /* Or all logging	 */
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO	2
#define LOG_LEVEL_WARN	3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_NONE	6

#define CLR_RESET "\033[0m"
#define CLR_TRACE "\033[90m" /* Bright black (gray) */
#define CLR_DEBUG "\033[36m" /* Cyan */
#define CLR_INFO  "\033[32m" /* Green */
#define CLR_WARN  "\033[33m" /* Yellow */
#define CLR_ERROR "\033[31m" /* Red */
#define CLR_FATAL "\033[91m" /* Bright red */

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...) fprintf(stderr, CLR_TRACE "[TRACE] (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_TRACE(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) fprintf(stderr, CLR_DEBUG "[DEBUG] (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) fprintf(stdout, CLR_INFO "[INFO]  (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...) fprintf(stderr, CLR_WARN "[WARN]  (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) fprintf(stderr, CLR_ERROR "[ERROR] (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

#define LOG_FATAL(format, ...)                                                                                                               \
	do {                                                                                                                                     \
		if (LOG_LEVEL <= LOG_LEVEL_FATAL) {                                                                                                  \
			fprintf(stderr, CLR_FATAL "[FATAL] (%s:%d %s) " CLR_RESET format "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__); \
		}                                                                                                                                    \
		exit(EXIT_FAILURE);                                                                                                                  \
	} while (0)

#endif
