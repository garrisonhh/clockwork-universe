#include <stddef.h>
#include <math.h>
#include <ghh/utils.h>

#include "voxel_traversal.h"

// see https://gist.github.com/garrisonhh/8c7427070272f64af32dcf7601c0e891

#define FSIGNF(n) ((n) < 0.0 ? -1.0 : 1.0)

traversal_ctx_t traversal_start(v3 start, v3 direction) {
    if (v3_eq(direction, v3_ZERO))
        ERROR("attempted to traverse voxels with direction of (0.0, 0.0, 0.0)\n");

    traversal_ctx_t ctx;

    // look at that metaprogramming yowzers
    ctx.current = v3_MAP(start, floor);
    ctx.step = v3_MAP(direction, FSIGNF);

    ctx.t_max = v3_div(v3_sub(v3_add(ctx.current, ctx.step), start), direction);
    ctx.t_delta = v3_div(v3_fill(1.0), v3_mul(direction, ctx.step));

    ctx.check_adjust = true;

    return ctx;
}

void traversal_next(traversal_ctx_t *ctx) {
    // check for initial negative ray adjustment
    if (ctx->check_adjust) {
        ctx->check_adjust = false;

        // see if any axis goes negative
        v3 diff = v3_ZERO;
        bool adjust = false;

        for (size_t i = 0; i < 3; ++i) {
            if (ctx->step.ptr[i] < 0.0) {
                adjust = true;
                diff.ptr[i] = -1.0;
            }
        }

        // an axis is negative, apply adjustment
        if (adjust) {
            ctx->current = v3_add(ctx->current, diff);

            return;
        }
    }

    // find min axis
    size_t axis = 0;
    float min_value = ctx->t_max.x;

    for (size_t i = 1; i < 3; ++i) {
        if (ctx->t_max.ptr[i] < min_value) {
            min_value = ctx->t_max.ptr[i];
            axis = i;
        }
    }

    // iterate on axis found
    ctx->current.ptr[axis] += ctx->step.ptr[axis];
    ctx->t_max.ptr[axis] += ctx->t_delta.ptr[axis];
}
