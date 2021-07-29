#include <ghh/memcheck.h>

#include "batch3d.h"
#include <ghhgfx/batcher.h>
#include <ghhgfx/gfx.h>
#include <ghhgfx/shader.h>
#include <ghhgfx/atlas.h>

atlas_t atlas3d;
shader_t *shader3d;
batcher_t *batcher3d;

void batch3d_init(int batch_array_size) {
    const size_t buffers[] = {3, 2, 2, 2, 2, 2, 2};
    const size_t num_buffers = ARRAY_LEN(buffers);

    batcher3d = batcher_create(buffers, num_buffers);

    shader3d = shader_create(
        .vert = "res/shaders/batch3d_vert.glsl",
        .frag = "res/shaders/batch3d_frag.glsl"
    );

	atlas_construct(&atlas3d);
    atlas_add_texture(&atlas3d, "blocktest", "res/blocks/BLOCKTEST.png");
    atlas_add_texture(&atlas3d, "blockdepth", "res/blocks/BLOCKDEPTH.png");
	atlas_add_texture(&atlas3d, "blocknormal", "res/blocks/BLOCKNORMAL.png");
	atlas_generate(&atlas3d);
}

void batch3d_quit() {
	atlas_destruct(&atlas3d);
    shader_destroy(shader3d);
	batcher_destroy(batcher3d);
}

void batch3d_queue(int ref_idx, int depth_idx, int normal_idx, vec3 pos, vec2 offset) {
    atlas_ref_t *ref = &atlas3d.refs[ref_idx];

    float *data[] = {
        pos,             // draw_pos
        ref->pixel_size, // draw_size
        offset,          // draw_offset
        ref->pos,        // tex_pos
        atlas3d.refs[depth_idx].pos,  // depth_tex_pos
        atlas3d.refs[normal_idx].pos, // normal_tex_pos
        ref->size        // tex_size
    };

    batcher_queue(batcher3d, data);
}

void batch3d_draw(int scale, float render_dist, vec3 light_pos) {
	vec2 disp_size, camera;

	shader_bind(shader3d);

	// pass in uniforms
	gfx_get_camera(camera);
	gfx_get_size(disp_size);

	GL(glUniform2fv(shader_uniform_location(shader3d, "camera"), 1, camera));
    GL(glUniform2fv(shader_uniform_location(shader3d, "screen_size"), 1, disp_size));
    GL(glUniform1f(shader_uniform_location(shader3d, "scale"), (float)scale));
	GL(glUniform1f(shader_uniform_location(shader3d, "render_dist"), render_dist));
    GL(glUniform3fv(shader_uniform_location(shader3d, "light_pos"), 1, light_pos));

	texture_bind(atlas3d.texture, 0);
	GL(glUniform1i(shader_uniform_location(shader3d, "atlas3d"), 0));

	// draw
	batcher_draw(batcher3d, GL_TRIANGLE_STRIP, 4);
}

int batch3d_get_ref(const char *name) {
	return *(int *)hashmap_get(atlas3d.ref_map, name);
}
