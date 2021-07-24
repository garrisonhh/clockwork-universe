#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ghh/vector.h>
#include <ghh/memcheck.h>

#include "batcher.h"
#include "gfx.h"

// linear growth block size for batch buffer
#define BATCH_BUFFER_GROWTH 256

struct batch_buffer {
    // manually memory management here instead of array_t or vector; in this
    // case a linear growth model makes more sense
    // TODO a block-based approach to prevent tons of realloc calls?
    float *data;
    size_t size, alloc_size; // occupied size of array, allocated size of array
    size_t size_data; // size of one buffered attribute (vec3 = 3, float * = 1)
    GLuint vbo;
};

void batcher_construct(batcher_t *batcher, GLenum instance_mode, size_t instance_count) {
    batcher->shader = shader_create();
    batcher->instance_mode = instance_mode;
    batcher->instance_count = instance_count;
    batcher->buffers = NULL;
    batcher->num_buffers = 0;

    GL(glGenVertexArrays(1, &batcher->vao));
}

void batcher_destruct(batcher_t *batcher) {
    for (size_t i = 0; i < batcher->num_buffers; ++i) {
        if (batcher->buffers[i].data != NULL)
            free(batcher->buffers[i].data);

        GL(glGenBuffers(1, &batcher->buffers[i].vbo));
    }

    shader_destroy(batcher->shader);
    free(batcher->buffers);

	GL(glDeleteVertexArrays(1, &batcher->vao));
}

void batcher_add_buffer(batcher_t *batcher, size_t vector_size) {
    size_t index = batcher->num_buffers;

    batcher->buffers = realloc(
        batcher->buffers,
        (++batcher->num_buffers) * sizeof(*batcher->buffers)
    );

    batcher->buffers[index] = (batch_buffer_t){0};
    batcher->buffers[index].data = NULL;
    batcher->buffers[index].size_data = vector_size;

    GL(glGenBuffers(1, &batcher->buffers[index].vbo));
}

void batcher_queue(batcher_t *batcher, float **data) {
    size_t index;
    batch_buffer_t *buffer;

    for (size_t i = 0; i < batcher->num_buffers; ++i) {
        buffer = &batcher->buffers[i];
        index = buffer->size;

        // resize buffer if needed
        buffer->size += buffer->size_data;

        if (buffer->size > buffer->alloc_size) {
            buffer->alloc_size += BATCH_BUFFER_GROWTH;

            buffer->data = realloc(buffer->data, buffer->alloc_size * sizeof(*buffer->data));
        }

        // copy vector to buffer
        memcpy(&buffer->data[index], data[i], buffer->size_data * sizeof(*buffer->data));
    }
}

void batcher_draw(batcher_t *batcher) {
    batch_buffer_t *buffer;
    size_t i, num_items;

    // get num items
    if (!(num_items = batcher->buffers[0].size))
        return; // zero items queued, nothing to draw

    // buffer batch data
	GL(glBindVertexArray(batcher->vao));

    for (i = 0; i < batcher->num_buffers; ++i) {
        buffer = &batcher->buffers[i];

        // pass in data
        GL(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
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
		GL(glVertexAttribPointer(i, buffer->size_data, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glVertexAttribDivisor(i, 1));
    }

	// draw
	GL(glDrawArraysInstanced(
        batcher->instance_mode,
        0,
        batcher->instance_count,
        num_items
    ));

	// clean up
	for (i = 0; i < batcher->num_buffers; ++i)
		GL(glDisableVertexAttribArray(i));

	GL(glBindVertexArray(0));
}
