#include <ghh/memcheck.h>
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
vec2 camera;
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
	 	ERROR("couldn't load opengl.\n");

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

void gfx_bind_target(texture_t *texture) {
	const GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};

	GL(glBindFramebuffer(GL_FRAMEBUFFER, texture->fbo));
	GL(glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		texture->texture,
		0
	));

	GL(glDrawBuffers(1, draw_buffers));

	fbo_bound = true;

	gfx_set_size((vec2){texture->w, texture->h});
}

void gfx_unbind_target(void) {
	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	fbo_bound = false;

	// TODO less stupid way to return to old gfx size
	gfx_on_resize();
}

void gfx_blit(texture_t *texture, float *in_pos, float *in_size) {
	vec2 pos = {0.0, 0.0}, size = {texture->w, texture->h};

	if (in_pos != NULL)
		glm_vec2_copy(in_pos, pos);
	if (in_size != NULL)
		glm_vec2_copy(in_size, size);

	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, texture->fbo));
	GL(glFramebufferTexture2D(
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		texture->texture,
		0
	));
	GL(glBlitFramebuffer(
		0.0, 0.0, texture->w, texture->h,
		pos[0], pos[1], pos[0] + size[0], pos[1] + size[1],
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	));
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
}

void gfx_set_camera(vec2 in_camera) {
	glm_vec2_copy(in_camera, camera);
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
