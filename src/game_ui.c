#include "game_ui.h"

#include <SDL.h>
#include <stdlib.h>
#include <math.h>

#include "opengl.h"

#define PI 3.14159265358979f

static inline f32 rand_f32(f32 min, f32 max) {
	return (((f32)rand()) / ((f32)RAND_MAX)) * (max - min) + min;
}

static u32 num_cube_animators;
struct cube_animator {
	f32 x, y, z;
	f32 x_disp_mag, y_disp_mag, z_disp_mag;
	f32 x_disp_off, y_disp_off, z_disp_off;
	f32 x_disp_freq, y_disp_freq, z_disp_freq;
	struct color color;
};
static struct cube_animator cube_animators[MAX_CUBES];

void run_game_ui(SDL_Window *window, struct level *level) {
	num_cube_animators = 0;
	// Init anim state
	for (u32 i = 0; i < level->num_blocks; ++i) {
		struct block block = level->blocks[i];
		switch (block.type) {
		case BLOCK_TYPE_PLAYER:
			// TODO -- add a player animator
			break;
		case BLOCK_TYPE_CUBE: {
			struct cube_animator cube_animator;
			cube_animator.x = (f32)block.pos.x;
			cube_animator.y = (f32)block.pos.y;
			cube_animator.z = (f32)block.pos.z;
			cube_animator.x_disp_mag = rand_f32(0.025f, 0.05f);
			cube_animator.y_disp_mag = rand_f32(0.025f, 0.05f);
			cube_animator.z_disp_mag = rand_f32(0.025f, 0.05f);
			cube_animator.x_disp_off = rand_f32(0.0f, 2.0f*PI);
			cube_animator.y_disp_off = rand_f32(0.0f, 2.0f*PI);
			cube_animator.z_disp_off = rand_f32(0.0f, 2.0f*PI);
			cube_animator.x_disp_freq = rand_f32(0.75f, 1.5f);
			cube_animator.y_disp_freq = rand_f32(0.75f, 1.5f);
			cube_animator.z_disp_freq = rand_f32(0.75f, 1.5f);
			cube_animator.color
				= level->color_map[block.cube.color];
			f32 variation = rand_f32(-0.08f, 0.08f);
			cube_animator.color.r += variation;
			cube_animator.color.g += variation;
			cube_animator.color.b += variation;
			cube_animators[num_cube_animators++]
				= cube_animator;
		} break;
		}
	}

	while (1) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return;
			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_q:
					return;
				}
			}
		}

		// Draw
		f32 time = ((f32)SDL_GetTicks()) / 1000.0f;
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
		draw_world();
		SDL_GL_SwapWindow(window);
	}
}
