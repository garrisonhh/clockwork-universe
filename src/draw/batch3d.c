#include <ghh/memcheck.h>

#include "batch3d.h"
#include "../gfx/batcher.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/atlas.h"

#define VBO_DEFS() \
    X(VBO_DRAWPOS, 3)\
    X(VBO_DRAWSIZE, 2)\
    X(VBO_DRAWOFFSET, 2)\
    X(VBO_ATLASPOS, 2)\
    X(VBO_ATLASSIZE, 2)\

enum BATCH3D_VBOS {
#define X(a, b) a,
    VBO_DEFS()
#undef X
    NUM_BATCH3D_VBOS
};

const int VBO_ITEMS[NUM_BATCH3D_VBOS] = {
#define X(a, b) b,
    VBO_DEFS()
#undef X
};

atlas_t atlas3d;
batcher_t batcher3d;

void batch3d_init(int batch_array_size) {
	batcher_construct(&batcher3d, GL_TRIANGLE_STRIP, 4);

	shader_attach(batcher3d.shader, "res/shaders/batch3d_vert.glsl", SHADER_VERTEX);
	shader_attach(batcher3d.shader, "res/shaders/batch_frag.glsl", SHADER_FRAGMENT);
	shader_compile(batcher3d.shader);

	for (size_t i = 0; i < NUM_BATCH3D_VBOS; ++i)
		batcher_add_buffer(&batcher3d, VBO_ITEMS[i]);

	// load
	atlas_construct(&atlas3d);
	atlas_add_texture(&atlas3d, "testblock", "res/blocks/TESTBLOCK.png");
	atlas_generate(&atlas3d);
}

void batch3d_quit() {
	atlas_destruct(&atlas3d);
	batcher_destruct(&batcher3d);
}

void batch3d_queue(int ref_idx, vec3 pos, vec2 offset) {
	atlas_ref_t *ref = &atlas3d.refs[ref_idx];

	batcher_queue_attr(&batcher3d, VBO_DRAWPOS, pos);
    batcher_queue_attr(&batcher3d, VBO_DRAWSIZE, ref->pixel_size);
	batcher_queue_attr(&batcher3d, VBO_DRAWOFFSET, offset);
	batcher_queue_attr(&batcher3d, VBO_ATLASPOS, ref->pos);
	batcher_queue_attr(&batcher3d, VBO_ATLASSIZE, ref->size);
}

void batch3d_draw(int scale) {
	vec2 disp_size, camera;

	shader_bind(batcher3d.shader);

	// pass in uniforms
	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	GL(glUniform2fv(shader_uniform_location(batcher3d.shader, "camera"), 1, camera));
    GL(glUniform2fv(shader_uniform_location(batcher3d.shader, "screen_size"), 1, disp_size));
	GL(glUniform1f(shader_uniform_location(batcher3d.shader, "scale"), (float)scale));

	texture_bind(atlas3d.texture, 0);
	GL(glUniform1i(shader_uniform_location(batcher3d.shader, "atlas3d"), 0));

	// draw
	batcher_draw(&batcher3d);
}

int batch3d_get_ref(const char *name) {
	return *(int *)hashmap_get(atlas3d.ref_map, name);
}
