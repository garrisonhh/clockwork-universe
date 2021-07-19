#ifndef BATCH_H
#define BATCH_H

#include <cglm/cglm.h>
#include <stddef.h>

#include "../gfx/texture.h"

// atlas-texture-relative rectangle
typedef struct atlas_ref {
	vec2 atlas_pos, atlas_size;
	vec2 pixel_size;
} atlas_ref_t;

// to use:
// - add() textures into atlas, storing atlas_ref pointer index
// - generate() atlas (finalizes, no more adding to atlas)
// - for each frame:
//   - queue() refs to be drawn
//   - draw() to draw batch and flush queue

void batch_init(int batch_array_size);
void batch_quit(void);

int batch_atlas_add_texture(const char *name, const char *filename);
// generates all possible refs, returns index of first ref
int batch_atlas_add_sheet(const char *name, const char *filename, vec2 cell_size);
void batch_atlas_generate(void);

// pos is relative to range [-1.0, 1.0] screen x, y values
void batch_queue(int ref_idx, vec2 pos);
void batch_queue_text(int font_idx, vec2 pos, const char *text);
void batch_draw();

int batch_get_ref(const char *name);

#endif
