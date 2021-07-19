#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ghh/utils.h>
#include <ghh/io.h>

#include "shader.h"

// maps to shader_types_e enum
GLenum GL_SHADER_TYPES[NUM_SHADER_TYPES] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER
};

struct shader {
	GLuint program;
	GLuint shaders[NUM_SHADER_TYPES];
	unsigned attached: NUM_SHADER_TYPES;
};

void check_shader(GLuint shader, GLuint flags, bool is_program, const char *msg);
GLuint load_shader(const char *filename, GLenum shader_type);

shader_t *shader_create() {
	shader_t *shader = malloc(sizeof(*shader));

	shader->program = glCreateProgram();
	shader->attached = 0;

	return shader;
}

void shader_destroy(shader_t *shader) {
	for (int i = 0; i < NUM_SHADER_TYPES; ++i) {
		if (BIT_GET(shader->attached, i)) {
			glDetachShader(shader->program, shader->shaders[i]);
			glDeleteShader(shader->shaders[i]);
		}
	}

	glDeleteProgram(shader->program);
	free(shader);
}

void shader_attach(shader_t *shader, const char *filename, shader_types_e type) {
	shader->shaders[type] = load_shader(filename, GL_SHADER_TYPES[type]);
	BIT_SET_TRUE(shader->attached, type);
}

void shader_compile(shader_t *shader) {
	if (!shader->attached)
		ERROR0("no shader sources attached to shader, cannot compile.\n");

	for (int i = 0; i < NUM_SHADER_TYPES; ++i)
		if (BIT_GET(shader->attached, i))
			glAttachShader(shader->program, shader->shaders[i]);

	glLinkProgram(shader->program);
	check_shader(shader->program, GL_LINK_STATUS, true, "program linking failed");

	glValidateProgram(shader->program);
	check_shader(shader->program, GL_VALIDATE_STATUS, true, "program validation failed");
}

GLint shader_uniform_location(shader_t *shader, const char *var) {
	return glGetUniformLocation(shader->program, var);
}

void shader_bind(shader_t *shader) {
	glUseProgram(shader->program);

#ifdef DEBUG
	check_shader(shader->program, GL_VALIDATE_STATUS, true, "shader validation failed on bind");
#endif
}

GLuint load_shader(const char *filename, GLenum shader_type) {
	GLuint shader = glCreateShader(shader_type);
	char *text = load_text_file(filename);

	const int num_sources = 1;
	const GLchar *shader_src[num_sources];
	GLint shader_src_lengths[num_sources];

	if (shader == 0)
		ERROR0("shader creation failed.\n");

	shader_src[0] = text;
	shader_src_lengths[0] = strlen(text);

	glShaderSource(shader, num_sources, shader_src, shader_src_lengths);
	glCompileShader(shader);

	check_shader(shader, GL_COMPILE_STATUS, false, "shader compilation failed");

	free(text);

	return shader;
}

void check_shader(GLuint handle, GLuint flags, bool is_program, const char *msg) {
	GLint success = 0;
	GLchar error[1024] = "";

	if (is_program)
		glGetProgramiv(handle, flags, &success);
	else
		glGetShaderiv(handle, flags, &success);

	if (!success) {
		if (is_program)
			glGetProgramInfoLog(handle, sizeof(error), NULL, error);
		else
			glGetShaderInfoLog(handle, sizeof(error), NULL, error);

		ERROR("%s:\n%s\n", msg, error);
	}
}
