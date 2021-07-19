#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <stddef.h>

typedef enum shader_types {
	SHADER_VERTEX,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT,

	NUM_SHADER_TYPES
} shader_types_e;

typedef struct shader shader_t;

// to use:
// 1. create shader
// 2. attach shader source files
// 3. compile shader

shader_t *shader_create(void);
void shader_destroy(shader_t *);

void shader_attach(shader_t *shader, const char *filename, shader_types_e type);
void shader_compile(shader_t *);

void shader_bind(shader_t *);

// returns -1 on failure
GLint shader_uniform_location(shader_t *, const char *var);

#endif
