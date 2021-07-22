#ifndef BATCH3D_H
#define BATCH3D_H

#include <cglm/cglm.h>
#include <stddef.h>

#include "../gfx/texture.h"

void batch3d_init(int batch_array_size);
void batch3d_quit(void);

// pos is relative to range [-1.0, 1.0] screen x, y values
void batch3d_queue(int ref_idx, vec3 pos, vec2 offset);
void batch3d_draw(int scale);

int batch3d_get_ref(const char *name);

#endif
