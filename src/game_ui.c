#include "game_ui.h"

#include <assert.h>
#include <SDL.h>
#include <stdlib.h>
#include <math.h>

#include "gl_3_3.h"
#include "opengl.h"

#define PI 3.14159265358979f

static inline f32 rand_f32(f32 min, f32 max) {
	return (((f32)rand()) / ((f32)RAND_MAX)) * (max - min) + min;
}

static u32 num_cube_animators;
struct cube_animator {
	u32 block_id;
	f32 x, y, z;
	f32 x_disp_mag, y_disp_mag, z_disp_mag;
	f32 x_disp_off, y_disp_off, z_disp_off;
	f32 x_disp_freq, y_disp_freq, z_disp_freq;
	struct color color;
};
static struct cube_animator cube_animators[MAX_CUBES];

enum item_animator_state {
	ITEM_STATE_IDLE,
	ITEM_STATE_BOBBING,
	ITEM_STATE_MOVING,
	ITEM_STATE_COLLECTING,
};

static u32 num_item_animators;
struct item_animator {
	enum item_animator_state state;
	struct {
		f32 x, y, z;
	} pos;
	u32 block_id;
	struct color color;
	u8 character;
	union {
		struct {
			struct {
				f32 mag, off, freq;
			} x_disp, y_disp, z_disp;
		} idle;
		struct {
			f32 mag, off, freq;
		} bobbing;
		struct {
			f32 start_time, duration;
			f32 sx, sy, sz;
			f32 ex, ey, ez;
		} moving;
		struct {
			f32 start_time, duration;
		} collecting;
	};
};
static struct item_animator item_animators[MAX_ITEMS];

static u32 num_events;
static struct event events[MAX_EVENTS];

enum program_state {
	STATE_AWAITING_INPUT,
	STATE_ANIMATING,
};

static struct item_animator *get_item_animator_by_id(u32 block_id) {
	for (u32 i = 0; i < num_item_animators; ++i) {
		if (item_animators[i].block_id == block_id) {
			return &item_animators[i];
		}
	}
	return NULL;
}

static void set_item_animator_state(
		struct item_animator *ia,
		enum item_animator_state new_state) {
	ia->state = new_state;
	switch (new_state) {
	case ITEM_STATE_IDLE:
		ia->idle.x_disp.mag  = rand_f32(0.025f, 0.05f);
		ia->idle.x_disp.off  = rand_f32(0.0f, 2.0f*PI);
		ia->idle.x_disp.freq = rand_f32(0.75f, 1.5f);
		ia->idle.y_disp.mag  = rand_f32(0.025f, 0.05f);
		ia->idle.y_disp.off  = rand_f32(0.0f, 2.0f*PI);
		ia->idle.y_disp.freq = rand_f32(0.75f, 1.5f);
		ia->idle.z_disp.mag  = rand_f32(0.025f, 0.05f);
		ia->idle.z_disp.off  = rand_f32(0.0f, 2.0f*PI);
		ia->idle.z_disp.freq = rand_f32(0.75f, 1.5f);
		break;
	}
}

static void start_animation(struct event e) {
	switch (e.type) {
	case EVENT_TYPE_MOVE: {
		struct item_animator *ia = get_item_animator_by_id(e.block_id);
		assert(ia);
		ia->state = ITEM_STATE_MOVING;
		ia->moving.sx = ia->pos.x;
		ia->moving.sy = ia->pos.y;
		ia->moving.sz = ia->pos.z;
		ia->moving.ex = e.move.x;
		ia->moving.ey = e.move.y;
		ia->moving.ez = e.move.z;
		ia->moving.start_time = e.start_time;
		ia->moving.duration   = e.duration;
	} break;
	case EVENT_TYPE_COLLECTED: {
		struct item_animator *ia = get_item_animator_by_id(e.block_id);
		assert(ia);
		ia->state = ITEM_STATE_COLLECTING;
		ia->collecting.start_time = e.start_time;
		ia->collecting.duration   = e.duration;
	} break;
	}
}

enum outcome run_game_ui(SDL_Window *window, struct level *level) {
	num_cube_animators = 0;
	num_events = 0;

	enum program_state cur_state = STATE_AWAITING_INPUT;

