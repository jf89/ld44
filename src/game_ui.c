#include "game_ui.h"

#include <assert.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gl_3_3.h"
#include "opengl.h"

#define PI 3.14159265358979f

static inline f32 rand_f32(f32 min, f32 max) {
	return (((f32)rand()) / ((f32)RAND_MAX)) * (max - min) + min;
}

enum program_state {
	STATE_AWAITING_INPUT,
	STATE_ANIMATING,
	STATE_FADE_IN,
	STATE_FADE_OUT,
	STATE_FINISHED,
};

static enum program_state cur_state;
static enum outcome program_outcome;

enum item_animator_state {
	ITEM_STATE_IDLE,
	ITEM_STATE_BOBBING,
	ITEM_STATE_MOVING,
	ITEM_STATE_COLLECTING,
	ITEM_STATE_FALLING,
	ITEM_STATE_REBOUND,
};

static u32 num_item_animators;
struct item_animator {
	enum item_animator_state state;
	struct {
		f32 x, y, z;
	} pos;
	u32 block_id;
	struct color color;
	u8 is_char;
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
			f32 dx, dy, dz;
		} rebound;
		struct {
			f32 start_time, duration;
		} collecting;
		struct {
			f32 start_time, duration;
			f32 sx, sy, sz;
			f32 ex, ey, ez;
		} falling;
	};
};
static struct item_animator item_animators[MAX_ITEMS];

// static u32 num_health_animators;
struct health_animator {
	enum {
		HEALTH_ANIM_IDLE,
		HEALTH_ANIM_FLASHING,
	} state;
	u8 amount;
	char text[MAX_HEALTH_TEXT];
	union {
		struct {
			f32 start_time, duration;
		} flashing;
	};
};
struct health_animator health_animators[MAX_COLORS];

static struct {
	f32 start_time, duration;
	struct {
		f32 r, g, b, a;
	} start_color;
	struct {
		f32 r, g, b, a;
	} end_color;
} fade_animator;

static u32 num_events;
static struct event events[MAX_EVENTS];

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
		ia->pos.x = e.move.x;
		ia->pos.y = e.move.y;
		ia->pos.z = e.move.z;
		ia->moving.start_time = e.start_time;
		ia->moving.duration   = e.duration;
	} break;
	case EVENT_TYPE_BOUNCE: {
		struct item_animator *ia = get_item_animator_by_id(e.block_id);
		assert(ia);
		ia->state = ITEM_STATE_REBOUND;
		ia->rebound.dx = e.bounce.dx;
		ia->rebound.dy = e.bounce.dy;
		ia->rebound.dz = e.bounce.dz;
		ia->rebound.start_time = e.start_time;
		ia->rebound.duration   = e.duration;
	} break;
	case EVENT_TYPE_COLLECTED: {
		struct item_animator *ia = get_item_animator_by_id(e.block_id);
		assert(ia);
		ia->state = ITEM_STATE_COLLECTING;
		ia->collecting.start_time = e.start_time;
		ia->collecting.duration   = e.duration;
	} break;
	case EVENT_TYPE_WIN:
		program_outcome = OUTCOME_SUCCESS;
		cur_state = STATE_FADE_OUT;
		fade_animator.start_time    = e.start_time;
		fade_animator.duration      = e.duration;
		fade_animator.start_color.r = 0.0f;
		fade_animator.start_color.b = 0.0f;
		fade_animator.start_color.g = 0.0f;
		fade_animator.start_color.a = 0.0f;
		fade_animator.end_color.r   = 0.0f;
		fade_animator.end_color.b   = 0.0f;
		fade_animator.end_color.g   = 0.0f;
		fade_animator.end_color.a   = 1.0f;
		break;
	case EVENT_TYPE_DEATH:
		program_outcome = OUTCOME_DEATH;
		cur_state = STATE_FADE_OUT;
		fade_animator.start_time    = e.start_time;
		fade_animator.duration      = e.duration;
		fade_animator.start_color.r = 0.0f;
		fade_animator.start_color.b = 0.0f;
		fade_animator.start_color.g = 0.0f;
		fade_animator.start_color.a = 0.0f;
		fade_animator.end_color.r   = 0.0f;
		fade_animator.end_color.b   = 0.0f;
		fade_animator.end_color.g   = 0.0f;
		fade_animator.end_color.a   = 1.0f;
		break;
	case EVENT_TYPE_FALL: {
		struct item_animator *ia = get_item_animator_by_id(e.block_id);
		assert(ia);
		ia->state = ITEM_STATE_FALLING;
		ia->falling.start_time = e.start_time;
		ia->falling.duration   = e.duration;
		ia->falling.sx = ia->pos.x;
		ia->falling.sy = ia->pos.y;
		ia->falling.sz = ia->pos.z;
		ia->falling.ex = e.fall.x;
		ia->falling.ey = e.fall.y;
		ia->falling.ez = e.fall.z;
		ia->pos.x = e.fall.x;
		ia->pos.y = e.fall.y;
		ia->pos.z = e.fall.z;
	} break;
	case EVENT_TYPE_LOSE_HEALTH: {
		struct health_animator *ha
			= &health_animators[e.lose_health.color];
		ha->state = HEALTH_ANIM_FLASHING;
		ha->amount = e.lose_health.new_amount;
		ha->flashing.start_time = e.start_time;
		ha->flashing.duration   = HEALTH_ANIM_DURATION;
	} break;
	case EVENT_TYPE_GAIN_HEALTH: {
		struct health_animator *ha
			= &health_animators[e.gain_health.color];
		ha->state = HEALTH_ANIM_FLASHING;
		ha->amount = e.gain_health.new_amount;
		ha->flashing.start_time = e.start_time;
		ha->flashing.duration   = HEALTH_ANIM_DURATION;
	} break;

	}
}

