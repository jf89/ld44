#include "opengl.h"

#include <assert.h>
#include <SDL.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "gl_3_3.h"

#define FONT_TEX_LOC 0

// =============================================================================
// helper functions
// =============================================================================

static GLuint compile_shader(GLenum shader_type, const GLchar *shader_src) {
	GLuint shader = glCreateShader(shader_type);
	if (shader == 0) {
		SDL_Log("Failed to create shader.");
		return 0;
	}
	glShaderSource(shader, 1, &shader_src, NULL);
	glCompileShader(shader);
	GLint compile_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE) {
		SDL_Log("Failed to compile shader.");
		SDL_Log("%s", shader_src);
		GLint log_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		char *buffer = malloc((log_length + 1) * sizeof(GLchar));
		glGetShaderInfoLog(shader, log_length + 1, NULL, buffer);
		SDL_Log(buffer);
		free(buffer);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint link_program(GLuint program) {
	glLinkProgram(program);
	GLint link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE) {
		SDL_Log("Failed to link program.");
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		char *buffer = malloc((log_length + 1) * sizeof(GLchar));
		glGetProgramInfoLog(program, log_length + 1, NULL, buffer);
		SDL_Log(buffer);
		free(buffer);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

static void print_matrix(mat4 m) {
	for (u32 j = 0; j < 4; ++j) {
		SDL_Log("%f %f %f %f",
			m.elems[(j << 2) + 0],
			m.elems[(j << 2) + 1],
			m.elems[(j << 2) + 2],
			m.elems[(j << 2) + 3]);
	}
}

// =============================================================================
// fade shader
// =============================================================================

static GLuint fade_buffer, fade_vao;
static GLuint fade_program, fade_vert_shader, fade_frag_shader;
static GLint fade_color_loc;

struct fade_vertex {
	f32 x, y;
};

static struct fade_vertex fade_vertices[] = {
	{ .x = -1.0f, .y = -1.0f },
	{ .x =  1.0f, .y = -1.0f },
	{ .x =  1.0f, .y =  1.0f },

	{ .x = -1.0f, .y = -1.0f },
	{ .x =  1.0f, .y =  1.0f },
	{ .x = -1.0f, .y =  1.0f },
};

static const char *fade_vert_shader_src = SHADER_SRC(
	layout (location = 0) in vec2 pos;

	void main() {
		gl_Position = vec4(pos, 0.0f, 1.0f);
	}
);

static const char *fade_frag_shader_src = SHADER_SRC(
	uniform vec4 fade_color;

	layout (location = 0) out vec4 color;

	void main() {
		color = fade_color;
	}
);

i32 init_fade(void) {
	// TODO -- error checking
	fade_vert_shader = compile_shader(GL_VERTEX_SHADER,   fade_vert_shader_src);
	fade_frag_shader = compile_shader(GL_FRAGMENT_SHADER, fade_frag_shader_src);

	fade_program = glCreateProgram();
	glAttachShader(fade_program, fade_vert_shader);
	glAttachShader(fade_program, fade_frag_shader);
	fade_program = link_program(fade_program);

	fade_color_loc = glGetUniformLocation(fade_program, "fade_color");

	glGenVertexArrays(1, &fade_vao);
	glBindVertexArray(fade_vao);

	glGenBuffers(1, &fade_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, fade_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fade_vertices), &fade_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}

void free_fade(void) {
	glDeleteProgram(fade_program);
	glDeleteShader(fade_vert_shader);
	glDeleteShader(fade_frag_shader);

	glDeleteBuffers(1, &fade_buffer);
	glDeleteVertexArrays(1, &fade_vao);
}

void set_fade_color(f32 r, f32 g, f32 b, f32 a) {
	glUseProgram(fade_program);
	glUniform4f(fade_color_loc, r, g, b, a);
}

void draw_fade(void) {
	glEnable(GL_BLEND);
	glUseProgram(fade_program);
	glBindVertexArray(fade_vao);
	glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(fade_vertices));
	glDisable(GL_BLEND);
}

// =============================================================================
// cube shader
// =============================================================================

static GLuint cube_static_buffer, cube_index_buffer, cube_buffer, cube_vao;
static GLuint cube_program, cube_vert_shader, cube_frag_shader;
static GLint cube_proj_mat_loc, cube_ambient_loc, cube_directional_loc, cube_light_dir_loc;

struct cube_static_vertex {
	struct {
		f32 x, y, z;
	} pos;
	struct {
		f32 x, y, z;
	} normal;
};

static struct cube_static_vertex cube_static_vertices[] = {
	{ .pos = { .x = -0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x = -1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x = -1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x = -1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x = -1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x =  1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x =  1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x =  1.0f, .y =  0.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x =  1.0f, .y =  0.0f, .z =  0.0f } },

	{ .pos = { .x = -0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y = -1.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y = -1.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  1.0f, .z =  0.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  1.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y = -1.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y = -1.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  1.0f, .z =  0.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  1.0f, .z =  0.0f } },

	{ .pos = { .x = -0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z = -1.0f } },
	{ .pos = { .x = -0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z =  1.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z = -1.0f } },
	{ .pos = { .x = -0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z =  1.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z = -1.0f } },
	{ .pos = { .x =  0.5f, .y = -0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z =  1.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z = -0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z = -1.0f } },
	{ .pos = { .x =  0.5f, .y =  0.5f, .z =  0.5f }, .normal = { .x =  0.0f, .y =  0.0f, .z =  1.0f } },
};

static u16 cube_static_indices[] = {
	 0,  2,  1,  1,  2,  3,
	 4,  5,  6,  6,  5,  7,
	 8,  9, 13,  0, 13, 12,
	10, 14, 15, 10, 15, 11,
	21, 17, 19, 21, 19, 23,
	16, 20, 18, 20, 22, 18,
};

static u32 num_cubes;
static struct cube_params cube_instance_params[MAX_CUBES];

void reset_cubes(void) {
	num_cubes = 0;
}

void add_cube(struct cube_params params) {
	assert(num_cubes < MAX_CUBES);
	cube_instance_params[num_cubes++] = params;
}

static const char *cube_vert_shader_src = SHADER_SRC(
	uniform mat4 projection_matrix;
	uniform vec3 ambient_light;
	uniform vec3 directional_light;
	uniform vec3 light_direction;

	layout (location = 0) in vec3 vertex_pos;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec3 color;
	layout (location = 3) in vec3 center_pos;

	out vec3 through_color;

	void main() {
		through_color = color * (ambient_light + dot(-light_direction, normal) * directional_light);
		vec3 pos = 0.95f * vertex_pos + center_pos;
		gl_Position = projection_matrix * vec4(pos, 1.0f);
	}
);

static const char *cube_frag_shader_src = SHADER_SRC(
	layout (location = 0) out vec4 color;

	in vec3 through_color;

	void main() {
		color = vec4(through_color, 1.0f);
	}
);

static i32 init_cube(void) {
	// TODO -- error checking
	cube_vert_shader = compile_shader(GL_VERTEX_SHADER,   cube_vert_shader_src);
	cube_frag_shader = compile_shader(GL_FRAGMENT_SHADER, cube_frag_shader_src);
	cube_program = glCreateProgram();
	glAttachShader(cube_program, cube_vert_shader);
	glAttachShader(cube_program, cube_frag_shader);
	cube_program = link_program(cube_program);

	cube_proj_mat_loc    = glGetUniformLocation(cube_program, "projection_matrix");
	cube_ambient_loc     = glGetUniformLocation(cube_program, "ambient_light");
	cube_directional_loc = glGetUniformLocation(cube_program, "directional_light");
	cube_light_dir_loc   = glGetUniformLocation(cube_program, "light_direction");

	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	glGenBuffers(1, &cube_static_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_static_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_static_vertices), &cube_static_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct cube_static_vertex), (GLvoid*)offsetof(struct cube_static_vertex, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct cube_static_vertex), (GLvoid*)offsetof(struct cube_static_vertex, normal));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &cube_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_static_indices), &cube_static_indices, GL_STATIC_DRAW);

	glGenBuffers(1, &cube_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_instance_params), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct cube_params), (GLvoid*)offsetof(struct cube_params, r));
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct cube_params), (GLvoid*)offsetof(struct cube_params, x));
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);

	return 0;
}

