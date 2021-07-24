#include <string.h>
#include <ghh/array.h>
#include <ghh/vector.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "batch_font.h"
#include "../gfx/batcher.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/atlas.h"

#define VBO_DEFS() \
    X(VBO_DRAWPOS, 3)\
    X(VBO_DRAWSIZE, 2)\
    X(VBO_ATLASPOS, 2)\
    X(VBO_ATLASSIZE, 2)\
    X(VBO_COLOR, 4)\
    X(VBO_ITALICIZE, 1)\
    X(VBO_SCALE, 1)\
    X(VBO_WAVINESS, 1)

enum BATCH_FONT_VBOS {
#define X(a, b) a,
    VBO_DEFS()
#undef X
    NUM_BATCH_FONT_VBOS
};

const int BATCH_FONT_ITEMS[NUM_BATCH_FONT_VBOS] = {
#define X(a, b) b,
    VBO_DEFS()
#undef X
};

font_attrs_t default_attrs = {
    .color = {1.0, 1.0, 1.0, 1.0},
    .italicize = 0.0,
    .scale = 1.0,
    .waviness = 0.0,
};

atlas_t font_atlas;
batcher_t font_batcher;

void batch_font_init(int batch_array_size) {
	batcher_construct(&font_batcher, GL_TRIANGLE_STRIP, 4);

	shader_attach(font_batcher.shader, "res/shaders/batch_font_vert.glsl", SHADER_VERTEX);
	shader_attach(font_batcher.shader, "res/shaders/batch_font_frag.glsl", SHADER_FRAGMENT);
	shader_compile(font_batcher.shader);

	for (size_t i = 0; i < NUM_BATCH_FONT_VBOS; ++i)
		batcher_add_buffer(&font_batcher, BATCH_FONT_ITEMS[i]);

	// load font_atlas
	atlas_construct(&font_atlas);
	atlas_add_sheet(&font_atlas, "font", "res/fonts/CGA8x8thick.png", (vec2){8.0, 8.0});
	atlas_generate(&font_atlas);
}

void batch_font_quit() {
	atlas_destruct(&font_atlas);
	batcher_destruct(&font_batcher);
}

// internal only
void batch_font_queue_lower(int ref_idx, vec2 pos, font_attrs_t *attrs) {
	atlas_ref_t *ref = &font_atlas.refs[ref_idx];

    /*
	batcher_queue_attr(&font_batcher, VBO_DRAWPOS, pos);
	batcher_queue_attr(&font_batcher, VBO_DRAWSIZE, ref->pixel_size);
	batcher_queue_attr(&font_batcher, VBO_ATLASPOS, ref->pos);
	batcher_queue_attr(&font_batcher, VBO_ATLASSIZE, ref->size);

    batcher_queue_attr(&font_batcher, VBO_COLOR, (float *)attrs->color);
    batcher_queue_attr(&font_batcher, VBO_ITALICIZE, (float *)attrs->color);
    batcher_queue_attr(&font_batcher, VBO_SCALE, (float *)&attrs->scale);
    batcher_queue_attr(&font_batcher, VBO_WAVINESS, (float *)&attrs->waviness);
    */

    float *data[NUM_BATCH_FONT_VBOS] = {
        pos,
        ref->pixel_size,
        ref->pos,
        ref->size,
        attrs->color,
        &attrs->italicize,
        &attrs->scale,
        &attrs->waviness
    };

    batcher_queue(&font_batcher, data);
}

void batch_font_queue(int font_idx, vec2 pos, const char *text, font_attrs_t *attrs) {
    vec2 carriage = {0.0, 0.0}, text_pos;
	int ref_idx;

    if (attrs == NULL)
        attrs = &default_attrs;

    while (*text) {
        ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage[0] = 0.0;
            carriage[1] += (font_atlas.refs[ref_idx].pixel_size[1] + 1.0) * attrs->scale;
        } else {
            glm_vec2_add(pos, carriage, text_pos);
            batch_font_queue_lower(ref_idx, text_pos, attrs);

            carriage[0] += (font_atlas.refs[ref_idx].pixel_size[0] + 1.0) * attrs->scale;
        }

        ++text;
    }
}

void batch_font_draw() {
	vec2 disp_size, camera;
    float time;

    shader_bind(font_batcher.shader);

	// pass in uniforms
	gfx_get_camera(camera);
	gfx_get_size(disp_size);
    time = fmod(timeit_get_time(), 1000000.0);

	GL(glUniform2fv(shader_uniform_location(font_batcher.shader, "camera"), 1, camera));
	GL(glUniform2fv(
        shader_uniform_location(font_batcher.shader, "screen_size"),
        1, disp_size
    ));

	GL(glUniform1f(shader_uniform_location(font_batcher.shader, "t"), time));

	texture_bind(font_atlas.texture, 0);
	GL(glUniform1i(shader_uniform_location(font_batcher.shader, "font_atlas"), 0));

	// draw
	batcher_draw(&font_batcher);
}

int batch_font_get_ref(const char *name) {
	return *(int *)hashmap_get(font_atlas.ref_map, name);
}
