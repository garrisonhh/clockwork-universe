#ifndef BATCH3D_H
#define BATCH3D_H

#include <cglm/cglm.h>
#include <stddef.h>

void batch3d_init(int batch_array_size);
void batch3d_quit(void);

// pos is 3d position, offset is pixel offset
// TODO store pixel offsets with refs somehow instead of submitting them to queue every time
void batch3d_queue(int ref_idx, int depth_idx, int normal_idx, vec3 pos, vec2 offset);
void batch3d_draw(int scale, float render_dist, vec3 light_pos);

int batch3d_get_ref(const char *name);

#endif
