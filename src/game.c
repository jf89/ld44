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
		i8 x, i8 y, i8 z) {
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
	case '!': {
		assert(level->num_blocks < MAX_BLOCKS);
		struct block block;
		block.type = BLOCK_TYPE_GOAL;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		block.block_id = level->num_blocks;
		level->blocks[level->num_blocks++] = block;
	}
	}
}

void build_level_from_strings(struct level *level, char **strings) {
	char **cur_str_ptr = strings;
	for (i8 j = level->layers; j; --j) {
		i8 y = j-1;
		for (i8 k = level->height; k; --k) {
			i8 z = k-1;
			char *cur_str = *cur_str_ptr;
			for (i8 x = 0; x < level->width; ++x) {
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

static void delete_block_by_id(struct level *level, u32 block_id) {
	for (u32 i = 0; i < level->num_blocks; ++i) {
		if (level->blocks[i].block_id == block_id) {
			level->blocks[i] = level->blocks[--level->num_blocks];
			return;
		}
	}
}

static inline u32 can_walk_through(enum block_type type) {
	switch (type) {
	case BLOCK_TYPE_CUBE:
	case BLOCK_TYPE_PLAYER:
		return 0;
	case BLOCK_TYPE_EMPTY:
	case BLOCK_TYPE_HEART:
	case BLOCK_TYPE_GOAL:
		return 1;
	}
	assert(0);
	return 0;
}

static inline u32 is_collectable(enum block_type type) {
	switch (type) {
	case BLOCK_TYPE_CUBE:
	case BLOCK_TYPE_PLAYER:
	case BLOCK_TYPE_EMPTY:
		return 0;
	case BLOCK_TYPE_HEART:
	case BLOCK_TYPE_GOAL:
		return 1;
	}
	assert(0);
	return 0;
}

static void collect(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time,
		struct block *actor,
		struct block *block) {
	struct block collected = *block;
	delete_block_by_id(level, collected.block_id);
	struct event e;
	e.type       = EVENT_TYPE_COLLECTED;
	e.block_id   = collected.block_id;
	e.start_time = time;
	e.duration   = COLLECT_DURATION;
	assert(*num_events_out < MAX_EVENTS);
	events_out[*num_events_out] = e;
	++(*num_events_out);
	if (actor->type == BLOCK_TYPE_PLAYER) {
		switch (collected.type) {
		case BLOCK_TYPE_EMPTY:
		case BLOCK_TYPE_PLAYER:
		case BLOCK_TYPE_CUBE:
			assert(0);
			break;
		case BLOCK_TYPE_HEART:
			// TODO -- add player health
			break;
		case BLOCK_TYPE_GOAL: {
			struct event e;
			e.type = EVENT_TYPE_WIN;
			e.block_id = 0;
			e.start_time = time + COLLECT_DURATION;
			e.duration   = 0.0f;
			assert(*num_events_out < MAX_EVENTS);
			events_out[*num_events_out] = e;
			++(*num_events_out);
		} break;

		}
	}
}

static void do_move(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time,
		struct block *mover, i8 dx, i8 dy, i8 dz) {
	i8 x = mover->pos.x + dx, y = mover->pos.y + dy, z = mover->pos.z + dz;
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
		if (is_collectable(b->type)) {
			collect(level, num_events_out, events_out, time,
				mover, b);
		}
		// TODO -- falling
		u32 fall_height = 0;
		do {
			--y; ++fall_height;
			b = block_in_pos(level, x, y, z);
		} while (y >= 0 && can_walk_through(b->type));
		--fall_height;
		if (y < 0) {
			// TODO -- fall to death
		}
	} else {
		switch (b->type) {
		case BLOCK_TYPE_CUBE:
			// TODO -- check for pushing cube
			break;
		}
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
		do_move(level, num_events_out, events_out, 0.0f,
			player, 0, 0, 1);
	} break;
	case MOVE_DOWN: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, 0.0f,
			player, 0, 0, -1);
	} break;
	case MOVE_LEFT: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, 0.0f,
			player, -1, 0, 0);
	} break;
	case MOVE_RIGHT: {
		struct block *player = get_player(level);
		do_move(level, num_events_out, events_out, 0.0f,
			player, 1, 0, 0);
	} break;
	}
}
