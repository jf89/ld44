#include "levels.h"

#include "opengl.h"

void build_level_1(struct level *level) {
	reset_level(level);
	struct camera_params params;
	params.camera_pos.x =   5.0f;
	params.camera_pos.y =  10.0f;
	params.camera_pos.z = -12.0f;
	params.look_at.x = 5.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 3.0f;
	set_camera(params);

	level->layers = 2;
	level->width  = 11;
	level->height = 3;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	char *level_strs[] = {
		"           ",
		" a   @     ",
		"           ",

		"### ### ###",
		"#######1###",
		"### ### ###",
	};
	build_level_from_strings(level, level_strs);
}