static void free_cube(void) {
	glDeleteProgram(cube_program);
	glDeleteShader(cube_vert_shader);
	glDeleteShader(cube_frag_shader);

	glDeleteVertexArrays(1, &cube_vao);
	glDeleteBuffers(1, &cube_static_buffer);
	glDeleteBuffers(1, &cube_index_buffer);
	glDeleteBuffers(1, &cube_buffer);
}

static void set_cube_proj_mat(mat4 m) {
	glUseProgram(cube_program);
	glUniformMatrix4fv(cube_proj_mat_loc, 1, GL_TRUE, m.elems);
}

static void set_cube_ambient_light(f32 r, f32 g, f32 b) {
	glUseProgram(cube_program);
	glUniform3f(cube_ambient_loc, r, g, b);
}

static void set_cube_directional_light(f32 r, f32 g, f32 b) {
	glUseProgram(cube_program);
	glUniform3f(cube_directional_loc, r, g, b);
}

static void set_cube_light_direction(f32 x, f32 y, f32 z) {
	glUseProgram(cube_program);
	glUniform3f(cube_light_dir_loc, x, y, z);
}

static void draw_cubes(void) {
	glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_cubes * sizeof(struct cube_params), cube_instance_params);
	glUseProgram(cube_program);
	glBindVertexArray(cube_vao);
	glDrawElementsInstanced(
		GL_TRIANGLES,
		ARRAY_LENGTH(cube_static_indices),
		GL_UNSIGNED_SHORT,
		(GLvoid*)0,
		num_cubes);
}