enum outcome run_game_ui(SDL_Window *window, struct level *level) {
	num_item_animators = 0;
	num_events = 0;

	cur_state = STATE_FADE_IN;
	fade_animator.start_time    = ((f32)SDL_GetTicks()) / 1000.0f;
	fade_animator.duration      = FADE_DURATION;
	fade_animator.start_color.r = 0.0f;
	fade_animator.start_color.b = 0.0f;
	fade_animator.start_color.g = 0.0f;
	fade_animator.start_color.a = 1.0f;
	fade_animator.end_color.r   = 0.0f;
	fade_animator.end_color.b   = 0.0f;
	fade_animator.end_color.g   = 0.0f;
	fade_animator.end_color.a   = 0.0f;
	// cur_state = STATE_AWAITING_INPUT;
	program_outcome = OUTCOME_QUIT;

	// Init anim state
	for (u32 i = 0; i < level->num_blocks; ++i) {
		struct block block = level->blocks[i];
		switch (block.type) {
		case BLOCK_TYPE_EMPTY:
			break;
		case BLOCK_TYPE_PLAYER: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.is_char = 1;
			ia.pos.x = (f32)block.pos.x;
			ia.pos.y = (f32)block.pos.y;
			ia.pos.z = (f32)block.pos.z;
			ia.color = level->player_color;
			ia.character = (u8)'\002';
			set_item_animator_state(&ia, ITEM_STATE_IDLE);
			item_animators[num_item_animators++] = ia;
		} break;
		case BLOCK_TYPE_CUBE: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.is_char = 0;
			ia.pos.x = (f32)block.pos.x;
			ia.pos.y = (f32)block.pos.y;
			ia.pos.z = (f32)block.pos.z;
			ia.color = level->color_map[block.cube.color];
			f32 variation = rand_f32(-0.08f, 0.08f);
			ia.color.r += variation;
			ia.color.g += variation;
			ia.color.b += variation;
			set_item_animator_state(&ia, ITEM_STATE_IDLE);
			item_animators[num_item_animators++] = ia;
		} break;
		case BLOCK_TYPE_HEART: {
			struct item_animator ia;
			ia.block_id = block.block_id;
			ia.is_char = 1;
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
			ia.is_char = 1;
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
	for (u32 i = 0; i < level->num_colors; ++i) {
		struct health_animator *ha = &health_animators[i];
		ha->amount = level->player_health[i];
	}

	glClearColor(level->background_color.r, level->background_color.g,
		level->background_color.b, 1.0f);
	while (cur_state != STATE_FINISHED) {
		SDL_Event e;
		enum move next_move = MOVE_NONE;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return OUTCOME_QUIT;
			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_q:
					return OUTCOME_QUIT;
				}
				break;
			case SDL_KEYDOWN:
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
				}
				break;
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
				case ITEM_STATE_MOVING:
					if (time > ia->moving.start_time
						+ ia->moving.duration) {

						set_item_animator_state(
							ia, ITEM_STATE_IDLE);
					} else {
						next_state = STATE_ANIMATING;
					}
					break;
				case ITEM_STATE_REBOUND:
					if (time > ia->rebound.start_time
						+ ia->rebound.duration) {
						set_item_animator_state(
							ia, ITEM_STATE_IDLE);
					} else {
						next_state = STATE_ANIMATING;
					}
					break;
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
				case ITEM_STATE_FALLING:
					if (time > ia->falling.start_time
						+ ia->falling.duration) {

						set_item_animator_state(
							ia, ITEM_STATE_IDLE);
					} else {
						next_state = STATE_ANIMATING;
					}
					break;
				}
				++i;
			}
			if (cur_state != STATE_FADE_OUT) {
				cur_state = next_state;
			}
		}
		if (cur_state==STATE_FADE_IN || cur_state==STATE_FADE_OUT) {
			if (time > fade_animator.start_time
					+ fade_animator.duration) {
				if (cur_state == STATE_FADE_IN) {
					cur_state = STATE_AWAITING_INPUT;
				} else if (cur_state == STATE_FADE_OUT) {
					cur_state = STATE_FINISHED;
				}
			}
		}

		// Draw
		// glClear();
		reset_cubes();
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
					+ 4.0f * dt*(1.0f - dt) * JUMP_HEIGHT;
			} break;
			case ITEM_STATE_REBOUND: {
				f32 dt = (time - ia.rebound.start_time)
					/ ia.rebound.duration;
				f32 dx = dt < 0.5f ? dt : 1.0f - dt;
				dx *= BOUNCE_DISTANCE;
				params.x += dx * ia.rebound.dx;
				params.z += dx * ia.rebound.dz;
				params.y += 4.0f * dt*(1.0f - dt)
					* JUMP_HEIGHT;
			} break;
			case ITEM_STATE_COLLECTING: {
				f32 dt = (time - ia.collecting.start_time)
					/ ia.collecting.duration;
				params.y += dt*dt;
			} break;
			case ITEM_STATE_FALLING: {
				f32 dt = (time - ia.falling.start_time)
					/ ia.falling.duration;
				params.x = (ia.falling.ex - ia.falling.sx) * dt
					+ ia.falling.sx;
				params.z = (ia.falling.ez - ia.falling.sz) * dt
					+ ia.falling.sz;
				params.y = (ia.falling.ey - ia.falling.sy) * dt
					+ ia.falling.sy;
			} break;

			}
			if (ia.is_char) {
				add_item(params);
			} else {
				struct cube_params c_params;
				c_params.r = params.r;
				c_params.g = params.g;
				c_params.b = params.b;
				c_params.x = params.x;
				c_params.y = params.y;
				c_params.z = params.z;
				add_cube(c_params);
			}
		}
		draw_world();
		reset_characters();
		for (u32 i = 1; i < level->num_colors; ++i) {
			struct health_animator *ha = &health_animators[i];
			sprintf(ha->text, "\003: %hhu", ha->amount);
				// level->player_health[i]);
			struct color c = level->color_map[i];
			if (ha->state == HEALTH_ANIM_FLASHING) {
				if (time > ha->flashing.start_time
						+ ha->flashing.duration) {
					ha->state = HEALTH_ANIM_IDLE;
				} else {
					f32 dt = (time
						- ha->flashing.start_time)
						/ ha->flashing.duration;
					if ((u32)(dt * (f32)NUM_HEALTH_FLASHES)
							% 2 == 0) {
						c = (struct color){
							.r=1.0f,
							.g=1.0f,
							.b=1.0f,
						};
					}
				}
			}
			add_string(ha->text, c, 4.0f, 2.0f, -((f32)i));
		}
		draw_characters();
		if (
				cur_state == STATE_FADE_IN  ||
				cur_state == STATE_FADE_OUT ||
				cur_state == STATE_FINISHED) {
			f32 dt = (time - fade_animator.start_time)
				/ fade_animator.duration;
			f32 r = (fade_animator.end_color.r
					- fade_animator.start_color.r)*dt
				+ fade_animator.start_color.r;
			f32 g = (fade_animator.end_color.g
					- fade_animator.start_color.g)*dt
				+ fade_animator.start_color.g;
			f32 b = (fade_animator.end_color.b
					- fade_animator.start_color.b)*dt
				+ fade_animator.start_color.b;
			f32 a = (fade_animator.end_color.a
					- fade_animator.start_color.a)*dt
				+ fade_animator.start_color.a;
			set_fade_color(r, g, b, a);
			draw_fade();
		}
		SDL_GL_SwapWindow(window);
	}

	return program_outcome;
}
