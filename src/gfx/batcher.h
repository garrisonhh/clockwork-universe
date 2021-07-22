#ifndef BATCHER_H
#define BATCHER_H

#include <ghh/array.h>
#include <glad/glad.h>

#include "shader.h"

typedef struct batcher {
    shader_t *shader;
    array_t *buffers; // TODO make this a vector/flat array?
    GLuint vao;
    GLenum instance_mode;
    GLsizei instance_count;
} batcher_t;

// setting up batch shader:
// 1. allocate and construct()
// 2. attach shader sources and compile
// 3. add buffers corresponding to shader

void batcher_construct(batcher_t *, GLenum instance_mode, size_t instance_count);
void batcher_destruct(batcher_t *);

// buffers are always GL_FLOAT; item_size = 3 for vec3, 2 for vec2, etc
void batcher_add_buffer(batcher_t *, size_t item_size);

// queues one item for one buffer, indexed by the order it is added to batcher
// use this to write wrappers for your use case!
void batcher_queue_attr(batcher_t *, size_t buffer_idx, float *item);
void batcher_draw(batcher_t *);

#endif