// =============================================================================
// font shader
// =============================================================================

static GLuint font_static_buffer, font_buffer, font_vao;
GLuint font_vert_shader, font_frag_shader, font_program;
static GLint screen_size_loc, glyph_tex_size_loc, glyph_screen_size_loc, font_tex_loc;

struct font_static_vertex {
	f32 x, y;
};

static struct font_static_vertex font_static_vertices[] = {
	{ .x = 0.0f, .y = 0.0f },
	{ .x = 1.0f, .y = 0.0f },
	{ .x = 1.0f, .y = 1.0f },

	{ .x = 0.0f, .y = 0.0f },
	{ .x = 1.0f, .y = 1.0f },
	{ .x = 0.0f, .y = 1.0f },
};

struct font_instance_params {
	f32 x, y;
	f32 zoom;
	u8 character;
	f32 r, g, b;
};

static u32 num_chars;
static struct font_instance_params font_instances[MAX_LETTERS];

static const char *font_vert_shader_src = SHADER_SRC(
	uniform vec2 screen_size;
	uniform vec2 glyph_screen_size;
	uniform vec2 glyph_tex_size;

	layout (location = 0) in vec2 pos;
	layout (location = 1) in vec2 screen_pos;
	layout (location = 2) in float zoom;
	layout (location = 3) in uint character;
	layout (location = 4) in vec3 color_in;

	out vec2 tex_coord;
	out vec3 color;

	void main() {
		vec2 glyph_loc = vec2(character % 32u, 7u - (character / 32u));
		tex_coord = (glyph_loc + pos) * glyph_tex_size;
		color = color_in;
		vec2 out_pos = (screen_pos + pos) * zoom * glyph_screen_size;
		out_pos.y += 1.0f - zoom * glyph_screen_size.y;
		out_pos = 2.0f * out_pos - 1.0f;
		gl_Position = vec4(out_pos, 0.0f, 1.0f);
	}
);

static const char *font_frag_shader_src = SHADER_SRC(
	uniform sampler2D font_tex;

	layout (location = 0) out vec4 color_out;

	in vec3 color;
	in vec2 tex_coord;

	void main() {
		vec4 color_sample = texture(font_tex, tex_coord);
		if (color_sample.a == 0.0f) {
			discard;
		}
		color_out = vec4(color_sample.rgb * color, 1.0f);
	}
);

