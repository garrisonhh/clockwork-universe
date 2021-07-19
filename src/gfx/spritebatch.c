#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ghh/array.h>
#include <ghh/utils.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "spritebatch.h"
#include "gfx.h"
#include "shader.h"

enum BATCH_VBOS {
	VBO_DRAWPOS,
	VBO_DRAWSIZE,
	VBO_ATLASPOS,
	VBO_ATLASSIZE,

	NUM_BATCH_VBOS
};

// used for pixel sizes pre-atlas-generation
typedef struct atlas_packet {
	vec2 pos, size;

	bool sheet;
	vec2 sheet_size, cell_size; // for sheet, if sheet
} atlas_packet_t;

typedef struct batch_packet {
	atlas_ref_t *ref;
	vec2 pos;
} batch_packet_t;

// used to construct atlas
struct atlas_context {
	array_t *textures;
	array_t *packets;
	array_t *bins; // TODO use linked list for this
	int index;
	bool final;
} atlas_ctx;

shader_t *batch_shader = NULL;
GLuint batch_vao;
GLuint batch_vbos[NUM_BATCH_VBOS];
array_t *batch = NULL; // batch_packet_t *

texture_t *atlas = NULL;
vec2 atlas_size;
atlas_ref_t *atlas_refs = NULL;
size_t num_atlas_refs = 0;

void spritebatch_init(int batch_array_size) {
	// shader
	batch_shader = shader_create();

	shader_attach(batch_shader, "res/shaders/spritebatch.vs", SHADER_VERTEX);
	shader_attach(batch_shader, "res/shaders/spritebatch.fs", SHADER_FRAGMENT);
	shader_compile(batch_shader);

	glGenVertexArrays(1, &batch_vao);
	glGenBuffers(NUM_BATCH_VBOS, batch_vbos);

	// batching
	batch = array_create(batch_array_size);

	// atlas_ctx
	atlas_ctx.textures = array_create(0);
	atlas_ctx.packets = array_create(0);
	atlas_ctx.bins = array_create(0);
	atlas_ctx.index = 0;
	atlas_ctx.final = false;

	atlas_packet_t *base_bin = malloc(sizeof(*base_bin));

	*base_bin = (atlas_packet_t){
		.pos = {0, 0},
		.size = {INFINITY, INFINITY}
	};

	array_push(atlas_ctx.bins, base_bin);
}

void spritebatch_quit() {
	glDeleteBuffers(NUM_BATCH_VBOS, batch_vbos);
	glDeleteVertexArrays(1, &batch_vao);

	shader_destroy(batch_shader);
	array_destroy(batch, false);

	free(atlas_refs);
}

int atlas_bin_compare(const void *a, const void *b) {
	atlas_packet_t *bina, *binb;

	bina = *(atlas_packet_t **)a;
	binb = *(atlas_packet_t **)b;

	return MAX(bina->pos[0], bina->pos[1]) - MAX(binb->pos[0], binb->pos[1]);
}

int spritebatch_atlas_add_lower(texture_t *texture, bool sheet, vec2 cell_size) {
	if (atlas_ctx.final)
		ERROR0("cannot add to spritebatch atlas after it has been generated.\n");

	int this_index = atlas_ctx.index;
	atlas_packet_t *bin = NULL;
	atlas_packet_t *packet = malloc(sizeof(*packet));
	atlas_packet_t *new_bins[2];

	// find best bin
	for (int i = 0; i < array_size(atlas_ctx.bins); ++i) {
		bin = array_get(atlas_ctx.bins, i);

		if (bin->size[0] >= texture->w && bin->size[1] >= texture->h) {
			bin = array_get(atlas_ctx.bins, i);
			array_del(atlas_ctx.bins, i);
			break;
		}
	}

	// store new packet
	*packet = (atlas_packet_t){
		.pos = {bin->pos[0], bin->pos[1]},
		.size = {texture->w, texture->h},
		.sheet = sheet,
		.cell_size = {cell_size[0], cell_size[1]}
	};

	array_push(atlas_ctx.packets, packet);
	array_push(atlas_ctx.textures, texture);

	// increment index
	if (sheet) {
		glm_vec2_div(packet->size, packet->cell_size, packet->sheet_size);

		glm_vec2_copy(
			(vec2){(int)packet->sheet_size[0], (int)packet->sheet_size[1]},
			packet->sheet_size
		);

		atlas_ctx.index += packet->sheet_size[0] * packet->sheet_size[1];
	} else {
		++atlas_ctx.index;
	}

	// create new bins
	for (int i = 0; i < 2; ++i)
		new_bins[i] = malloc(sizeof(*new_bins[i]));

	if (bin->size[0] < bin->size[1]) {
		*new_bins[0] = (atlas_packet_t){
			.pos = {bin->pos[0] + packet->size[0], bin->pos[1]},
			.size = {bin->size[0] - packet->size[0], packet->size[1]}
		};
		*new_bins[1] = (atlas_packet_t){
			.pos = {bin->pos[0], bin->pos[1] + packet->size[1]},
			.size = {bin->size[0], bin->size[1] - packet->size[1]}
		};
	} else {
		*new_bins[0] = (atlas_packet_t){
			.pos = {bin->pos[0], bin->pos[1] + packet->size[1]},
			.size = {packet->size[0], bin->size[1] - packet->size[1]}
		};
		*new_bins[1] = (atlas_packet_t){
			.pos = {bin->pos[0] + packet->size[0], bin->pos[1]},
			.size = {bin->size[0] - packet->size[0], bin->size[1]}
		};
	}

	free(bin);

	// add new bins back to bins and sort
	for (int i = 0; i < 2; ++i)
		if (new_bins[i]->size[0] && new_bins[i]->size[1])
			array_push(atlas_ctx.bins, new_bins[i]);

	array_qsort(atlas_ctx.bins, atlas_bin_compare);

	return this_index;
}