	// Init anim state
	for (u32 i = 0; i < level->num_blocks; ++i) {
		struct block block = level->blocks[i];
		switch (block.type) {
		case BLOCK_TYPE_EMPTY:
			break;
		case BLOCK_TYPE_PLAYER: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.pos.x = (f32)block.pos.x;
			ia.pos.y = (f32)block.pos.y;
			ia.pos.z = (f32)block.pos.z;
			ia.color = level->player_color;
			ia.character = (u8)'\002';
			set_item_animator_state(&ia, ITEM_STATE_IDLE);
			item_animators[num_item_animators++] = ia;
		} break;
		case BLOCK_TYPE_CUBE: {
			struct cube_animator ca;
			ca.block_id = block.block_id;
			ca.x = (f32)block.pos.x;
			ca.y = (f32)block.pos.y;
			ca.z = (f32)block.pos.z;
			ca.x_disp_mag = rand_f32(0.025f, 0.05f);
			ca.y_disp_mag = rand_f32(0.025f, 0.05f);
			ca.z_disp_mag = rand_f32(0.025f, 0.05f);
			ca.x_disp_off = rand_f32(0.0f, 2.0f*PI);
			ca.y_disp_off = rand_f32(0.0f, 2.0f*PI);
			ca.z_disp_off = rand_f32(0.0f, 2.0f*PI);
			ca.x_disp_freq = rand_f32(0.75f, 1.5f);
			ca.y_disp_freq = rand_f32(0.75f, 1.5f);
			ca.z_disp_freq = rand_f32(0.75f, 1.5f);
			ca.color = level->color_map[block.cube.color];
			f32 variation = rand_f32(-0.08f, 0.08f);
			ca.color.r += variation;
			ca.color.g += variation;
			ca.color.b += variation;
			cube_animators[num_cube_animators++] = ca;
		} break;
		case BLOCK_TYPE_HEART: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.state = ITEM_STATE_BOBBING;
			ia.pos.x = (f32)block.pos.x;
			ia.pos.y = (f32)block.pos.y;
			ia.pos.z = (f32)block.pos.z;

			ia.color = level->color_map[block.heart.color];
			ia.character = (u8)'\003';

			ia.bobbing.mag  = rand_f32(0.2f, 0.3f);
			ia.bobbing.off  = rand_f32(0.0f, 2.0f*PI);
			ia.bobbing.freq = rand_f32(1.0f, 1.5f);
			item_animators[num_item_animators++] = ia;
		} break;
		case BLOCK_TYPE_GOAL: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.state = ITEM_STATE_BOBBING;
			ia.pos.x = (f32)block.pos.x;
			ia.pos.y = (f32)block.pos.y;
			ia.pos.z = (f32)block.pos.z;

			ia.color = level->goal_color;
			ia.character = (u8)'!';