static i32 init_font(void) {
	// TODO -- error checking
	font_vert_shader = compile_shader(GL_VERTEX_SHADER,   font_vert_shader_src);
	font_frag_shader = compile_shader(GL_FRAGMENT_SHADER, font_frag_shader_src);
	font_program = glCreateProgram();
	glAttachShader(font_program, font_vert_shader);
	glAttachShader(font_program, font_frag_shader);
	font_program = link_program(font_program);

	screen_size_loc       = glGetUniformLocation(font_program, "screen_size");
	glyph_tex_size_loc    = glGetUniformLocation(font_program, "glyph_tex_size");
	glyph_screen_size_loc = glGetUniformLocation(font_program, "glyph_screen_size");
	font_tex_loc          = glGetUniformLocation(font_program, "font_tex");
	// SDL_Log("%d %d %d %d", screen_size_loc, glyph_tex_size_loc, glyph_screen_size_loc, font_tex_loc);

	glUseProgram(font_program);
	glUniform2f(screen_size_loc,    SCREEN_WIDTH, SCREEN_HEIGHT);
	glUniform2f(glyph_screen_size_loc,
		((f32)FONT_GLYPH_WIDTH)  / ((f32)SCREEN_WIDTH),
		((f32)FONT_GLYPH_HEIGHT) / ((f32)SCREEN_HEIGHT));
	glUniform2f(glyph_tex_size_loc,
		((f32)FONT_GLYPH_WIDTH)  / ((f32)FONT_TEX_WIDTH),
		((f32)FONT_GLYPH_HEIGHT) / ((f32)FONT_TEX_HEIGHT));
	glUniform1i(font_tex_loc, FONT_TEX_LOC);

	glGenVertexArrays(1, &font_vao);
	glBindVertexArray(font_vao);

	glGenBuffers(1, &font_static_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, font_static_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(font_static_vertices), &font_static_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
		sizeof(struct font_static_vertex), (GLvoid*)offsetof(struct font_static_vertex, x));
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &font_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, font_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(font_instances), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(struct font_instance_params), (GLvoid*)offsetof(struct font_instance_params, x));
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
		sizeof(struct font_instance_params), (GLvoid*)offsetof(struct font_instance_params, zoom));
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);

	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE,
		sizeof(struct font_instance_params), (GLvoid*)offsetof(struct font_instance_params, character));
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct font_instance_params), (GLvoid*)offsetof(struct font_instance_params, r));
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);

	return 0;
}

static void free_font(void) {
	glDeleteProgram(font_program);
	glDeleteShader(font_vert_shader);
	glDeleteShader(font_frag_shader);

	glDeleteBuffers(1, &font_static_buffer);
	glDeleteBuffers(1, &font_buffer);
}

void reset_characters(void) {
	num_chars = 0;
}

static void add_character(struct font_instance_params params) {
	assert(num_chars < MAX_LETTERS);
	font_instances[num_chars++] = params;
}

void add_string(char *string, struct color color, f32 zoom, f32 x, f32 y) {
	struct font_instance_params params;
	params.x = x;
	params.y = y;
	params.r = color.r;
	params.g = color.g;
	params.b = color.b;
	params.zoom = zoom;
	for (char *p = string; *p; ++p) {
		params.character = (u8)*p;
		add_character(params);
		params.x += 1.0f;
	}
}

void draw_characters(void) {
	glBindBuffer(GL_ARRAY_BUFFER, font_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_chars * sizeof(struct font_instance_params), font_instances);
	glUseProgram(font_program);
	glBindVertexArray(font_vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, ARRAY_LENGTH(font_static_vertices), num_chars);
}

// =============================================================================
// item
// =============================================================================

static GLuint item_static_buffer, item_buffer, item_vao;
GLuint item_vert_shader, item_frag_shader, item_program;
static GLint item_glyph_tex_size_loc, item_font_tex_loc;
static GLint item_proj_mat_loc, item_ambient_loc, item_directional_loc, item_light_dir_loc;

struct item_static_vertex {
	struct {
		f32 x, y, z;
	} pos;
	struct {
		f32 x, y, z;
	} normal;
	struct {
		f32 u, v;
	} tex;
};

