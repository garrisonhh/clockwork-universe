#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cglm/cglm.h>
#include <ghh/utils.h>
#include <stdio.h>

#include "gfx.h"
#include "shader.h"

SDL_Window *window;
SDL_GLContext *gl_ctx;

int width, height;
float aspect;
vec2 camera, bounds;
vec2 min_margin, max_margin;
bool fbo_bound;

void gfx_init(const char *name, int width, int height) {
	// SDL and OpenGL init
	window = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL)
		ERROR("couldn't create window:\n%s\n", SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	gl_ctx = SDL_GL_CreateContext(window);

	if (gl_ctx == NULL)
		ERROR("couldn't create gl context:\n%s\n", SDL_GetError());

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	 	ERROR0("couldn't load opengl.\n");

	// OpenGL config
	// first try adaptive vsync, then normal vsync, then forget about it
	if (SDL_GL_SetSwapInterval(-1))
		SDL_GL_SetSwapInterval(1);

	GL(glEnable(GL_BLEND));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// gfx vars init
	fbo_bound = false;
	gfx_on_resize(); // detects vars based on gfx size
	glm_vec2_copy((vec2){0.0, 0.0}, camera);
}

void gfx_quit() {
	GL(SDL_GL_DeleteContext(gl_ctx));
	SDL_DestroyWindow(window);
}

void gfx_bind_render_target(texture_t *texture) {
	const GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};

	GL(glBindFramebuffer(GL_FRAMEBUFFER, texture->fbo));

	if (!glIsFramebuffer(texture->fbo))
		ERROR0("must generate texture fbo before binding as render target.\n");

	GL(glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		texture->texture,
		0
	));

	GL(glDrawBuffers(1, draw_buffers));

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("render target binding error.\n");

	fbo_bound = true;

	gfx_set_size((vec2){texture->w, texture->h});
}

void gfx_unbind_render_target(void) {
	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	fbo_bound = false;

	// TODO less stupid way to return to old gfx size
	gfx_on_resize();
}

void gfx_draw_texture(texture_t *texture) {
	if (!glIsFramebuffer(texture->fbo))
		ERROR0("can't blit unbuffered textures.\n");

	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, texture->fbo));
	GL(glFramebufferTexture2D(
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		texture->texture,
		0
	));

	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("blit src framebuffer error\n");

	GL(glBlitFramebuffer(
		0.0, 0.0, texture->w, texture->h,
		0.0, 0.0, width, height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	));

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("blitting framebuffer error\n");

	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
}

void gfx_clear(float r, float g, float b, float a) {
	GL(glClearColor(r, g, b, a));
	GL(glClear(GL_COLOR_BUFFER_BIT));
}

void gfx_flip() {
	SDL_GL_SwapWindow(window);
}

void gfx_on_resize() {
	if (!fbo_bound) {
		int w, h;

		SDL_GetWindowSize(window, &w, &h);

		gfx_set_size((vec2){w, h});
	}
}

void gfx_set_size(vec2 size) {
	width = size[0];
	height = size[1];

	aspect = (float)width / (float)height;
	GL(glViewport(0, 0, width, height));

	glm_vec2_scale((vec2){width, height}, 0.5, min_margin);
	glm_vec2_sub(bounds, min_margin, max_margin);
}

void gfx_set_camera(vec2 camera) {
	for (int i = 0; i < 2; ++i) {
		GL(camera[i] = glm_clamp(
			camera[i],
			min_margin[i],
			max_margin[i]
		));
	}
}

void gfx_set_bounds(vec2 bounds) {
	GL(glm_vec2_copy(bounds, bounds));

	gfx_on_resize();
}

bool gfx_get_fbo_bound() {
	return fbo_bound;
}

float gfx_get_aspect() {
	return aspect;
}

void gfx_get_size(vec2 out_size) {
 	out_size[0] = width;
	out_size[1] = height;
}

void gfx_get_camera(vec2 out_camera) {
 	glm_vec2_copy(camera, out_camera);
}
