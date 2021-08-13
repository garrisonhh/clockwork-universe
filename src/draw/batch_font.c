#include <ghh/array.h>
#include <ghh/hashmap.h>
#include <ghh/vector.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "batch_font.h"
#include <ghhgfx/batcher.h>
#include <ghhgfx/gfx.h>
#include <ghhgfx/shader.h>
#include <ghhgfx/atlas.h>

static font_attrs_t default_attrs = {
    .color = {{1.0, 1.0, 1.0, 1.0}},
    .italicize = 0.0,
    .scale = 1.0,
    .waviness = 0.0,
};

static struct {
    atlas_t atlas;
    shader_t *shader;
    batcher_t *batcher;
} font;

void batch_font_init() {
    const size_t buffers[] = {3, 2, 2, 2, 4, 1, 1, 1};
    const size_t num_buffers = ARRAY_LEN(buffers);

    font.batcher = batcher_create(buffers, num_buffers);

    font.shader = shader_create(
        .vert = "res/shaders/batch_font_vert.glsl",
        .frag = "res/shaders/batch_font_frag.glsl"
    );

	atlas_construct(&font.atlas);
	atlas_add_sheet(&font.atlas, "font", "res/fonts/CGA8x8thick.png", v2_fill(8.0));
	atlas_generate(&font.atlas);
}

void batch_font_quit() {
	atlas_destruct(&font.atlas);
	batcher_destroy(font.batcher);
    shader_destroy(font.shader);
}

static void queue_lower(int ref_idx, v2 pos, font_attrs_t *attrs) {
	atlas_ref_t *ref = &font.atlas.refs[ref_idx];

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

    batcher_queue(font.batcher, data);
}

void batch_font_queue(int font_idx, v2 pos, const char *text, font_attrs_t *attrs) {
    v2 carriage = v2_ZERO;

    if (attrs == NULL)
        attrs = &default_attrs;

    while (*text) {
        int ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage.x = 0.0;
            carriage.y += (font.atlas.refs[ref_idx].pixel_size.y + 1.0) * attrs->scale;
        } else {
            queue_lower(ref_idx, v2_add(pos, carriage), attrs);

            carriage.x += (font.atlas.refs[ref_idx].pixel_size.x + 1.0) * attrs->scale;
        }

        ++text;
    }
}

void batch_font_draw() {
    shader_bind(font.shader);

	// pass in uniforms
	GL(glUniform2fv(
        shader_uniform_location(font.shader, "camera"),
        1, gfx_get_camera().ptr
    ));
	GL(glUniform2fv(
        shader_uniform_location(font.shader, "screen_size"),
        1, gfx_get_size().ptr
    ));

	GL(glUniform1f(
        shader_uniform_location(font.shader, "t"),
        fmod(timeit_get_time(), 1000000.0)
    ));

	texture_bind(font.atlas.texture, 0);
	GL(glUniform1i(shader_uniform_location(font.shader, "font.atlas"), 0));

	// draw
	batcher_draw(font.batcher, GL_TRIANGLE_STRIP, 4);
}

int batch_font_get_ref(const char *name) {
	return *(int *)hashmap_get(font.atlas.ref_map, name);
}
