#ifndef TEXTURE_H
#define TEXTURE_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdbool.h>

typedef struct ghh_texture {
	GLuint texture, fbo;
	int w, h;
} texture_t;

texture_t *texture_create(const char *filename);
texture_t *texture_create_empty(int w, int h);
void texture_destroy(texture_t *);

void texture_bind(texture_t *, int unit);

// framebuffer stuff
void texture_fbo_generate(texture_t *);
void texture_fbo_delete(texture_t *);

//void texture_fbo_blit(texture_t *dest, texture_t *src, vec2 pos);

#endif
