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

// this just maps shader_types_e values to GLenum values
GLenum GL_SHADER_TYPES[NUM_SHADER_TYPES] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER
};

struct shader {
	GLuint shaders[NUM_SHADER_TYPES];
	GLuint program;
	unsigned attached: NUM_SHADER_TYPES;
};
void check_shader(GLuint shader, GLuint flags, bool is_program, const char *msg);
GLuint load_shader(const char *filename, GLenum shader_type);

shader_t *shader_create() {
	shader_t *shader = malloc(sizeof(*shader));

	GL(shader->program = glCreateProgram());
	shader->attached = 0;

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

void shader_attach(shader_t *shader, const char *filename, shader_types_e type) {
	shader->shaders[type] = load_shader(filename, GL_SHADER_TYPES[type]);
	BIT_SET_TRUE(shader->attached, type);
}

void shader_compile(shader_t *shader) {
	for (int i = 0; i < NUM_SHADER_TYPES; ++i)
		if (BIT_GET(shader->attached, i))
			GL(glAttachShader(shader->program, shader->shaders[i]));

	glLinkProgram(shader->program);
	check_shader(shader->program, GL_LINK_STATUS, true, "program linking failed");

	glValidateProgram(shader->program);
	check_shader(shader->program, GL_VALIDATE_STATUS, true, "program validation failed");
}

int shader_uniform_location(shader_t *shader, const char *var) {
	return glGetUniformLocation(shader->program, var);
}

void shader_bind(shader_t *shader) {
	GL(glUseProgram(shader->program));
}

GLuint load_shader(const char *filename, GLenum shader_type) {
	GLuint shader;
	char *text = load_text_file(filename);

	const int num_sources = 1;
	const GLchar *shader_src[num_sources];
	GLint shader_src_lengths[num_sources];

	GL(shader = glCreateShader(shader_type));

	shader_src[0] = text;
	shader_src_lengths[0] = strlen(text);

	GL(glShaderSource(shader, num_sources, shader_src, shader_src_lengths));

	glCompileShader(shader);
	check_shader(shader, GL_COMPILE_STATUS, false, "shader compilation failed");

	free(text);

	return shader;
}

void check_shader(GLuint handle, GLuint flags, bool is_program, const char *msg) {
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
