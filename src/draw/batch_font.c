#include <string.h>
#include <ghh/array.h>
#include <ghh/vector.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "batch_font.h"
#include <ghhgfx/batcher.h>
#include <ghhgfx/gfx.h>
#include <ghhgfx/shader.h>
#include <ghhgfx/atlas.h>

font_attrs_t default_attrs = {
    .color = {{1.0, 1.0, 1.0, 1.0}},
    .italicize = 0.0,
    .scale = 1.0,
    .waviness = 0.0,
};

atlas_t font_atlas;
shader_t *font_shader;
batcher_t *font_batcher;

void batch_font_init(int batch_array_size) {
    const size_t buffers[] = {3, 2, 2, 2, 4, 1, 1, 1};
    const size_t num_buffers = ARRAY_LEN(buffers);

    font_batcher = batcher_create(buffers, num_buffers);

    font_shader = shader_create(
        .vert = "res/shaders/batch_font_vert.glsl",
        .frag = "res/shaders/batch_font_frag.glsl"
    );

	atlas_construct(&font_atlas);
	atlas_add_sheet(&font_atlas, "font", "res/fonts/CGA8x8thick.png", v2_fill(8.0));
	atlas_generate(&font_atlas);
}

void batch_font_quit() {
	atlas_destruct(&font_atlas);
	batcher_destroy(font_batcher);
    shader_destroy(font_shader);
}

// internal only
void batch_font_queue_lower(int ref_idx, v2 pos, font_attrs_t *attrs) {
	atlas_ref_t *ref = &font_atlas.refs[ref_idx];

    float *data[] = {
        pos.ptr,
        ref->pixel_size.ptr,
        ref->pos.ptr,
        ref->size.ptr,
        attrs->color.ptr,
        &attrs->italicize,
        &attrs->scale,
        &attrs->waviness
    };

    batcher_queue(font_batcher, data);
}

void batch_font_queue(int font_idx, v2 pos, const char *text, font_attrs_t *attrs) {
    v2 carriage = v2_ZERO;
	int ref_idx;

    if (attrs == NULL)
        attrs = &default_attrs;

    while (*text) {
        ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage.x = 0.0;
            carriage.y += (font_atlas.refs[ref_idx].pixel_size.y + 1.0) * attrs->scale;
        } else {
            batch_font_queue_lower(ref_idx, v2_add(pos, carriage), attrs);

            carriage.x += (font_atlas.refs[ref_idx].pixel_size.x + 1.0) * attrs->scale;
        }

        ++text;
    }
}

void batch_font_draw() {
    shader_bind(font_shader);

	// pass in uniforms
	GL(glUniform2fv(
        shader_uniform_location(font_shader, "camera"),
        1, gfx_get_camera().ptr
    ));
	GL(glUniform2fv(
        shader_uniform_location(font_shader, "screen_size"),
        1, gfx_get_size().ptr
    ));

	GL(glUniform1f(
        shader_uniform_location(font_shader, "t"),
        fmod(timeit_get_time(), 1000000.0)
    ));

	texture_bind(font_atlas.texture, 0);
	GL(glUniform1i(shader_uniform_location(font_shader, "font_atlas"), 0));

	// draw
	batcher_draw(font_batcher, GL_TRIANGLE_STRIP, 4);
}

int batch_font_get_ref(const char *name) {
	return *(int *)hashmap_get(font_atlas.ref_map, name);
}