int spritebatch_atlas_add_texture(const char *filename) {
	return spritebatch_atlas_add_lower(texture_create(filename), false, GLM_VEC2_ZERO);
}

int spritebatch_atlas_add_sheet(const char *filename, vec2 cell_size) {
	return spritebatch_atlas_add_lower(texture_create(filename), true, cell_size);
}

/*
int spritebatch_atlas_add_font(const char *image_path, const char *json_path) {
	size_t i;
	int font, sheet_stride;
	cJSON *json;
	array_t *arr;
	vec2 sheet_pos, cell_size;
	atlas_packet_t *packet;

	json = json_create(json_path);

	// load to atlas
	font = spritebatch_atlas_add_texture(image_path);

	// get cell size
	arr = json_get_array(json, "cell_size");

	for (i = 0; i < 2; ++i)
		cell_size[i] = json_to_number(array_get(arr, i));

	array_destroy(arr, false);

	// create packets
	arr = json_get_array(json, "char_widths");
	glm_vec2_copy(((atlas_packet_t *)array_peek(atlas_ctx.packets))->pos, sheet_pos);
	sheet_stride = (double)((texture_t *)array_peek(atlas_ctx.textures))->w / cell_size[0];

	for (i = 1; i < array_size(arr); ++i) {
		packet = malloc(sizeof(*packet));

		*packet = (atlas_packet_t){
			.pos = {
				sheet_pos[0] + (double)(i % sheet_stride) * cell_size[0],
				sheet_pos[1] + (double)(i / sheet_stride) * cell_size[1]
			},
			.size = {json_to_number(array_get(arr, i)), cell_size[1]},
			.sheet = false
		};

		array_push(atlas_ctx.packets, packet);
		++atlas_ctx.index;
	}

	array_destroy(arr, true);

	printf("added font\n");

	return font;
}
*/

