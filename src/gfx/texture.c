#include <ghh/memcheck.h>
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

// unit means GL_TEXTURE0 .. 31
void texture_bind(texture_t *texture, int unit) {
	if (unit > 31)
		ERROR("whoops, texture unit over 31.\n");

	GL(glActiveTexture(GL_TEXTURE0 + unit));
	GL(glBindTexture(GL_TEXTURE_2D, texture->texture));
}

void texture_fbo_generate(texture_t *texture) {
	GL(glGenFramebuffers(1, &texture->fbo));
	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, texture->fbo));
	GL(glFramebufferTexture2D(
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		texture->texture,
		0
	));
	GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
}

void texture_fbo_delete(texture_t *texture) {
	GL(glDeleteFramebuffers(1, &texture->fbo));
}
