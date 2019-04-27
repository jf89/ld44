#include "opengl.h"

#include <SDL.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "gl_3_3.h"

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
// test shader
// =============================================================================

static GLuint test_buffer, test_vao;
static GLuint test_program, test_vert_shader, test_frag_shader;

struct test_vertex {
	f32 x, y;
};

static struct test_vertex test_vertices[] = {
	{ .x = -1.0f, .y = -1.0f },
	{ .x =  1.0f, .y = -1.0f },
	{ .x =  1.0f, .y =  1.0f },

	{ .x = -1.0f, .y = -1.0f },
	{ .x =  1.0f, .y =  1.0f },
	{ .x = -1.0f, .y =  1.0f },
};

static const char *test_vert_shader_src = SHADER_SRC(
	layout (location = 0) in vec2 in_pos;

	out vec2 pos;

	void main() {
		pos = in_pos;
		gl_Position = vec4(in_pos, 0.0f, 1.0f);
	}
);

static const char *test_frag_shader_src = SHADER_SRC(
	layout (location = 0) out vec4 color;

	in vec2 pos;

	void main() {
		color = vec4(pos, 0.0f, 1.0f);
	}
);

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
//
// =============================================================================

void set_camera(void) {
	struct { f32 x, y, z; } camera_pos, look_at;
	camera_pos.x =  0.0f;
	camera_pos.y =  10.0f;
	camera_pos.z = -10.0f;
	look_at.x = 0.0f;
	look_at.y = 0.0f;
	look_at.z = 5.0f;

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
	set_cube_proj_mat(proj_mat);
}

i32 init_opengl(void) {
	// TODO -- error checking
	test_vert_shader = compile_shader(GL_VERTEX_SHADER,   test_vert_shader_src);
	test_frag_shader = compile_shader(GL_FRAGMENT_SHADER, test_frag_shader_src);

	test_program = glCreateProgram();
	glAttachShader(test_program, test_vert_shader);
	glAttachShader(test_program, test_frag_shader);
	test_program = link_program(test_program);

	glGenVertexArrays(1, &test_vao);
	glBindVertexArray(test_vao);

	glGenBuffers(1, &test_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, test_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(test_vertices), &test_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// TODO -- error checking
	init_cube();

	glEnable(GL_CULL_FACE);
	glClearDepth(-1.0f);
	glDepthFunc(GL_GREATER);

	set_camera();
	set_cube_ambient_light(0.5f, 0.5f, 0.5f);
	set_cube_directional_light(0.5f, 0.5f, 0.5f);
	set_cube_light_direction(-0.1f, -0.7f, 1.0f - sqrt(0.1f*0.1f + 0.7f*0.7f));
	// TMP
	reset_cubes();
	for (i32 j = 0; j < 10; ++j) {
		for (i32 i = -5; i <= 5; ++i) {
			f32 x_offset = 0.1f * ((f32)rand()) / ((f32)RAND_MAX);
			f32 y_offset = 0.1f * ((f32)rand()) / ((f32)RAND_MAX);
			f32 z_offset = 0.1f * ((f32)rand()) / ((f32)RAND_MAX);
			add_cube((struct cube_params){
				.r = ((f32)j)/10.0f, .g = ((f32)i + 5)/11.0f, .b = 0.5f,
				.x = (f32)i + x_offset, .y = 0.0f + y_offset, .z = (f32)j + z_offset,
			});
		}
	}
	// TMP

	return 0;
}

void quit_opengl(void) {
	glDeleteProgram(test_program);
	glDeleteShader(test_vert_shader);
	glDeleteShader(test_frag_shader);

	glDeleteBuffers(1, &test_buffer);
	glDeleteVertexArrays(1, &test_vao);

	free_cube();
}

void test_draw(void) {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(test_program);
	glBindVertexArray(test_vao);
	glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(test_vertices));

	glEnable(GL_DEPTH_TEST);
	draw_cubes();
	glDisable(GL_DEPTH_TEST);
}
