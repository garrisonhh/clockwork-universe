#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ghh/array.h>
#include <ghh/utils.h>
#include <ghh/hashmap.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "batch.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"

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

// batch shader vars
shader_t *batch_shader = NULL;
GLuint batch_vao;
GLuint batch_vbos[NUM_BATCH_VBOS];
array_t *batch = NULL; // batch_packet_t *

// atlas vars
texture_t *atlas = NULL;
vec2 atlas_size;
atlas_ref_t *atlas_refs = NULL;
size_t num_atlas_refs = 0;
hashmap_t *atlas_ref_map = NULL;

void batch_init(int batch_array_size) {
	// shader
	batch_shader = shader_create();

	shader_attach(batch_shader, "res/shaders/batch.vs", SHADER_VERTEX);
	shader_attach(batch_shader, "res/shaders/batch.fs", SHADER_FRAGMENT);
	shader_compile(batch_shader);

	GL(glGenVertexArrays(1, &batch_vao));
	GL(glGenBuffers(NUM_BATCH_VBOS, batch_vbos));

	// batching
	batch = array_create(batch_array_size);

    // atlas
    atlas_ref_map = hashmap_create(0, -1, true);

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

void batch_quit() {
	GL(glDeleteBuffers(NUM_BATCH_VBOS, batch_vbos));
	GL(glDeleteVertexArrays(1, &batch_vao));

	shader_destroy(batch_shader);
	array_destroy(batch, false);
    hashmap_destroy(atlas_ref_map, true);

	free(atlas_refs);
}

int atlas_bin_compare(const void *a, const void *b) {
	atlas_packet_t *bina, *binb;

	bina = *(atlas_packet_t **)a;
	binb = *(atlas_packet_t **)b;

	return MAX(bina->pos[0], bina->pos[1]) - MAX(binb->pos[0], binb->pos[1]);
}

int batch_atlas_add_lower(texture_t *texture, bool sheet, vec2 cell_size) {
	if (atlas_ctx.final)
		ERROR0("cannot add to batch atlas after it has been generated.\n");

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

void map_ref(const char *name, int ref) {
    int *ref_ptr = malloc(sizeof(*ref_ptr));

    *ref_ptr = ref;

    hashmap_set(atlas_ref_map, (char *)name, ref_ptr);
}

int batch_atlas_add_texture(const char *name, const char *filename) {
	int ref = batch_atlas_add_lower(texture_create(filename), false, GLM_VEC2_ZERO);
    map_ref(name, ref);

    return ref;
}

int batch_atlas_add_sheet(const char *name, const char *filename, vec2 cell_size) {
	int ref = batch_atlas_add_lower(texture_create(filename), true, cell_size);
    map_ref(name, ref);

    return ref;
}

void batch_atlas_generate() {
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
    texture_fbo_generate(atlas);
    gfx_bind_target(atlas);

	for (i = 0; i < array_size(atlas_ctx.textures); ++i) {
		tex = array_get(atlas_ctx.textures, i);
        packet = array_get(atlas_ctx.packets, i);

        texture_fbo_generate(tex);
        gfx_blit(tex, packet->pos, packet->size);
        texture_fbo_delete(tex);
	}

    gfx_unbind_target();
    texture_fbo_delete(atlas);

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

void batch_queue(int ref_idx, vec2 pos) {
	batch_packet_t *packet = malloc(sizeof(*packet));

	packet->ref = atlas_refs + ref_idx;
	glm_vec2_copy(pos, packet->pos);

	array_push(batch, packet);
}

void batch_queue_text(int font_idx, vec2 pos, const char *text) {
	int ref_idx;
	vec2 carriage = {0.0, 0.0}, text_pos;

	while (*text) {
        ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage[0] = 0.0;
    		carriage[1] += atlas_refs[ref_idx].pixel_size[1] + 1;
        } else {
            glm_vec2_add(pos, carriage, text_pos);
    		batch_queue(ref_idx, text_pos);

    		carriage[0] += atlas_refs[ref_idx].pixel_size[0] + 1;
        }

        ++text;
	}
}

void batch_draw() {
	size_t i;
	vec2 disp_size, camera;

	shader_bind(batch_shader);

	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	GL(glUniform2f(shader_uniform_location(batch_shader, "camera"), camera[0], camera[1]));
	GL(glUniform2f(
        shader_uniform_location(batch_shader, "screen_size"),
        disp_size[0], disp_size[1]
    ));

	texture_bind(atlas, 0);
	GL(glUniform1i(shader_uniform_location(batch_shader, "atlas"), 0));

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
	GL(glBindVertexArray(batch_vao));

	for (i = 0; i < NUM_BATCH_VBOS; ++i) {
		GL(glBindBuffer(GL_ARRAY_BUFFER, batch_vbos[i]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(arrays[i]), arrays[i], GL_STREAM_DRAW));

		GL(glEnableVertexAttribArray(i));
		GL(glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, 0, 0));
		GL(glVertexAttribDivisor(i, 1));
	}

	// draw
	GL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, batch_count));

	// clean up
	for (i = 0; i < NUM_BATCH_VBOS; ++i)
		glDisableVertexAttribArray(i);

	glBindVertexArray(0);
}

int batch_get_ref(const char *name) {
    int *ref;

    if ((ref = hashmap_get(atlas_ref_map, name)) == NULL)
        ERROR("could not find batch ref under name \"%s\".", name);

    return *ref;
}
