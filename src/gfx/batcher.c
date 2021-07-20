#include <stdlib.h>
#include <stdio.h>
#include <ghh/vector.h>
#include <ghh/memcheck.h>

#include "batcher.h"
#include "gfx.h"

struct batch_buffer {
    GLuint vbo;
    size_t item_size;
    float *items; // vector
};

void batcher_construct(batcher_t *batcher) {
    batcher->shader = shader_create();
    batcher->buffers = array_create(0);

    GL(glGenVertexArrays(1, &batcher->vao));
}

void batcher_destruct(batcher_t *batcher) {
    // clean up buffers
    batch_buffer_t *buffer;

    for (size_t i = 0; i < array_size(batcher->buffers); ++i) {
        buffer = array_get(batcher->buffers, i);

    	GL(glDeleteBuffers(1, &buffer->vbo));
        VECTOR_FREE(buffer->items);

        FREE(buffer);
    }

    // clean up batcher
    shader_destroy(batcher->shader);
    array_destroy(batcher->buffers, false);

	GL(glDeleteVertexArrays(1, &batcher->vao));
}

void batcher_add_buffer(batcher_t *batcher, size_t item_size) {
    batch_buffer_t *buffer = MALLOC(sizeof(*buffer));

    GL(glGenBuffers(1, &buffer->vbo));

    VECTOR_ALLOC(buffer->items, 0);
    buffer->item_size = item_size;

    array_push(batcher->buffers, buffer);
}

void batcher_queue_attr(batcher_t *batcher, size_t buffer_idx, float *item) {
    batch_buffer_t *buffer = array_get(batcher->buffers, buffer_idx);

    for (size_t i = 0; i < buffer->item_size; ++i)
        VECTOR_PUSH(buffer->items, item[i]);
}

static inline size_t buffer_size(batcher_t *batcher, size_t buffer_idx) {
    batch_buffer_t *buffer = array_get(batcher->buffers, buffer_idx);

    return VECTOR_SIZE(buffer->items) / buffer->item_size;
}

void batcher_draw(batcher_t *batcher) {
    batch_buffer_t *buffer;
    size_t i, num_items;

    // get num items
    if (array_size(batcher->buffers))
        num_items = buffer_size(batcher, 0);
    else
        return; // zero items queued, nothing to draw

#ifdef DEBUG
    // verify buffers
    for (i = 1; i < array_size(batcher->buffers); ++i) {
        if (buffer_size(batcher, i) != num_items)
             ERROR("batcher buffers are not equal in size on draw.");
    }
#endif

    // buffer batch data
	GL(glBindVertexArray(batcher->vao));

    for (i = 0; i < array_size(batcher->buffers); ++i) {
        buffer = array_get(batcher->buffers, i);

        GL(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
        GL(glBufferData(
            GL_ARRAY_BUFFER,
            num_items * buffer->item_size * sizeof(*buffer->items),
            buffer->items,
            GL_STREAM_DRAW
        ));

        VECTOR_CLEAR(buffer->items);

		GL(glEnableVertexAttribArray(i));
		GL(glVertexAttribPointer(i, buffer->item_size, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glVertexAttribDivisor(i, 1));
    }

	// draw
    // TODO parameterize instancing options through batcher_construct
	GL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num_items));

	// clean up
	for (i = 0; i < array_size(batcher->buffers); ++i)
		glDisableVertexAttribArray(i);

	glBindVertexArray(0);
}
