#include "opengl.h"

#include <SDL.h>
#include <stdlib.h>

#include "gl_3_3.h"

// =============================================================================
// test shader
// =============================================================================

static GLuint test_buffer, test_vao;
static GLuint test_program, test_vert_shader, test_frag_shader;

struct test_vertex {
	f32 x, y;
};

struct test_vertex test_vertices[] = {
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
// =============================================================================

GLuint compile_shader(GLenum shader_type, const GLchar *shader_src) {
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

GLuint link_program(GLuint program) {
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(test_vertices), &test_vertices, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}

void quit_opengl(void) {
	glDeleteShader(test_vert_shader);
	glDeleteShader(test_frag_shader);
	glDeleteProgram(test_program);

	glDeleteBuffers(1, &test_buffer);
	glDeleteVertexArrays(1, &test_vao);
}

void test_draw(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(test_program);
	glBindVertexArray(test_vao);
	glDrawArrays(GL_TRIANGLES, 0, ARRAY_LENGTH(test_vertices));
}
