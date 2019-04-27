#pragma once

typedef u32  GLbitfield;
typedef u32  GLenum;
typedef u8   GLboolean;
typedef void GLvoid;
typedef i8   GLbyte;
typedef i16  GLshort;
typedef i32  GLint;
typedef u8   GLubyte;
typedef u16  GLushort;
typedef u32  GLuint;
typedef i32  GLsizei;
typedef f32  GLfloat;
typedef f32  GLclampf;
typedef f64  GLdouble;
typedef f64  GLclampd;
typedef char GLchar;

#ifdef _WIN64
typedef i64 GLsizeiptr;
typedef i64 GLintptr;
#else
typedef i32 GLsizeiptr;
typedef i32 GLintptr;
#endif

#define SHADER_SRC(...) "#version 330 core\n" #__VA_ARGS__

#define GL_FALSE 0
#define GL_TRUE  1

#define GL_VERTEX_SHADER    0x8B31
#define GL_GEOMETRY_SHADER  0x8DD9
#define GL_FRAGMENT_SHADER  0x8B30
#define GL_COMPILE_STATUS   0x8B81
#define GL_LINK_STATUS      0x8B82
#define GL_INFO_LOG_LENGTH  0x8B84

#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW          0x88E0
#define GL_STREAM_READ          0x88E1
#define GL_STREAM_COPY          0x88E2
#define GL_STATIC_DRAW          0x88E4
#define GL_STATIC_READ          0x88E5
#define GL_STATIC_COPY          0x88E6
#define GL_DYNAMIC_DRAW         0x88E8
#define GL_DYNAMIC_READ         0x88E9
#define GL_DYNAMIC_COPY         0x88EA

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406
#define GL_2_BYTES        0x1407
#define GL_3_BYTES        0x1408
#define GL_4_BYTES        0x1409
#define GL_DOUBLE         0x140A

#define GL_TRIANGLES 0x0004

#define GL_CULL_FACE  0x0B44
#define GL_DEPTH_TEST 0x0B71

#define GL_GREATER 0x0204

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_3_3_FUNCTIONS \
	/* begin function list */ \
	GL_FUNC(void,   glClear,            GLbitfield mask) \
	GL_FUNC(void,   glClearColor,       GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) \
	GL_FUNC(void,   glClearDepth,       GLclampd depth) \
	GL_FUNC(GLuint, glCreateShader,     GLenum shaderType) \
	GL_FUNC(void,   glDeleteShader,     GLuint shader) \
	GL_FUNC(void,   glShaderSource,     GLuint shader, GLsizei count, const GLchar **string, const GLint *length) \
	GL_FUNC(void,   glCompileShader,    GLuint shader) \
	GL_FUNC(void,   glGetShaderiv,      GLuint shader, GLenum pname, GLint *params) \
	GL_FUNC(void,   glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
	GL_FUNC(GLuint, glCreateProgram,    void) \
	GL_FUNC(GLuint, glDeleteProgram,    GLuint program) \
	GL_FUNC(void,   glAttachShader,     GLuint program, GLuint shader) \
	GL_FUNC(void,   glLinkProgram,      GLuint program) \
	GL_FUNC(void,   glGetProgramiv,     GLuint program, GLenum pname, GLint *params) \
	GL_FUNC(void,   glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
	GL_FUNC(void,   glUseProgram,       GLuint program) \
	GL_FUNC(GLint,  glGetUniformLocation, GLuint program, const GLchar *name) \
	GL_FUNC(void,   glUniform3f,        GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
	GL_FUNC(void,   glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	GL_FUNC(void,   glGenBuffers,       GLsizei n, GLuint *buffers) \
	GL_FUNC(void,   glDeleteBuffers,    GLsizei n, GLuint *buffers) \
	GL_FUNC(void,   glBindBuffer,       GLenum target, GLuint buffer) \
	GL_FUNC(void,   glBufferData,       GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) \
	GL_FUNC(void,   glBufferSubData,    GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) \
	GL_FUNC(void,   glGenVertexArrays,  GLsizei n, GLuint *arrays) \
	GL_FUNC(void,   glDeleteVertexArrays, GLsizei n, GLuint *arrays) \
	GL_FUNC(void,   glBindVertexArray,  GLuint array) \
	GL_FUNC(void,   glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, \
		GLsizei stride, const GLvoid *pointer) \
	GL_FUNC(void,   glVertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, \
		const GLvoid *pointer) \
	GL_FUNC(void,   glVertexAttribDivisor, GLuint index, GLuint divisor) \
	GL_FUNC(void,   glEnableVertexAttribArray, GLuint index) \
	GL_FUNC(void,   glEnable,           GLenum cap) \
	GL_FUNC(void,   glDisable,          GLenum cap) \
	GL_FUNC(void,   glDepthFunc,        GLenum func) \
	GL_FUNC(void,   glDrawArrays,       GLenum mode, GLint first, GLsizei count) \
	GL_FUNC(void,   glDrawElements,     GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) \
	GL_FUNC(void,   glDrawElementsInstanced, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, \
		GLsizei primcount) \
	/* end function list */

#define GL_FUNC(return_type, name, ...) \
	typedef return_type _##name(__VA_ARGS__); \
	extern _##name *name;
GL_3_3_FUNCTIONS
#undef GL_FUNC
