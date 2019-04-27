#pragma once

struct block {
	enum {
		BLOCK_TYPE_PLAYER,
		BLOCK_TYPE_CUBE,
	} type;
	struct {
		u8 x, y, z;
	} pos;
	union {
		struct {
			struct {
				u32 id;
				f32 r, g, b;
			} color;
		} cube;
	};
};

struct level {
	u32 width, height, layers;
	u32 num_blocks;
	struct block blocks[MAX_CUBES];
};

void reset_level(struct level *level);
void build_level_from_strings(struct level *level, char ***strings);
