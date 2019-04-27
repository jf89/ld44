#include "game.h"

#include <assert.h>
#include <SDL.h>

void reset_level(struct level *level) {
	level->width      = 0;
	level->height     = 0;
	level->layers     = 0;
	level->num_blocks = 1;
	level->blocks[0] = (struct block){
		.type     = BLOCK_TYPE_EMPTY,
		.block_id = 0,
	};
}

static void level_add_block(struct level *level, char block_char,
		u8 x, u8 y, u8 z) {
	switch (block_char) {
	case '1':
	case '2':
	case '#': {
		assert(level->num_blocks < MAX_BLOCKS);
		struct block block;
		block.type = BLOCK_TYPE_CUBE;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		switch (block_char) {
		case '#': block.cube.color = 0; break;
		case '1': block.cube.color = 1; break;
		case '2': block.cube.color = 2; break;
		}
		block.block_id = level->num_blocks;
		level->blocks[level->num_blocks++] = block;
	} break;
	case '@': {
		assert(level->num_blocks < MAX_BLOCKS);
		struct block block;
		block.type = BLOCK_TYPE_PLAYER;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		block.block_id = level->num_blocks;
		level->blocks[level->num_blocks++] = block;
	} break;
	case 'a':
	case 'b': {
		assert(level->num_blocks < MAX_BLOCKS);
		struct block block;
		block.type = BLOCK_TYPE_HEART;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		switch (block_char) {
		case 'a': block.heart.color = 1; break;
		case 'b': block.heart.color = 2; break;
		}
		block.block_id = level->num_blocks;
		level->blocks[level->num_blocks++] = block;
	} break;
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

struct block_pos {
	i8 x, y, z;
};

static struct block *get_player(struct level *level) {
	for (u32 i = 0; i < level->num_blocks; ++i) {
		struct block *b = &level->blocks[i];
		if (b->type == BLOCK_TYPE_PLAYER) {
			return b;
		}
	}
	assert(0);
}

static struct block *block_in_pos(struct level *level, i8 x, i8 y, i8 z) {
	for (u32 i = 0; i < level->num_blocks; ++i) {
		struct block *b = &level->blocks[i];
		if (b->pos.x == x && b->pos.y == y && b->pos.z == z) {
			return b;
		}
	}
	return &level->blocks[0];
}

static inline u32 can_walk_through(enum block_type type) {
	switch (type) {
	case BLOCK_TYPE_CUBE:
		return 0;
	case BLOCK_TYPE_EMPTY:
	case BLOCK_TYPE_HEART:
		return 1;
	case BLOCK_TYPE_PLAYER:
		assert(0);
		return 0;
	}
	assert(0);
	return 0;
}

static void do_move(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		struct block *mover, i8 x, i8 y, i8 z) {
	struct block *b = block_in_pos(level, x, y, z);
	if (can_walk_through(b->type)) {
		struct event *e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_MOVE;
		e->block_id = mover->block_id;
		e->start_time = 0.0f;
		e->duration = MOVE_DURATION;
		e->move.x = x;
		e->move.y = y;
		e->move.z = z;
		mover->pos.x = x;
		mover->pos.y = y;
		mover->pos.z = z;
	} else {
	}
}

void play_move(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		enum move move) {
	switch (move) {
	case MOVE_NONE:
		break;
	case MOVE_UP: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, player,
			player->pos.x, player->pos.y, player->pos.z + 1);
	} break;
	case MOVE_DOWN: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, player,
			player->pos.x, player->pos.y, player->pos.z - 1);
	} break;
	case MOVE_LEFT: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, player,
			player->pos.x - 1, player->pos.y, player->pos.z);
	} break;
	case MOVE_RIGHT: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, player,
			player->pos.x + 1, player->pos.y, player->pos.z);
	} break;
	}
}
