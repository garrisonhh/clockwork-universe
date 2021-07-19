#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ghh/utils.h>

#include "texture.h"
#include "gfx.h"

texture_t *texture_create(const char *filename) {
	texture_t *texture = malloc(sizeof(*texture));

	texture->fbo = 0;

	// load texture data, width, height
	uint8_t *image_data = stbi_load(
		filename,
		&texture->w, &texture->h,
		NULL, 4
	);

	if (image_data == NULL)
		ERROR("texture loading failed for texture \"%s\"\n", filename);

	GL(glGenTextures(1, &texture->texture));
	GL(glBindTexture(GL_TEXTURE_2D, texture->texture));

	GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL(glTexImage2D(
		GL_TEXTURE_2D, // type of texture
		0, // mip-mapping (0 = default)
		GL_RGBA, // internal format (how GL stores pixels on gpu)
		texture->w, texture->h, // size of texture
		0, // border (if true, adds 1 pixel margin)
		GL_RGBA, // input format of pixel data
		GL_UNSIGNED_BYTE, // type of pixel data
		image_data // pixel data
	));

	stbi_image_free(image_data);

	return texture;
}

texture_t *texture_create_empty(int w, int h) {
	texture_t *texture = malloc(sizeof(*texture));

	texture->w = w;
	texture->h = h;
	texture->fbo = 0;

	GL(glGenTextures(1, &texture->texture));
	GL(glBindTexture(GL_TEXTURE_2D, texture->texture));

	GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL(glTexImage2D(
		GL_TEXTURE_2D, // type of texture
		0, // mip-mapping (0 = default)
		GL_RGBA, // internal format (how GL stores pixels on gpu)
		texture->w, texture->h, // size of texture
		0, // border (if true, adds 1 pixel margin)
		GL_RGBA, // input format of pixel data
		GL_UNSIGNED_INT, // type of pixel data
		0 // pixel data (0 means no data)
	));

	return texture;
}

void texture_destroy(texture_t *texture) {
	texture_fbo_delete(texture);

	GL(glDeleteTextures(1, &texture->texture));
	free(texture);
}

// unit maens GL_TEXTURE0 .. 31
void texture_bind(texture_t *texture, int unit) {
	if (unit > 31)
		ERROR0("whoops, texture unit over 31.\n");

	GL(glActiveTexture(GL_TEXTURE0 + unit));
	GL(glBindTexture(GL_TEXTURE_2D, texture->texture));
}

void texture_fbo_generate(texture_t *texture) {
	GL(glGenFramebuffers(1, &texture->fbo));
}

void texture_fbo_delete(texture_t *texture) {
	GL(glDeleteFramebuffers(1, &texture->fbo));
}

void texture_fbo_blit(texture_t *dst, texture_t *src, vec2 pos) {
	GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->fbo));
	GL(glFramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		dst->texture,
		0
	));

	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, src->fbo));
	GL(glFramebufferTexture2D(
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		src->texture,
		0
	));

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("blit dst framebuffer failed.\n");

	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("blit src framebuffer failed.\n");

	GL(glBlitFramebuffer(
		0, 0, src->w, src->h,
		pos[0], pos[1],
		pos[0] + src->w, pos[1] + src->h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	));

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("texture blitting failed.n");

	GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
}
