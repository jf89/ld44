#pragma once

i32 init_opengl(void);
void quit_opengl(void);
void test_draw(void);

void set_camera(void);

struct cube_params {
	f32 r, g, b;
	f32 x, y, z;
};

void reset_cubes(void);
void add_cube(struct cube_params params);
