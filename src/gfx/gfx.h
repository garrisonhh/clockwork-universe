#ifndef GFX_H
#define GFX_H

#include <cglm/cglm.h>
#include <stdbool.h>
#include <ghh/utils.h>

#include "texture.h"

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

void gfx_clear(float r, float g, float b, float a);
void gfx_flip(void);
void gfx_on_resize(void);

bool gfx_get_fbo_bound(void);
float gfx_get_aspect(void);
void gfx_get_size(vec2 out_size);
void gfx_get_camera(vec2 out_camera);

void gfx_set_size(vec2 size);
void gfx_set_camera(vec2 camera);

void gfx_bind_target(texture_t *);
void gfx_unbind_target(void);
// draws onto current render target at vec2 destination pos and size
// if pos is NULL, defaults to (vec2){0.0, 0.0}
// if size is NULL, defaults to (vec2){texture->w, texture->h}
// TODO alpha blend on blit
void gfx_blit(texture_t *, float *pos, float *size);

// OpenGL debug macro
#ifdef DEBUG

static inline const char *gfx_gl_error(GLenum got_error) {
    switch (got_error) {
#define X(x) case x: return #x "\n"
    X(GL_INVALID_ENUM);
    X(GL_INVALID_VALUE);
    X(GL_INVALID_OPERATION);
    X(GL_INVALID_FRAMEBUFFER_OPERATION);
    X(GL_OUT_OF_MEMORY);
#undef X
    default: return "no error...? GL() is broken\n";
    }
}

#define GL(line) do {\
    line;\
    GLenum got_error;\
    if ((got_error = glGetError()) != GL_NO_ERROR)\
        ERROR(gfx_gl_error(got_error));\
} while(0)

#else

#define GL(line) line

#endif

#endif