static struct item_static_vertex item_static_vertices[] = {
	{ .pos={ .x=-0.4f, .y=-0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.01f, .v=0.01f } },
	{ .pos={ .x= 0.4f, .y=-0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.99f, .v=0.01f } },
	{ .pos={ .x= 0.4f, .y= 0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.99f, .v=0.99f } },

	{ .pos={ .x=-0.4f, .y=-0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.01f, .v=0.01f } },
	{ .pos={ .x= 0.4f, .y= 0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.99f, .v=0.99f } },
	{ .pos={ .x=-0.4f, .y= 0.5f, .z=0.0f }, .normal={ .x=0.0f, .y=0.0f, .z=1.0f }, .tex={ .u=0.01f, .v=0.99f } },
};

static u32 num_items;
static struct item_params item_instances[MAX_ITEMS];

static const char *item_vert_shader_src = SHADER_SRC(
	uniform vec2 glyph_tex_size;

	uniform mat4 projection_matrix;
	uniform vec3 ambient_light;
	uniform vec3 directional_light;
	uniform vec3 light_direction;

	layout (location = 0) in vec3 vertex_pos;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 tex_in;
	layout (location = 3) in vec3 color_in;
	layout (location = 4) in vec3 center_pos;
	layout (location = 5) in uint character;

	out vec2 tex_coord;
	out vec3 color;

	void main() {
		vec2 glyph_loc = vec2(character % 32u, 7u - (character / 32u));
		tex_coord = (tex_in + glyph_loc) * glyph_tex_size;
		// tex_coord = tex_in;
		color = color_in;
		// color = color_in * (ambient_light + dot(-light_direction, normal) * directional_light);
		vec3 pos = vertex_pos + center_pos;
		gl_Position = projection_matrix * vec4(pos, 1.0f);
		// gl_Position = vec4((center_pos.xy + vertex_pos.xy) * 0.1f, 0.0f, 1.0f);
	}
);

static const char *item_frag_shader_src = SHADER_SRC(
	uniform sampler2D font_tex;

	layout (location = 0) out vec4 color_out;

	in vec3 color;
	in vec2 tex_coord;

	void main() {
		// color_out = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		vec4 color_sample = texture(font_tex, tex_coord);
		if (color_sample.a == 0.0f) {
			discard;
		}
		color_out = vec4(color_sample.rgb * color, 1.0f);
		// color_out = vec4(tex_coord.xy, 0.0f, 1.0f);
	}
);

static i32 init_item(void) {
	// TODO -- error checking
	item_vert_shader = compile_shader(GL_VERTEX_SHADER,   item_vert_shader_src);
	item_frag_shader = compile_shader(GL_FRAGMENT_SHADER, item_frag_shader_src);
	item_program = glCreateProgram();
	glAttachShader(item_program, item_vert_shader);
	glAttachShader(item_program, item_frag_shader);
	item_program = link_program(item_program);

	item_glyph_tex_size_loc = glGetUniformLocation(item_program, "glyph_tex_size");
	item_font_tex_loc       = glGetUniformLocation(item_program, "font_tex");
	item_proj_mat_loc       = glGetUniformLocation(item_program, "projection_matrix");
	item_ambient_loc        = glGetUniformLocation(item_program, "ambient_light");
	item_directional_loc    = glGetUniformLocation(item_program, "directional_light");
	item_light_dir_loc      = glGetUniformLocation(item_program, "light_direction");

	glUseProgram(item_program);
	glUniform2f(item_glyph_tex_size_loc,
		((f32)FONT_GLYPH_WIDTH)  / ((f32)FONT_TEX_WIDTH),
		((f32)FONT_GLYPH_HEIGHT) / ((f32)FONT_TEX_HEIGHT));
	glUniform1i(item_font_tex_loc, FONT_TEX_LOC);

	glGenVertexArrays(1, &item_vao);
	glBindVertexArray(item_vao);

	glGenBuffers(1, &item_static_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, item_static_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(item_static_vertices), &item_static_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct item_static_vertex), (GLvoid*)offsetof(struct item_static_vertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct item_static_vertex), (GLvoid*)offsetof(struct item_static_vertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
		sizeof(struct item_static_vertex), (GLvoid*)offsetof(struct item_static_vertex, tex));
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &item_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, item_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(item_instances), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct item_params), (GLvoid*)offsetof(struct item_params, r));
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
		sizeof(struct item_params), (GLvoid*)offsetof(struct item_params, x));
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(4);

	glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE,
		sizeof(struct item_params), (GLvoid*)offsetof(struct item_params, character));
	glVertexAttribDivisor(5, 1);
	glEnableVertexAttribArray(5);

	glBindVertexArray(0);

	return 0;
}

