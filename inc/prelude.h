#pragma once

#include <stdint.h>

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

struct color {
	f32 r, g, b;
};

#define ARRAY_LENGTH(xs) (sizeof(xs) / sizeof((xs)[0]))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 700

#define FONT_TEX_WIDTH    288
#define FONT_TEX_HEIGHT   128
#define FONT_GLYPH_WIDTH  9
#define FONT_GLYPH_HEIGHT 16

#define MAX_CUBES        1000
#define MAX_LETTERS      1000
#define MAX_ITEMS        1000
#define MAX_BLOCKS       (MAX_CUBES + MAX_ITEMS)
#define MAX_COLORS       10
#define MAX_EVENTS       1000
#define MAX_HEALTH_TEXT  100
#define MAX_LEVEL_WIDTH  21
#define MAX_LEVEL_HEIGHT 21
#define MAX_LEVEL_LAYERS 10

#define MOVE_DURATION          0.25f
#define BOUNCE_DURATION        0.2f
#define BOUNCE_DISTANCE        0.5f
#define COLLECT_DURATION       0.25f
#define FADE_DURATION          0.5f
#define JUMP_HEIGHT            0.4f
#define FALL_SPEED             (16.0f * JUMP_HEIGHT)
#define HEALTH_ANIM_DURATION   0.25f
#define NUM_HEALTH_FLASHES     5

typedef struct { f32 elems[16]; } mat4;

#define MAT4(...) (mat4) { .elems = { __VA_ARGS__ } }

inline static mat4 mat_mul(mat4 a, mat4 b) {
#define A(c, r) a.elems[(r << 2) | c]
#define B(c, r) b.elems[(r << 2) | c]
#define DOT(c, r) A(0,r)*B(c,0) + A(1,r)*B(c,1) + A(2,r)*B(c,2) + A(3,r)*B(c,3)
	return MAT4(
		DOT(0, 0), DOT(1, 0), DOT(2, 0), DOT(3, 0),
		DOT(0, 1), DOT(1, 1), DOT(2, 1), DOT(3, 1),
		DOT(0, 2), DOT(1, 2), DOT(2, 2), DOT(3, 2),
		DOT(0, 3), DOT(1, 3), DOT(2, 3), DOT(3, 3),
	);
#undef DOT
#undef A
#undef B
}
