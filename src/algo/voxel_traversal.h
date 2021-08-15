#ifndef VOXEL_TRAVERSAL_H
#define VOXEL_TRAVERSAL_H

#include <stdbool.h>
#include <gglm/gglm.h>

typedef struct traversal_ctx {
    v3 current, step, t_max, t_delta;
    bool check_adjust;
} traversal_ctx_t;

/*
usage:
traversal_ctx_t ctx = traversal_start(start, dir);
while (condition) {
    // do stuff...
    traversal_next(&ctx);
}
*/

traversal_ctx_t traversal_start(v3 start, v3 direction);
void traversal_next(traversal_ctx_t *);

#endif
