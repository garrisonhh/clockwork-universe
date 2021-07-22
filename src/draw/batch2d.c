#include <ghh/memcheck.h>

#include "batch2d.h"
#include "../gfx/batcher.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/atlas.h"

enum BATCH2D_VBOS {
	VBO_DRAWPOS,
	VBO_DRAWSIZE,
	VBO_ATLASPOS,
	VBO_ATLASSIZE,

	NUM_BATCH_VBOS
};

atlas_t atlas2d;
batcher_t batcher2d;

void batch2d_init(int batch_array_size) {
	batcher_construct(&batcher2d, GL_TRIANGLE_STRIP, 4);

	shader_attach(batcher2d.shader, "res/shaders/batch2d_vert.glsl", SHADER_VERTEX);
	shader_attach(batcher2d.shader, "res/shaders/atlas_frag.glsl", SHADER_FRAGMENT);
	shader_compile(batcher2d.shader);

	for (size_t i = 0; i < NUM_BATCH_VBOS; ++i)
		batcher_add_buffer(&batcher2d, 2);

	// loadatlas2d
	atlas_construct(&atlas2d);
	atlas_add_sheet(&atlas2d, "font", "res/fonts/CGA8x8thick.png", (vec2){8.0, 8.0});
	atlas_generate(&atlas2d);
}

void batch2d_quit() {
	atlas_destruct(&atlas2d);
	batcher_destruct(&batcher2d);
}

void batch2d_queue(int ref_idx, vec2 pos) {
	atlas_ref_t *ref = &atlas2d.refs[ref_idx];

	batcher_queue_attr(&batcher2d, VBO_DRAWPOS, pos);
	batcher_queue_attr(&batcher2d, VBO_DRAWSIZE, ref->pixel_size);
	batcher_queue_attr(&batcher2d, VBO_ATLASPOS, ref->pos);
	batcher_queue_attr(&batcher2d, VBO_ATLASSIZE, ref->size);
}

void batch2d_queue_text(int font_idx, vec2 pos, const char *text) {
	int ref_idx;
	vec2 carriage = {0.0, 0.0}, text_pos;

	while (*text) {
        ref_idx = font_idx + *text;

        if (*text == '\n') {
            carriage[0] = 0.0;
    		carriage[1] += atlas2d.refs[ref_idx].pixel_size[1] + 1;
        } else {
            glm_vec2_add(pos, carriage, text_pos);
    		batch2d_queue(ref_idx, text_pos);

    		carriage[0] += atlas2d.refs[ref_idx].pixel_size[0] + 1;
        }

        ++text;
	}
}

void batch2d_draw() {
	vec2 disp_size, camera;

	shader_bind(batcher2d.shader);

	// pass in uniforms
	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	GL(glUniform2fv(shader_uniform_location(batcher2d.shader, "camera"), 1, camera));
	GL(glUniform2fv(shader_uniform_location(batcher2d.shader, "screen_size"), 1, disp_size));

	texture_bind(atlas2d.texture, 0);
	GL(glUniform1i(shader_uniform_location(batcher2d.shader, "atlas2d"), 0));

	// draw
	batcher_draw(&batcher2d);
}

int batch2d_get_ref(const char *name) {
	return *(int *)hashmap_get(atlas2d.ref_map, name);
}
