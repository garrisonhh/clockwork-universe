#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ghh/vector.h>
#include <ghh/memcheck.h>

#include "batcher.h"
#include "gfx.h"

// linear growth block size for batch buffer
#define BATCH_BUFFER_GROWTH 256

typedef struct batch_buffer {
    // manual memory management here instead of array_t or vector; in this
    // case a linear growth model makes more sense
    // TODO minimize realloc calls from queue()
    float *data;
    size_t size, alloc_size; // occupied size of array, allocated size of array
    size_t vector_size;
} batch_buffer_t;

struct batcher {
   batch_buffer_t *buffers;
   size_t num_buffers;
   GLuint vao;
   GLuint *vbos;
};

batcher_t *batcher_create(const size_t *vector_sizes, const size_t num_buffers) {
    batcher_t *batcher = malloc(sizeof(*batcher));

    batcher->num_buffers = num_buffers;
    batcher->buffers = calloc(batcher->num_buffers, sizeof(*batcher->buffers));

    for (size_t i = 0; i < batcher->num_buffers; ++i) {
        batcher->buffers[i].data = NULL;
        batcher->buffers[i].vector_size = vector_sizes[i];
    }

    batcher->vbos = malloc(batcher->num_buffers * sizeof(*batcher->vbos));

    GL(glGenBuffers(batcher->num_buffers, batcher->vbos));
    GL(glGenVertexArrays(1, &batcher->vao));

    return batcher;
}

void batcher_destroy(batcher_t *batcher) {
    for (size_t i = 0; i < batcher->num_buffers; ++i)
        free(batcher->buffers[i].data);

    GL(glDeleteBuffers(batcher->num_buffers, batcher->vbos));

    free(batcher->vbos);
    free(batcher->buffers);
    free(batcher);
}

void batcher_queue(batcher_t *batcher, float **data) {
    size_t old_size;
    batch_buffer_t *buffer;

    for (size_t i = 0; i < batcher->num_buffers; ++i) {
        buffer = &batcher->buffers[i];
        old_size = buffer->size;

        // resize buffer if needed
        buffer->size += buffer->vector_size;

        if (buffer->size > buffer->alloc_size) {
            buffer->alloc_size += BATCH_BUFFER_GROWTH;

            buffer->data = realloc(buffer->data, buffer->alloc_size * sizeof(*buffer->data));
        }

        // copy vector data to buffer
        memcpy(&buffer->data[old_size], data[i], buffer->vector_size * sizeof(*buffer->data));
    }
}

void batcher_draw(batcher_t *batcher, const GLenum instance_mode, const size_t instance_count) {
    batch_buffer_t *buffer;
    size_t i, num_items;

    // get num items
    if (!(num_items = batcher->buffers[0].size))
        return; // zero items queued, nothing to draw

    // buffer batch data and draw
	GL(glBindVertexArray(batcher->vao));

    for (i = 0; i < batcher->num_buffers; ++i) {
        buffer = &batcher->buffers[i];

        // pass in data
        GL(glBindBuffer(GL_ARRAY_BUFFER, batcher->vbos[i]));
        GL(glBufferData(
            GL_ARRAY_BUFFER,
            buffer->size * sizeof(*buffer->data),
            buffer->data,
            GL_STREAM_DRAW
        ));

        // clear buffer
        free(buffer->data);
        buffer->data = NULL;
        buffer->size = buffer->alloc_size = 0;

        // enable vertex
		GL(glEnableVertexAttribArray(i));
		GL(glVertexAttribPointer(i, buffer->vector_size, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glVertexAttribDivisor(i, 1));
    }

	GL(glDrawArraysInstanced(instance_mode, 0, instance_count, num_items));

	// clean up
	for (i = 0; i < batcher->num_buffers; ++i)
		GL(glDisableVertexAttribArray(i));

	GL(glBindVertexArray(0));
}
