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

void reset_characters(void);
void add_string(char *string, struct color color, f32 zoom, f32 x, f32 y);
void draw_characters(void);

struct item_params {
	f32 r, g, b;
	f32 x, y, z;
	u8 character;
};

void reset_items(void);
void add_item(struct item_params params);
