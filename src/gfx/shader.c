#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <glad/glad.h>
#include <ghh/io.h>
#include <ghh/array.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "gfx.h"
#include "shader.h"

#define SHADER_TYPES() \
	X(SHADER_VERTEX, GL_VERTEX_SHADER)\
	X(SHADER_GEOMETRY, GL_GEOMETRY_SHADER)\
	X(SHADER_FRAGMENT, GL_FRAGMENT_SHADER)

typedef enum shader_types {
#define X(a, b) a,
	SHADER_TYPES()
#undef X
	NUM_SHADER_TYPES
} shader_types_e;

static GLenum GL_SHADER_TYPES[] = {
#define X(a, b) b,
	SHADER_TYPES()
#undef X
};

struct shader {
	GLuint shaders[NUM_SHADER_TYPES];
	GLuint program;
	unsigned attached: NUM_SHADER_TYPES;
};

static GLuint load_shader(const char *filename, GLenum shader_type);
static void check_shader(GLuint handle, GLuint flags, bool is_program, const char *msg);
static void attach_source(shader_t *shader, const char *filename, shader_types_e type);
static void compile_program(shader_t *shader);

shader_t *shader_create_lower(shader_params_t params) {
	shader_t *shader = malloc(sizeof(*shader));
	const char **sources = (const char **)&params;

	GL(shader->program = glCreateProgram());
	shader->attached = 0;

	for (size_t i = 0; i < NUM_SHADER_TYPES; ++i)
		if (sources[i] != NULL)
			attach_source(shader, sources[i], i);

	compile_program(shader);

	return shader;
}

void shader_destroy(shader_t *shader) {
	for (int i = 0; i < NUM_SHADER_TYPES; ++i) {
		if (BIT_GET(shader->attached, i)) {
			GL(glDetachShader(shader->program, shader->shaders[i]));
			GL(glDeleteShader(shader->shaders[i]));
		}
	}

	GL(glDeleteProgram(shader->program));
	free(shader);
}

static void attach_source(shader_t *shader, const char *filename, shader_types_e type) {
	shader->shaders[type] = load_shader(filename, GL_SHADER_TYPES[type]);
	BIT_SET_TRUE(shader->attached, type);
}

static void compile_program(shader_t *shader) {
	for (int i = 0; i < NUM_SHADER_TYPES; ++i)
		if (BIT_GET(shader->attached, i))
			GL(glAttachShader(shader->program, shader->shaders[i]));

	glLinkProgram(shader->program);
	check_shader(shader->program, GL_LINK_STATUS, true, "program linking failed");

	glValidateProgram(shader->program);
	check_shader(shader->program, GL_VALIDATE_STATUS, true, "program validation failed");
}

static GLuint load_shader(const char *filename, GLenum shader_type) {
	GLuint shader;
	const char * const text = load_text_file(filename);
	GLint source_length = strlen(text);

	GL(shader = glCreateShader(shader_type));

	GL(glShaderSource(shader, 1, &text, &source_length));

	glCompileShader(shader);
	check_shader(shader, GL_COMPILE_STATUS, false, "shader compilation failed");

	free((char *)text);

	return shader;
}

static void check_shader(GLuint handle, GLuint flags, bool is_program, const char *msg) {
	GLint success = 0;
	GLchar error[1024] = "";

	if (is_program)
		GL(glGetProgramiv(handle, flags, &success));
	else
		GL(glGetShaderiv(handle, flags, &success));

	if (!success) {
		if (is_program)
			GL(glGetProgramInfoLog(handle, sizeof(error), NULL, error));
		else
			GL(glGetShaderInfoLog(handle, sizeof(error), NULL, error));

		fprintf(stderr, "%s:\n%s\n", msg, error);
		exit(-1);
	}
}

void shader_bind(shader_t *shader) {
	GL(glUseProgram(shader->program));
}

int shader_uniform_location(shader_t *shader, const char *name) {
	return glGetUniformLocation(shader->program, name);
}