static void free_item(void) {
	glDeleteProgram(item_program);
	glDeleteShader(item_vert_shader);
	glDeleteShader(item_frag_shader);

	// glDeleteVertexArrays...
	glDeleteBuffers(1, &item_static_buffer);
	glDeleteBuffers(1, &item_buffer);
}

void reset_items(void) {
	num_items = 0;
}

void add_item(struct item_params params) {
	assert(num_items < MAX_ITEMS);
	item_instances[num_items++] = params;
}
static void set_item_proj_mat(mat4 m) {
	glUseProgram(item_program);
	glUniformMatrix4fv(item_proj_mat_loc, 1, GL_TRUE, m.elems);
}

static void set_item_ambient_light(f32 r, f32 g, f32 b) {
	glUseProgram(item_program);
	glUniform3f(item_ambient_loc, r, g, b);
}

static void set_item_directional_light(f32 r, f32 g, f32 b) {
	glUseProgram(item_program);
	glUniform3f(item_directional_loc, r, g, b);
}

static void set_item_light_direction(f32 x, f32 y, f32 z) {
	glUseProgram(item_program);
	glUniform3f(item_light_dir_loc, x, y, z);
}

void draw_items(void) {
	glBindBuffer(GL_ARRAY_BUFFER, item_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_items * sizeof(struct item_params), item_instances);
	glUseProgram(item_program);
	glBindVertexArray(item_vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, ARRAY_LENGTH(item_static_vertices), num_items);
}

// =============================================================================
// textures
// =============================================================================

static GLuint cp437_tex;

struct __attribute__((__packed__)) texture {
	u16 width, height;
	u32 pixels[0];
};

static GLuint load_texture(char *filename, GLenum texture_unit) {
	// TODO -- error checking
	char *base_path = SDL_GetBasePath();
	u32 len = strlen(base_path) + strlen(filename) + 1;
	char *full_filename = malloc(len * sizeof(char));
	full_filename[0] = 0;
	strcat(full_filename, base_path);
	strcat(full_filename, filename);
	SDL_free(base_path);
	// SDL_Log("Loading texture: '%s'", full_filename);
	FILE *fp = fopen(full_filename, "rb");
	free(full_filename);
	fseek(fp, 0, SEEK_END);
	i32 filesize = ftell(fp);
	struct texture *tex_data = malloc(filesize);
	fseek(fp, 0, SEEK_SET);
	fread(tex_data, filesize, 1, fp);
	// SDL_Log("Texture size: (%hu x %hu)", tex_data->width, tex_data->height);
	fclose(fp);
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_data->width, tex_data->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, tex_data->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	free(tex_data);
	return texture;
}

static i32 load_textures(void) {
	// TODO -- error checking
	cp437_tex = load_texture("cp437.bin", FONT_TEX_LOC);
	return 0;
}

static void free_textures(void) {
	glDeleteTextures(1, &cp437_tex);
}

// =============================================================================
//
// =============================================================================

static void set_proj_mat(mat4 m) {
	set_cube_proj_mat(m);
	set_item_proj_mat(m);
}

static void set_ambient_light(f32 r, f32 g, f32 b) {
	set_cube_ambient_light(r, g, b);
	set_item_ambient_light(r, g, b);
}

static void set_directional_light(f32 r, f32 g, f32 b) {
	set_cube_directional_light(r, g, b);
	set_item_directional_light(r, g, b);
}