void spritebatch_atlas_generate() {
	int i, x, y;
	int idx;
	int atlas_w = 0, atlas_h = 0;
	GLuint atlas_fbo, tex_fbo;
	texture_t *tex;
	atlas_packet_t *packet;

	// find size of atlas
	for (i = 0; i < array_size(atlas_ctx.packets); ++i) {
		packet = array_get(atlas_ctx.packets, i);

		if (packet->pos[0] + packet->size[0] > atlas_w)
			atlas_w = packet->pos[0] + packet->size[0];
		if (packet->pos[1] + packet->size[1] > atlas_h)
			atlas_h = packet->pos[1] + packet->size[1];
	}

	atlas = texture_create_empty(atlas_w, atlas_h);
	atlas_size[0] = atlas->w;
	atlas_size[1] = atlas->h;

	// blit all textures
	glGenFramebuffers(1, &atlas_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, atlas_fbo);

	glFramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		atlas->texture,
		0
	);

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR0("draw framebuffer error\n");

	glGenFramebuffers(1, &tex_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, tex_fbo);

	for (i = 0; i < array_size(atlas_ctx.textures); ++i) {
		tex = array_get(atlas_ctx.textures, i);
		packet = array_get(atlas_ctx.packets, i);

		glFramebufferTexture2D(
			GL_READ_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			tex->texture,
			0
		);

		if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("read framebuffer error\n");
			exit(-1);
		}

		// params get cast to int
		glBlitFramebuffer(
			0, 0, tex->w, tex->h,
			packet->pos[0], packet->pos[1],
			packet->pos[0] + packet->size[0], packet->pos[1] + packet->size[1],
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);

		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("framebuffer blit error\n");
			exit(-1);
		}
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &atlas_fbo);
	glDeleteFramebuffers(1, &tex_fbo);

	// store references
	num_atlas_refs = atlas_ctx.index;
	atlas_refs = malloc(num_atlas_refs * sizeof(*atlas_refs));

	idx = 0;

	for (i = 0; i < array_size(atlas_ctx.packets); ++i) {
		packet = array_get(atlas_ctx.packets, i);

		if (packet->sheet) {
			for (y = 0; y < packet->sheet_size[1]; ++y) {
				for (x = 0; x < packet->sheet_size[0]; ++x) {
					glm_vec2_copy(packet->cell_size, atlas_refs[idx].pixel_size);
					glm_vec2_div(packet->cell_size, atlas_size, atlas_refs[idx].atlas_size);

					glm_vec2_mul((vec2){x, y}, packet->cell_size, atlas_refs[idx].atlas_pos);
					glm_vec2_add(atlas_refs[idx].atlas_pos, packet->pos, atlas_refs[idx].atlas_pos);
					glm_vec2_div(atlas_refs[idx].atlas_pos, atlas_size, atlas_refs[idx].atlas_pos);

					++idx;
				}
			}
		} else {
			glm_vec2_copy(packet->size, atlas_refs[idx].pixel_size);
			glm_vec2_div(packet->pos, atlas_size, atlas_refs[idx].atlas_pos);
			glm_vec2_div(packet->size, atlas_size, atlas_refs[idx].atlas_size);
			++idx;
		}
	}

	// clean up ctx
	for (i = 0; i < array_size(atlas_ctx.textures); ++i)
		texture_destroy(array_get(atlas_ctx.textures, i));

	array_destroy(atlas_ctx.textures, false);
	array_destroy(atlas_ctx.packets, true);
	array_destroy(atlas_ctx.bins, true);
	atlas_ctx.textures = NULL;
	atlas_ctx.packets = NULL;
	atlas_ctx.bins = NULL;

	atlas_ctx.final = true;
}

void spritebatch_queue(int ref_idx, vec2 pos) {
	batch_packet_t *packet = malloc(sizeof(*packet));

	packet->ref = atlas_refs + ref_idx;
	glm_vec2_copy(pos, packet->pos);

	array_push(batch, packet);
}

void spritebatch_queue_text(int font_idx, vec2 pos, const char *text) {
	int ref_idx;
	double carriage = 0;

	while (*text) {
		ref_idx = font_idx + *text;
		spritebatch_queue(ref_idx, (vec2){pos[0] + carriage, pos[1]});

		carriage += atlas_refs[ref_idx].pixel_size[0] + 1;
		++text;
	}
}

void spritebatch_draw() {
	size_t i;

	// pass in uniforms (TODO do I need to do this every frame?)
	GLint loc;
	vec2 disp_size, camera;

	shader_bind(batch_shader);

	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	glUniform2f(shader_uniform_location(batch_shader, "camera"), camera[0], camera[1]);
	glUniform2f(shader_uniform_location(batch_shader, "screen_size"), disp_size[0], disp_size[1]);

	if ((loc = shader_uniform_location(batch_shader, "time")) >= 0)
		glUniform1f(loc, SDL_GetTicks());

	texture_bind(atlas, 0);
	glUniform1i(shader_uniform_location(batch_shader, "atlas"), 0);

	// process batch data
	batch_packet_t *packet;

	size_t batch_count = array_size(batch);
	vec2 arrays[NUM_BATCH_VBOS][batch_count];

	for (i = 0; i < batch_count; ++i) {
		packet = array_get(batch, i);

		glm_vec2_copy(packet->pos, arrays[VBO_DRAWPOS][i]);
		glm_vec2_copy(packet->ref->pixel_size, arrays[VBO_DRAWSIZE][i]);
		glm_vec2_copy(packet->ref->atlas_pos, arrays[VBO_ATLASPOS][i]);
		glm_vec2_copy(packet->ref->atlas_size, arrays[VBO_ATLASSIZE][i]);
	}

	array_clear(batch, true);

	// buffer batch data
	glBindVertexArray(batch_vao);

	for (i = 0; i < NUM_BATCH_VBOS; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, batch_vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(arrays[i]), arrays[i], GL_STREAM_DRAW);

		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(i, 1);
	}

	// draw
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, batch_count);

	// clean up
	for (i = 0; i < NUM_BATCH_VBOS; ++i)
		glDisableVertexAttribArray(i);

	glBindVertexArray(0);
}

atlas_ref_t *spritebatch_get_ref(int ref_idx) {
	return atlas_refs + ref_idx;
}