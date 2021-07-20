#ifndef BATCHER_H
#define BATCHER_H

#include <ghh/array.h>
#include <glad/glad.h>

#include "shader.h"

typedef struct batch_buffer batch_buffer_t;

typedef struct batcher {
    shader_t *shader;
    array_t *buffers; // TODO make this a vector/flat array?
    GLuint vao;
} batcher_t;

// setting up batch shader:
// 1. allocate and construct()
// 2. attach shader sources and compile
// 3. add buffers corresponding to shader

void batcher_construct(batcher_t *);
void batcher_destruct(batcher_t *);

// buffers are always GL_FLOAT; item_size = 3 for vec3, 2 for vec2, etc
void batcher_add_buffer(batcher_t *, size_t item_size);

// queues one item for one buffer, indexed by the order it is added to batcher
// write wrappers for this!
void batcher_queue_attr(batcher_t *, size_t buffer_idx, float *item);
void batcher_draw(batcher_t *);

#endif