static void set_light_direction(f32 x, f32 y, f32 z) {
	set_cube_light_direction(x, y, z);
	set_item_light_direction(x, y, z);
}

void set_camera(struct camera_params params) {
	struct { f32 x, y, z; } camera_pos, look_at;
	camera_pos.x = params.camera_pos.x;
	camera_pos.y = params.camera_pos.y;
	camera_pos.z = params.camera_pos.z;
	look_at.x = params.look_at.x;
	look_at.y = params.look_at.y;
	look_at.z = params.look_at.z;

	mat4 perspective = MAT4(
		1.0f, 0.0f,  0.0f, 0.0f,
		0.0f, 1.0f,  0.0f, 0.0f,
		0.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 0.05f, 0.0f,
	);
	mat4 scaling_matrix = MAT4(
		0.2f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.2f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.001f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	);
	mat4 screen_matrix = MAT4(
		(f32)MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / (f32)(SCREEN_WIDTH), 0.0f, 0.0f, 0.0f,
		0.0f, (f32)MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / (f32)(SCREEN_HEIGHT), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	);
	mat4 camera_to_origin = MAT4(
		1.0f, 0.0f, 0.0f, -camera_pos.x,
		0.0f, 1.0f, 0.0f, -camera_pos.y,
		0.0f, 0.0f, 1.0f, -camera_pos.z,
		0.0f, 0.0f, 0.0f, 1.0f,
	);
	f32 horiz_rot = atan2(look_at.x - camera_pos.x, look_at.z - camera_pos.z);
	// SDL_Log("%f", horiz_rot);
	mat4 horiz_rot_mat = MAT4(
		cos(horiz_rot), 0.0f, -sin(horiz_rot), 0.0f,
		          0.0f, 1.0f,            0.0f, 0.0f,
		sin(horiz_rot), 0.0f,  cos(horiz_rot), 0.0f,
		          0.0f, 0.0f,            0.0f, 1.0f,
	);
	f32 look_at_dist = sqrt(
		(look_at.x - camera_pos.x)*(look_at.x - camera_pos.x) +
		(look_at.z - camera_pos.z)*(look_at.z - camera_pos.z));
	f32 vert_rot = atan2(look_at.y - camera_pos.y, look_at_dist);
	// SDL_Log("%f", vert_rot);
	mat4 vert_rot_mat = MAT4(
		1.0f,          0.0f,           0.0f, 0.0f,
		0.0f, cos(vert_rot), -sin(vert_rot), 0.0f,
		0.0f, sin(vert_rot),  cos(vert_rot), 0.0f,
		0.0f,          0.0f,           0.0f, 1.0f,
	);
	mat4 proj_mat = camera_to_origin;
	// print_matrix(proj_mat);
	proj_mat = mat_mul(horiz_rot_mat, proj_mat);
	// print_matrix(proj_mat);
	proj_mat = mat_mul(vert_rot_mat, proj_mat);
	// print_matrix(proj_mat);
	proj_mat = mat_mul(perspective, proj_mat);
	// print_matrix(proj_mat);
	proj_mat = mat_mul(screen_matrix, proj_mat);
	// print_matrix(proj_mat);
	proj_mat = mat_mul(scaling_matrix, proj_mat);
	// print_matrix(proj_mat);
	set_proj_mat(proj_mat);
}

i32 init_opengl(void) {
	// TODO -- error checking
	load_textures();
	init_fade();
	init_cube();
	init_font();
	init_item();

	glEnable(GL_CULL_FACE);
	glClearDepth(-1.0f);
	glDepthFunc(GL_GREATER);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// TODO -- set this elsewhere
	set_ambient_light(0.5f, 0.5f, 0.5f);
	set_directional_light(0.5f, 0.5f, 0.5f);
	set_light_direction(-0.1f, -0.7f, 1.0f - sqrt(0.1f*0.1f + 0.7f*0.7f));

	return 0;
}

void quit_opengl(void) {
	free_fade();
	free_cube();
	free_font();
	free_item();
	free_textures();
}

void draw_world(void) {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	draw_cubes();
	draw_items();
	glDisable(GL_DEPTH_TEST);
}
