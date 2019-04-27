#include "game.h"

#include <assert.h>
#include <SDL.h>

void reset_level(struct level *level) {
	level->width      = 0;
	level->height     = 0;
	level->layers     = 0;
	level->num_blocks = 0;
}

static void level_add_block(struct level *level, char block_char,
		u8 x, u8 y, u8 z) {
	switch (block_char) {
	case '1':
	case '#': {
		assert(level->num_blocks < MAX_CUBES);
		struct block block;
		block.type = BLOCK_TYPE_CUBE;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		switch (block_char) {
		case '#': block.cube.color = 0; break;
		case '1': block.cube.color = 1; break;
		}
		level->blocks[level->num_blocks++] = block;
	} break;
	case '@':
		// TODO -- add player
		break;
	case 'a':
		// TODO -- add hearts
		break;
	}
}

void build_level_from_strings(struct level *level, char **strings) {
	char **cur_str_ptr = strings;
	for (u8 j = level->layers; j; --j) {
		u8 y = j-1;
		for (u8 k = level->height; k; --k) {
			u8 z = k-1;
			char *cur_str = *cur_str_ptr;
			for (u8 x = 0; x < level->width; ++x) {
				level_add_block(level, cur_str[x], x, y, z);
			}
			++cur_str_ptr;
		}
	}
}
