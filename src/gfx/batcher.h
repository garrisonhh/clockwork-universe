#ifndef BATCHER_H
#define BATCHER_H

#include <ghh/array.h>
#include <glad/glad.h>

#include "shader.h"

typedef struct batch_buffer batch_buffer_t;

typedef struct batcher {
    // shader
    shader_t *shader;
    GLuint vao;
    GLenum instance_mode;
    size_t instance_count;

    // buffering
    batch_buffer_t *buffers;
    size_t num_buffers;
} batcher_t;

// setting up batch shader:
// 1. allocate and construct()
// 2. attach shader sources and compile
// 3. add buffers corresponding to shader

void batcher_construct(batcher_t *, GLenum instance_mode, size_t instance_count);
void batcher_destruct(batcher_t *);

// buffers only support floats currently. pass length of float vector to vector size
void batcher_add_buffer(batcher_t *, size_t vector_size);

// data is copied from data ptr
void batcher_queue(batcher_t *, float **data);
void batcher_draw(batcher_t *);

#endif
