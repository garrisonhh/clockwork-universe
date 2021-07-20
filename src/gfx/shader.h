#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <stddef.h>

#define SHADER_TYPES() \
	X(SHADER_VERTEX, GL_VERTEX_SHADER)\
	X(SHADER_GEOMETRY, GL_GEOMETRY_SHADER)\
	X(SHADER_FRAGMENT, GL_FRAGMENT_SHADER)

typedef enum shader_types {
#define	X(type, gl_type) type,
	SHADER_TYPES()
#undef X

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
