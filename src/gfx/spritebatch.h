#ifndef SPRITEBATCH_H
#define SPRITEBATCH_H

#include <cglm/cglm.h>
#include <stddef.h>

#include "texture.h"

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

void spritebatch_init(int batch_array_size);
void spritebatch_quit(void);

int spritebatch_atlas_add_texture(const char *filename);
// generates all possible refs, returns index of first ref
int spritebatch_atlas_add_sheet(const char *filename, vec2 cell_size);
int spritebatch_atlas_add_font(const char *image_path, const char *json_path);
void spritebatch_atlas_generate(void);

// pos is relative to range [-1.0, 1.0] screen x, y values
void spritebatch_queue(int ref_idx, vec2 pos);
void spritebatch_queue_text(int font_idx, vec2 pos, const char *text);
void spritebatch_draw();

atlas_ref_t *spritebatch_get_ref(int ref_idx);

#endif