			ia.bobbing.mag  = rand_f32(0.2f, 0.3f);
			ia.bobbing.off  = rand_f32(0.0f, 2.0f*PI);
			ia.bobbing.freq = rand_f32(1.0f, 1.5f);
			item_animators[num_item_animators++] = ia;
		} break;
		}
	}

	glClearColor(level->background_color.r, level->background_color.g,
		level->background_color.b, 1.0f);
	while (1) {
		SDL_Event e;
		enum move next_move = MOVE_NONE;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return OUTCOME_QUIT;
			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					next_move = MOVE_UP;
					break;
				case SDLK_DOWN:
					next_move = MOVE_DOWN;
					break;
				case SDLK_LEFT:
					next_move = MOVE_LEFT;
					break;
				case SDLK_RIGHT:
					next_move = MOVE_RIGHT;
					break;
				case SDLK_q:
					return OUTCOME_QUIT;
				}
			}
		}

		f32 time = ((f32)SDL_GetTicks()) / 1000.0f;
		if (next_move != MOVE_NONE
				&& cur_state == STATE_AWAITING_INPUT) {
			play_move(level, &num_events, events, next_move);
			if (num_events) {
				cur_state = STATE_ANIMATING;
				for (u32 i = 0; i < num_events; ++i) {
					events[i].start_time += time;
				}
			}
		}

		if (cur_state == STATE_ANIMATING) {
			enum program_state next_state = STATE_AWAITING_INPUT;
			if (num_events) {
				next_state = STATE_ANIMATING;
			}
			u32 i = 0;
			while (i < num_events) {
				struct event e = events[i];
				if (time > e.start_time + e.duration) {
					events[i] = events[--num_events];
					continue;
				}
				if (time > e.start_time) {
					events[i] = events[--num_events];
					start_animation(e);
					continue;
				}
				++i;
			}
			i = 0;
			while (i < num_item_animators) {
				struct item_animator *ia
					= &item_animators[i];
				switch (ia->state) {
				case ITEM_STATE_IDLE:
				case ITEM_STATE_BOBBING:
					break;
				case ITEM_STATE_MOVING: {
					if (time > ia->moving.start_time
						+ ia->moving.duration) {

						ia->pos.x = ia->moving.ex;
						ia->pos.y = ia->moving.ey;
						ia->pos.z = ia->moving.ez;
						set_item_animator_state(
							ia, ITEM_STATE_IDLE);
					} else {
						next_state = STATE_ANIMATING;
					}
				} break;
				case ITEM_STATE_COLLECTING:
					if (time > ia->collecting.start_time
						+ ia->collecting.duration) {

						item_animators[i]
							= item_animators[
							--num_item_animators];
						continue;
					} else {
						next_state = STATE_ANIMATING;
					}
					break;

				}
				++i;
			}
			cur_state = next_state;
		}

		// Draw
		// glClear();
		reset_cubes();
		for (u32 i = 0; i < num_cube_animators; ++i) {
			struct cube_animator ca = cube_animators[i];
			struct cube_params params;
			params.x = ca.x+ca.x_disp_mag*sin(
				(ca.x_disp_off+time)*ca.x_disp_freq);
			params.y = ca.y+ca.y_disp_mag*sin(
				(ca.y_disp_off+time)*ca.y_disp_freq);
			params.z = ca.z+ca.z_disp_mag*sin(
				(ca.z_disp_off+time)*ca.z_disp_freq);
			params.r = ca.color.r;
			params.b = ca.color.b;
			params.g = ca.color.g;
			add_cube(params);
		}
		reset_items();
		for (u32 i = 0; i < num_item_animators; ++i) {
			struct item_animator ia = item_animators[i];
			struct item_params params;
			params.r = ia.color.r;
			params.g = ia.color.g;
			params.b = ia.color.b;
			params.x = ia.pos.x;
			params.y = ia.pos.y;
			params.z = ia.pos.z;
			// SDL_Log("%f %f %f", params.x, params.y, params.z);
			params.character = ia.character;
			switch (ia.state) {
			case ITEM_STATE_IDLE:
				params.x += ia.idle.x_disp.mag*(
					sin(ia.idle.x_disp.off
						+ ia.idle.x_disp.freq*time));
				params.y += ia.idle.y_disp.mag*(
					sin(ia.idle.y_disp.off
						+ ia.idle.y_disp.freq*time));
				params.z += ia.idle.z_disp.mag*(
					sin(ia.idle.z_disp.off
						+ ia.idle.z_disp.freq*time));
				break;
			case ITEM_STATE_BOBBING:
				params.y += ia.bobbing.mag * sin(
					ia.bobbing.off + ia.bobbing.freq*time);
				break;
			case ITEM_STATE_MOVING: {
				f32 dt = (time - ia.moving.start_time)
					/ ia.moving.duration;
				params.x = (ia.moving.ex - ia.moving.sx) * dt
					+ ia.moving.sx;
				params.z = (ia.moving.ez - ia.moving.sz) * dt
					+ ia.moving.sz;
				params.y = (ia.moving.ey - ia.moving.sy) * dt
					+ ia.moving.sy
					+ 4.0f * dt*(1.0f - dt) * 0.4f;
			} break;
			case ITEM_STATE_COLLECTING: {
				f32 dt = (time - ia.collecting.start_time)
					/ ia.collecting.duration;
				params.y += dt*dt;
			} break;

			}
			add_item(params);
		}
		draw_world();
		reset_characters();
		add_string("\003: 1",
			(struct color){ .r=1.0f, .g=0.0f, .b=0.0f },
			4.0f, 2.0f, -1.0f);
		draw_characters();
		SDL_GL_SwapWindow(window);
	}
}
