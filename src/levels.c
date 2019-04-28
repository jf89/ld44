#include "levels.h"

#include <assert.h>

#include "opengl.h"

static void build_level_0(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  3.0f;
	params.camera_pos.y =  5.0f;
	params.camera_pos.z = -9.0f;
	params.look_at.x = 3.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 3.0f;
	set_camera(params);

	level->layers = 2;
	level->width  = 7;
	level->height = 1;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	level->player_health[1] = 1;
	level->num_colors = 2;
	char *level_strs[] = {
		"@     !",

		"#######",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_1(struct level *level) {
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
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	level->player_health[1] = 1;
	level->num_colors = 2;
	char *level_strs[] = {
		"           ",
		" a   @   ! ",
		"           ",

		"### ### ###",
		"#######1###",
		"### ### ###",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_2(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =   5.0f;
	params.camera_pos.y =   9.0f;
	params.camera_pos.z = -12.0f;
	params.look_at.x = 5.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 3.0f;
	set_camera(params);

	level->layers =  2;
	level->width  = 11;
	level->height =  4;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	level->player_health[1] = 2;
	level->num_colors = 2;
	char *level_strs[] = {
		"    ###    ",
		"    # #    ",
		" @   1   ! ",
		"      #    ",

		"    ###    ",
		"### ### ###",
		"###########",
		"### ### ###",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_3(struct level *level) {
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
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.6f, .b=0.1f };
	level->color_map[2] = (struct color){ .r=0.1f, .g=0.6f, .b=1.0f };
	level->player_health[1] = 1;
	level->player_health[2] = 1;
	level->num_colors = 3;
	char *level_strs[] = {
		"           ",
		" @       ! ",
		"           ",

		"###1# #2###",
		"### #2# ###",
		"###2# #1###",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_4(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =   6.0f;
	params.camera_pos.y =  12.0f;
	params.camera_pos.z = -15.0f;
	params.look_at.x = 6.0f;
	params.look_at.y = 2.0f;
	params.look_at.z = 3.0f;
	set_camera(params);

	level->layers = 4;
	level->width  = 13;
	level->height = 3;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.6f, .b=0.1f };
	level->color_map[2] = (struct color){ .r=0.1f, .g=0.6f, .b=1.0f };
	level->player_health[1] = 1;
	level->player_health[2] = 1;
	level->num_colors = 3;
	char *level_strs[] = {
		"             ",
		" 1@2      !  ",
		"             ",

		"#####   #####",
		"###### ######",
		"#####   #####",

		"             ",
		"     # #     ",
		"             ",

		"             ",
		"     ###     ",
		"             ",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_5(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  5.0f;
	params.camera_pos.y =  12.0f;
	params.camera_pos.z = -12.0f;
	params.look_at.x = 5.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 5.0f;
	set_camera(params);

	level->layers =  2;
	level->width  = 11;
	level->height =  7;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 0.0f, .g = 1.0f, .b = 0.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	level->player_health[1] = 3;
	level->num_colors = 2;
	char *level_strs[] = {
		" a       a ",
		"a a     a a",
		" a  1 1  a ",
		"     @     ",
		" a  1 1    ",
		"a a      ! ",
		" a         ",

		"###     ###",
		"###     ###",
		"### ### ###",
		"    ###   ",
		"### ### 111",
		"###     111",
		"###     111",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_6(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  4.0f;
	params.camera_pos.y =  15.0f;
	params.camera_pos.z = -15.0f;
	params.look_at.x = 4.0f;
	params.look_at.y = 2.0f;
	params.look_at.z = 5.0f;
	set_camera(params);

	level->layers = 6;
	level->width  = 9;
	level->height = 9;
	level->background_color
		= (struct color){ .r = 0.1f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.9f, .g = 0.9f, .b = 0.9f };
	level->goal_color
		= (struct color){ .r = 1.0f, .g = 1.0f, .b = 1.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=1.0f };
	level->color_map[2] = (struct color){ .r=0.0f, .g=1.0f, .b=1.0f };
	level->player_health[1] = 2;
	level->player_health[2] = 2;
	level->num_colors = 3;
	char *level_strs[] = {
		"         ",
		" @       ",
		"         ",
		"         ",
		"         ",
		"         ",
		"         ",
		"         ",
		"         ",

		"###      ",
		"### a    ",
		"###      ",
		"         ",
		" b       ",
		"         ",
		"         ",
		"         ",
		"         ",

		"   ###   ",
		"   ### a ",
		"   ###   ",
		"###      ",
		"### a    ",
		"###      ",
		"         ",
		" b       ",
		"         ",

		"      ###",
		"      ###",
		"      ###",
		"   ###   ",
		"   ### b ",
		"   ###   ",
		"###      ",
		"### a    ",
		"###      ",

		"         ",
		"         ",
		"         ",
		"      ###",
		"      ###",
		"      ###",
		"   ###   ",
		"   ### ! ",
		"   ###   ",

		"         ",
		"         ",
		"         ",
		"         ",
		"         ",
		"         ",
		"      ###",
		"      ###",
		"      ###",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_7(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  5.0f;
	params.camera_pos.y =  14.0f;
	params.camera_pos.z = -10.0f;
	params.look_at.x = 5.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 5.0f;
	set_camera(params);

	level->layers =  3;
	level->width  = 11;
	level->height =  5;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 1.0f, .g = 1.0f, .b = 1.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0f, .b=0.0f };
	level->color_map[2] = (struct color){ .r=0.0f, .g=1.0f, .b=0.0f };
	level->color_map[3] = (struct color){ .r=0.0f, .g=0.0f, .b=1.0f };
	level->player_health[1] = 1;
	level->player_health[2] = 2;
	level->player_health[3] = 1;
	level->num_colors = 4;
	char *level_strs[] = {
		"            ",
		"  1         ",
		" @2       ! ",
		"  3         ",
		"            ",

		" ###        ",
		"####     ###",
		"#### # # ###",
		"####     ###",
		" ###        ",

		"            ",
		"            ",
		"   #######  ",
		"            ",
		"            ",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_8(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  6.0f;
	params.camera_pos.y =  16.0f;
	params.camera_pos.z = -10.0f;
	params.look_at.x = 6.0f;
	params.look_at.y = 0.0f;
	params.look_at.z = 6.0f;
	set_camera(params);

	level->layers =  3;
	level->width  = 13;
	level->height =  9;
	level->background_color
		= (struct color){ .r = 0.0f, .g = 0.0f, .b = 0.1f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 1.0f, .g = 1.0f, .b = 1.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.75f, .b=0.0f };
	level->color_map[2] = (struct color){ .r=0.0f, .g=0.75f, .b=1.0f };
	level->player_health[1] = 0;
	level->player_health[2] = 4;
	level->num_colors = 3;
	char *level_strs[] = {
		"             ",
		"   @         ",
		"             ",
		"  2 2        ",
		"   1       ! ",
		"  2 2        ",
		"             ",
		"             ",
		"             ",

		"  ###        ",
		"  ###        ",
		"  ###        ",
		"######    ###",
		"######    ###",
		"######    ###",
		"  ###        ",
		"  ###        ",
		"  ###        ",

		"             ",
		"             ",
		"             ",
		"     ######  ",
		"     ######  ",
		"     ######  ",
		"             ",
		"             ",
		"             ",
	};
	build_level_from_strings(level, level_strs);
}

static void build_level_9(struct level *level) {
	struct camera_params params;
	params.camera_pos.x =  4.5f;
	params.camera_pos.y =  10.0f;
	params.camera_pos.z = -10.0f;
	params.look_at.x = 4.5f;
	params.look_at.y = 0.0f;
	params.look_at.z = 5.0f;
	set_camera(params);

	level->layers =  5;
	level->width  = 10;
	level->height =  3;
	level->background_color
		= (struct color){ .r = 0.1f, .g = 0.0f, .b = 0.0f };
	level->player_color
		= (struct color){ .r = 0.75f, .g = 0.75f, .b = 0.75f };
	level->goal_color
		= (struct color){ .r = 1.0f, .g = 1.0f, .b = 1.0f };
	level->color_map[0] = (struct color){ .r=0.5f, .g=0.5f, .b=0.5f };
	level->color_map[1] = (struct color){ .r=1.0f, .g=0.0, .b=0.0f };
	level->color_map[2] = (struct color){ .r=0.0f, .g=1.0, .b=0.0f };
	level->player_health[1] = 0;
	level->player_health[2] = 3;
	level->num_colors = 3;
	char *level_strs[] = {
		"          ",
		"  2       ",
		"          ",

		"          ",
		"  2       ",
		"          ",

		"          ",
		" @1     ! ",
		"          ",

		"#####  ###",
		"#####  ###",
		"#####  ###",

		"          ",
		"    ####  ",
		"          ",
	};
	build_level_from_strings(level, level_strs);
}

i32 build_level(struct level *level, u32 n) {
	reset_level(level);
	switch (n) {
	case 0: build_level_0(level); return 0;
	case 1: build_level_1(level); return 0;
	case 2: build_level_2(level); return 0;
	case 3: build_level_5(level); return 0;
	case 4: build_level_3(level); return 0;
	case 5: build_level_4(level); return 0;
	case 6: build_level_6(level); return 0;
	case 7: build_level_7(level); return 0;
	case 8: build_level_8(level); return 0;
	case 9: build_level_9(level); return 0;
	}
	return 1;
}
