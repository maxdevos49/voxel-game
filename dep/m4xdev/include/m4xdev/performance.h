#ifndef PERFORMANCE_H
#define PERFORMANCE_H 1

#include <assert.h>
#include <time.h>

#include "logging.h"

static double time_ms(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1e6);
}

struct Perf {
	double start;
};

static inline void perf_start(struct Perf *performance) {
	assert(performance != NULL);
	performance->start = time_ms();
}

static inline double perf_stop(const struct Perf *performance) {
	assert(performance != NULL);
	return time_ms() - performance->start;
}

static inline void perf_report(const struct Perf *performance, const char *name) {
	assert(performance != NULL);
	assert(name != NULL);

	double duration = time_ms() - performance->start;
	LOG_TRACE("%s | Perf %0.4fms", name, duration);
}

#endif
