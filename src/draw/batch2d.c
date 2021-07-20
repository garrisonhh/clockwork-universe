#include <ghh/memcheck.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ghh/array.h>
#include <ghh/utils.h>
#include <ghh/hashmap.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "batch2d.h"
#include "../gfx/batcher.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/atlas.h"

enum BATCH_VBOS {
	VBO_DRAWPOS,
	VBO_DRAWSIZE,
	VBO_ATLASPOS,
	VBO_ATLASSIZE,

	NUM_BATCH_VBOS
};

atlas_t atlas;
batcher_t batcher;

void batch2d_init(int batch_array_size) {
	batcher_construct(&batcher);

	shader_attach(batcher.shader, "res/shaders/batch2d_vert.glsl", SHADER_VERTEX);
	shader_attach(batcher.shader, "res/shaders/batch_frag.glsl", SHADER_FRAGMENT);
	shader_compile(batcher.shader);

	for (size_t i = 0; i < NUM_BATCH_VBOS; ++i)
		batcher_add_buffer(&batcher, 2);

	// load
	atlas_construct(&atlas);
	atlas_add_sheet(&atlas, "font", "res/fonts/CGA8x8thick.png", (vec2){8.0, 8.0});
	atlas_generate(&atlas);
}

void batch2d_quit() {
	atlas_destruct(&atlas);
	batcher_destruct(&batcher);
}

void batch2d_queue(int ref_idx, vec2 pos) {
	atlas_ref_t *ref = &atlas.refs[ref_idx];

	batcher_queue_attr(&batcher, VBO_DRAWPOS, pos);
	batcher_queue_attr(&batcher, VBO_DRAWSIZE, ref->pixel_size);
	batcher_queue_attr(&batcher, VBO_ATLASPOS, ref->pos);
	batcher_queue_attr(&batcher, VBO_ATLASSIZE, ref->size);
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
	vec2 disp_size, camera;

	shader_bind(batcher.shader);

	// pass in uniforms
	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	GL(glUniform2f(
		shader_uniform_location(batcher.shader, "camera"),
		camera[0], camera[1]
	));
	GL(glUniform2f(
        shader_uniform_location(batcher.shader, "screen_size"),
        disp_size[0], disp_size[1]
    ));

	texture_bind(atlas.texture, 0);
	GL(glUniform1i(shader_uniform_location(batcher.shader, "atlas"), 0));

	// draw
	batcher_draw(&batcher);
}

int batch2d_get_ref(const char *name) {
	return *(int *)hashmap_get(atlas.ref_map, name);
}
