#pragma once

i32 init_opengl(void);
void quit_opengl(void);
void test_draw(void);

struct camera_params {
	struct {
		f32 x, y, z;
	} camera_pos;
	struct {
		f32 x, y, z;
	} look_at;
};

void set_camera(struct camera_params params);

struct cube_params {
	f32 r, g, b;
	f32 x, y, z;
};

void reset_cubes(void);
void add_cube(struct cube_params params);
void draw_world(void);
