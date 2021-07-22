#ifndef BATCH2D_H
#define BATCH2D_H

#include <cglm/cglm.h>
#include <stddef.h>

#include "../gfx/texture.h"

void batch2d_init(int batch_array_size);
void batch2d_quit(void);

// pos is relative to range [-1.0, 1.0] screen x, y values
void batch2d_queue(int ref_idx, vec2 pos);
void batch2d_queue_text(int font_idx, vec2 pos, const char *text);
void batch2d_draw();

int batch2d_get_ref(const char *name);

#endif
