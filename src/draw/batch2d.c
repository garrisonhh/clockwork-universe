#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ghh/array.h>
#include <ghh/utils.h>
#include <ghh/hashmap.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "batch2d.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/atlas.h"

// this module for 2d sprite batching
// TODO would it be possible to generalize something like this?

enum BATCH_VBOS {
	VBO_DRAWPOS,
	VBO_DRAWSIZE,
	VBO_ATLASPOS,
	VBO_ATLASSIZE,

	NUM_BATCH_VBOS
};

typedef struct batch_packet {
	atlas_ref_t *ref;
	vec2 pos;
} batch_packet_t;

atlas_t atlas;
shader_t *batch_shader = NULL;
GLuint batch_vao;
GLuint batch_vbos[NUM_BATCH_VBOS];
array_t *batch = NULL; // batch_packet_t *

void batch2d_init(int batch_array_size) {
	// init vars
	batch_shader = shader_create();

	shader_attach(batch_shader, "res/shaders/batch2d_vert.glsl", SHADER_VERTEX);
	shader_attach(batch_shader, "res/shaders/batch_frag.glsl", SHADER_FRAGMENT);
	shader_compile(batch_shader);

	GL(glGenVertexArrays(1, &batch_vao));
	GL(glGenBuffers(NUM_BATCH_VBOS, batch_vbos));

	batch = array_create(batch_array_size);

	// load
	atlas_construct(&atlas);
	atlas_add_sheet(&atlas, "font", "res/fonts/CGA8x8thick.png", (vec2){8.0, 8.0});
	atlas_generate(&atlas);
}

void batch2d_quit() {
	atlas_destruct(&atlas);

	GL(glDeleteBuffers(NUM_BATCH_VBOS, batch_vbos));
	GL(glDeleteVertexArrays(1, &batch_vao));

	shader_destroy(batch_shader);
	array_destroy(batch, false);
}

void batch2d_queue(int ref_idx, vec2 pos) {
	batch_packet_t *packet = malloc(sizeof(*packet));

	packet->ref = atlas.refs + ref_idx;
	glm_vec2_copy(pos, packet->pos);

	array_push(batch, packet);
}

void batch2d_queue_text(int font_idx, vec2 pos, const char *text) {
	int ref_idx;
	vec2 carriage = {0.0, 0.0}, text_pos;

	while (*text) {
        ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage[0] = 0.0;
    		carriage[1] += atlas.refs[ref_idx].pixel_size[1] + 1;
        } else {
            glm_vec2_add(pos, carriage, text_pos);
    		batch2d_queue(ref_idx, text_pos);

    		carriage[0] += atlas.refs[ref_idx].pixel_size[0] + 1;
        }

        ++text;
	}
}

void batch2d_draw() {
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

	texture_bind(atlas.texture, 0);
	GL(glUniform1i(shader_uniform_location(batch_shader, "atlas"), 0));

	// process batch data
	batch_packet_t *packet;
	size_t batch_count = array_size(batch);
	vec2 arrays[NUM_BATCH_VBOS][batch_count];

	for (i = 0; i < batch_count; ++i) {
		packet = array_get(batch, i);

		glm_vec2_copy(packet->pos, arrays[VBO_DRAWPOS][i]);
		glm_vec2_copy(packet->ref->pixel_size, arrays[VBO_DRAWSIZE][i]);
		glm_vec2_copy(packet->ref->pos, arrays[VBO_ATLASPOS][i]);
		glm_vec2_copy(packet->ref->size, arrays[VBO_ATLASSIZE][i]);
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

int batch2d_get_ref(const char *name) {
    int *ref;

    if ((ref = hashmap_get(atlas.ref_map, name)) == NULL)
        ERROR("could not find batch ref under name \"%s\".", name);

    return *ref;
}
