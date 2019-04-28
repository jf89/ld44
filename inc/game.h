#pragma once

enum block_type {
	BLOCK_TYPE_EMPTY,
	BLOCK_TYPE_PLAYER,
	BLOCK_TYPE_CUBE,
	BLOCK_TYPE_HEART,
	BLOCK_TYPE_GOAL,
};

struct block {
	enum block_type type;
	u32 block_id;
	struct {
		i8 x, y, z;
	} pos;
	union {
		struct {
			u8 color;
		} cube;
		struct {
			u8 color;
		} heart;
	};
};

struct level {
	u32 width, height, layers;
	u32 num_blocks;
	struct block blocks[MAX_BLOCKS];
	struct color background_color, player_color, goal_color;
	u32 num_colors;
	struct color color_map[MAX_COLORS];
	u8 player_health[MAX_COLORS];
};

enum move {
	MOVE_NONE,
	MOVE_UP,
	MOVE_DOWN,
	MOVE_LEFT,
	MOVE_RIGHT,
};

struct event {
	enum {
		EVENT_TYPE_BOUNCE,
		EVENT_TYPE_MOVE,
		EVENT_TYPE_COLLECTED,
		EVENT_TYPE_WIN,
		EVENT_TYPE_FALL,
		EVENT_TYPE_DEATH,
		EVENT_TYPE_LOSE_HEALTH,
		EVENT_TYPE_GAIN_HEALTH,
	} type;
	u32 block_id;
	f32 start_time, duration;
	union {
		struct {
			i8 dx, dy, dz;
		} bounce;
		struct {
			i8 x, y, z;
		} move;
		struct {
			i8 x, y, z;
		} fall;
		struct {
			u8 color;
			u8 amount;
			u8 new_amount;
		} lose_health;
		struct {
			u8 color;
			u8 amount;
			u8 new_amount;
		} gain_health;
	};
};

void reset_level(struct level *level);
void build_level_from_strings(struct level *level, char **strings);
void play_move(
	struct level *level,
	u32 *num_events_out,
	struct event *events_out,
	enum move move);
