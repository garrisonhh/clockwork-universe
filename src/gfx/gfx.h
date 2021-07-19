#ifndef GFX_H
#define GFX_H

#include <cglm/cglm.h>
#include <stdbool.h>
#include <ghh/utils.h>
#include "texture.h"

#ifdef DEBUG
#define GL_DO(line) do {line;if(glGetError() != GL_NO_ERROR) ERROR0("OpenGL error!")} while(0)
#define GL(line) do {gl##line;if(glGetError() != GL_NO_ERROR) ERROR0("OpenGL error!")} while(0)
#else
#define GL_DO(line) line
#define GL(line) gl##line
#endif

/*
yes, the java-style getters and setters are annoying, but this interface has a LOT of gotchas
under the hood, so it makes sense

in order to work properly, you need to:
- call init once
- set bounds before set_camera and when level sizes change
- set_camera when camera position changes
*/

void gfx_init(const char *name, int width, int height);
void gfx_quit(void);

void gfx_bind_render_target(texture_t *);
void gfx_unbind_render_target(void);
void gfx_draw_texture(texture_t *);
void gfx_clear(float r, float g, float b, float a);
void gfx_flip(void);
void gfx_on_resize(void);

void gfx_set_size(vec2 size);
void gfx_set_camera(vec2 camera);
void gfx_set_bounds(vec2 bounds);

bool gfx_get_fbo_bound(void);
float gfx_get_aspect(void);
void gfx_get_size(vec2 out_size);
void gfx_get_camera(vec2 out_camera);

#endif
