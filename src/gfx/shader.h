#ifndef SHADER_H
#define SHADER_H

#include <stddef.h>

typedef struct shader shader_t;

typedef struct shader_params {
	// order here has to match shader_types_e in shader.c
	const char *vert;
	const char *geom;
	const char *frag;
} shader_params_t;

// this macro mess just allows for default arguments
shader_t *shader_create_lower(shader_params_t);
#define shader_create(...) shader_create_lower((shader_params_t){\
	.vert = NULL,\
	.geom = NULL,\
	.frag = NULL,\
	__VA_ARGS__\
});
void shader_destroy(shader_t *);

void shader_bind(shader_t *);

int shader_uniform_location(shader_t *, const char *name);

#endif
