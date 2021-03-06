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
	for (u32 i = 0; i < MAX_COLORS; ++i) {
		level->color_map[i] = (struct color){.r=0.0f,.g=0.0f,.b=0.0f};
		level->player_health[i] = 0;
	}
}

static void level_add_block(struct level *level, char block_char,
		i8 x, i8 y, i8 z) {
	switch (block_char) {
	case '1':
	case '2':
	case '3':
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
		case '3': block.cube.color = 3; break;
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
	case 'b':
	case 'c': {
		assert(level->num_blocks < MAX_BLOCKS);
		struct block block;
		block.type = BLOCK_TYPE_HEART;
		block.pos.x = x;
		block.pos.y = y;
		block.pos.z = z;
		switch (block_char) {
		case 'a': block.heart.color = 1; break;
		case 'b': block.heart.color = 2; break;
		case 'c': block.heart.color = 3; break;
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
		if (b->type == BLOCK_TYPE_EMPTY) {
			continue;
		}
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

static void gain_health(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time,
		struct block *recipient,
		u8 color, u8 amount) {
	if (recipient->type != BLOCK_TYPE_PLAYER) {
		return;
	}
	struct event *e = &events_out[*num_events_out];
	++(*num_events_out);
	e->type       = EVENT_TYPE_GAIN_HEALTH;
	e->block_id   = recipient->block_id;
	e->start_time = time;
	e->duration   = HEALTH_ANIM_DURATION;
	e->gain_health.color  = color;
	e->gain_health.amount = amount;
	assert(((u32)level->player_health[color]) + ((u32)amount) < 256);
	level->player_health[color] += amount;
	e->gain_health.new_amount = level->player_health[color];
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
	e.collect.block_type = collected.type;
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
			gain_health(level, num_events_out, events_out,
				time, actor, collected.heart.color, 1);
			break;
		case BLOCK_TYPE_GOAL: {
			struct event e;
			e.type = EVENT_TYPE_WIN;
			e.block_id = 0;
			e.start_time = time + COLLECT_DURATION;
			e.duration   = FADE_DURATION;
			assert(*num_events_out < MAX_EVENTS);
			events_out[*num_events_out] = e;
			++(*num_events_out);
		} break;

		}
	}
}

static void lose_health(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time,
		struct block *victim,
		u8 color, u8 amount) {
	if (victim->type != BLOCK_TYPE_PLAYER) {
		return;
	}
	struct event *e;
	if (color == 0) {
		for (u32 i = 1; i < level->num_colors; ++i) {
			e = &events_out[*num_events_out];
			++(*num_events_out);
			e->type = EVENT_TYPE_LOSE_HEALTH;
			e->block_id = victim->block_id;
			e->start_time = time;
			e->duration   = HEALTH_ANIM_DURATION;
			e->lose_health.color  = i;
			e->lose_health.amount = amount;
			if (level->player_health[i] > amount) {
				level->player_health[i] -= amount;
			} else {
				level->player_health[i] = 0;
			}
			e->lose_health.new_amount = level->player_health[i];
		}
	} else {
		e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_LOSE_HEALTH;
		e->block_id = victim->block_id;
		e->start_time = time;
		e->duration   = HEALTH_ANIM_DURATION;
		e->lose_health.color  = color;
		e->lose_health.amount = amount;
		if (level->player_health[color] > amount) {
			level->player_health[color] -= amount;
		} else {
			level->player_health[color] = 0;
		}
		e->lose_health.new_amount = level->player_health[color];
	}
	u32 player_alive = 0;
	for (u32 i = 1; i < level->num_colors; ++i) {
		if (level->player_health[i]) {
			player_alive = 1;
			break;
		}
	}
	if (!player_alive) {
		e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_DEATH;
		e->start_time = time + HEALTH_ANIM_DURATION;
		e->duration = FADE_DURATION;
	}
}

static void do_fall(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time, struct block *faller) {
	i8 ox = faller->pos.x, oy = faller->pos.y, oz = faller->pos.z;
	i8 x = ox, y = oy, z = oz;
	u32 fall_height = 0;
	f32 fall_duration = 0.0f;
	struct block *b;
	struct event *e;
	do {
		--y; ++fall_height;
		b = block_in_pos(level, x, y, z);
		// TODO -- add collection while falling through items
	} while (y >= 0 && can_walk_through(b->type));
	--fall_height;
	if (fall_height) {
		e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_FALL;
		e->block_id = faller->block_id;
		e->start_time = time;
		fall_duration = ((f32)fall_height) / FALL_SPEED;
		e->duration   = fall_duration;
		e->fall.x = x;
		e->fall.y = y+1;
		e->fall.z = z;
		faller->pos.x = x;
		faller->pos.y = y+1;
		faller->pos.z = z;
		if (y < 0 && faller->type == BLOCK_TYPE_PLAYER) {
			e = &events_out[*num_events_out];
			++(*num_events_out);
			e->type = EVENT_TYPE_DEATH;
			e->start_time = time + fall_duration;
			e->duration = FADE_DURATION;
		}
		if (b->type != BLOCK_TYPE_EMPTY) {
			lose_health(level, num_events_out, events_out,
				time + fall_duration,
				faller, 0, fall_height);
		}
		struct block *above = block_in_pos(level, ox, oy+1, oz);
		if (above->type == BLOCK_TYPE_CUBE && above->cube.color) {
			do_fall(level, num_events_out, events_out,
				time + BETWEEN_FALL_DELAY, above);
		}
	}
	if (b->type != BLOCK_TYPE_EMPTY) {
		switch (b->type) {
		case BLOCK_TYPE_CUBE:
			if (b->cube.color) {
				lose_health(level, num_events_out,
					events_out, time + fall_duration,
					faller, b->cube.color, 1);
			}
			break;
		}
	}
}

static void do_move(
		struct level *level,
		u32 *num_events_out,
		struct event *events_out,
		f32 time,
		struct block *mover, i8 dx, i8 dy, i8 dz) {
	i8 ox = mover->pos.x, oy = mover->pos.y, oz = mover->pos.z;
	i8 x = ox + dx, y = oy + dy, z = oz + dz;
	struct block *b = block_in_pos(level, x, y, z);
	if (can_walk_through(b->type)) {
		struct event *e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_MOVE;
		e->block_id = mover->block_id;
		e->start_time = time;
		e->duration = MOVE_DURATION;
		e->move.is_player = (mover->type == BLOCK_TYPE_PLAYER) ? 1 : 0;
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
		do_fall(level, num_events_out, events_out,
			time + MOVE_DURATION, mover);
		b = block_in_pos(level, ox, oy+1, oz);
		if (b->type == BLOCK_TYPE_CUBE && b->cube.color) {
			do_fall(level, num_events_out, events_out,
				time + MOVE_DURATION, b);
		}

	} else {
		struct event *e = &events_out[*num_events_out];
		++(*num_events_out);
		e->type = EVENT_TYPE_BOUNCE;
		e->block_id = mover->block_id;
		e->start_time = time;
		e->duration = BOUNCE_DURATION;
		e->bounce.dx = dx;
		e->bounce.dy = dy;
		e->bounce.dz = dz;
		switch (b->type) {
		case BLOCK_TYPE_CUBE:
			// TODO -- check for pushing cube
			if (b->cube.color) {
				lose_health(level, num_events_out, events_out,
					time + BOUNCE_DURATION / 2.0f,
					mover, b->cube.color, 1);
				do_move(level, num_events_out, events_out,
					time + BOUNCE_DURATION / 2.0f,
					b, dx, dy, dz);
			}
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
