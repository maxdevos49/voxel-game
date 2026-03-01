#ifndef TYPES_H
#define TYPES_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define LEN(arr)  (sizeof(arr) / sizeof(arr[0]))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t usize;

#endif
