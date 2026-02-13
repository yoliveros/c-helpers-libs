#pragma once

#include <stdint.h>

// Signed int types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Unsigned int types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Boolean types
typedef i8 b8;
typedef i32 b32;

// Floating point types
typedef float f32;
typedef double f64;

static const u8 TRUE = 1;
static const u8 FALSE = 0;

// String
typedef struct {
  u8 *str;
  u64 len;
} string;

// Platform declaration
#if defined(_WIN32)
#define P_WINDOWS 1
#elif defined(__linux__)
#define P_LINUX 1
#endif

#define STR_FMT(str) (int)(str).len, (str).str

static inline string STR_LIT(u8 *str) {
  return (string){.str = str, .len = sizeof(str) - 1};
}

static inline u64 KiB(u64 n) { return n << 10; }
static inline u64 MiB(u64 n) { return n << 20; }
static inline u64 GiB(u64 n) { return n << 30; }

static inline u64 MIN(i32 a, i32 b) { return a < b ? a : b; }
static inline u64 MAX(i32 a, i32 b) { return a > b ? a : b; }
static inline u64 ALIGN_UP_POW2(u64 n, u64 p) {
  return (n + (p - 1)) & ~(p - 1);
}
