#ifndef BATCHER_H
#define BATCHER_H

#include <stddef.h>
#include <glad/glad.h>

typedef struct batcher batcher_t;

// buffers are all floats or float vectors
batcher_t *batcher_create(const size_t *vector_sizes, const size_t num_buffers);
void batcher_destroy(batcher_t *);

// data is copied from data ptr
void batcher_queue(batcher_t *, float **data);
void batcher_draw(batcher_t *, const GLenum instance_mode, const size_t instance_count);

#endif
