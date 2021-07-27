#include <ghh/memcheck.h>
#include <ghh/array.h>
#include <ghh/list.h>
#include <ghh/utils.h>

#include "atlas.h"
#include "gfx.h"

// used to construct atlas
struct atlas_context {
	size_t index;
	array_t *textures;
	array_t *packets;
	array_t *bins; // if list_t every gets a sorting algorithm, use it instead
};

// used for pixel sizes pre-atlas-generation
typedef struct atlas_packet {
	vec2 pos, size;

	bool sheet;
	vec2 sheet_size, cell_size; // for sheet, if packet is a sheet
} atlas_packet_t;

atlas_context_t *atlas_context_create() {
	atlas_context_t *ctx = malloc(sizeof(*ctx));

	ctx->textures = array_create(0);
	ctx->packets = array_create(0);
	ctx->bins = array_create(0);
	ctx->index = 0;

	return ctx;
}

void atlas_context_destroy(atlas_context_t *ctx) {
	for (size_t i = 0; i < array_size(ctx->textures); ++i)
		texture_destroy(array_get(ctx->textures, i));

	array_destroy(ctx->textures, false);
	array_destroy(ctx->packets, true);
	array_destroy(ctx->bins, true);
	free(ctx);
}

void atlas_construct(atlas_t *atlas) {
	atlas->ctx = atlas_context_create();
	atlas->texture = NULL;

	atlas->refs = NULL;
	atlas->num_refs = 0;
	atlas->ref_map = hashmap_create(0, -1, true);

	// set up bins
	atlas_packet_t *base_bin = malloc(sizeof(*base_bin));

	*base_bin = (atlas_packet_t){
		.pos = {0, 0},
		.size = {INFINITY, INFINITY}
	};

	array_push(atlas->ctx->bins, base_bin);
}

void atlas_destruct(atlas_t *atlas) {
	texture_destroy(atlas->texture);
    hashmap_destroy(atlas->ref_map, true);
    free(atlas->refs);
}

static int bin_compare(const void *a, const void *b) {
	atlas_packet_t *bina, *binb;

	bina = *(atlas_packet_t **)a;
	binb = *(atlas_packet_t **)b;

	return MAX(bina->pos[0], bina->pos[1]) - MAX(binb->pos[0], binb->pos[1]);
}

static int atlas_add_lower(atlas_t *atlas, const char *name, texture_t *tex, bool sheet, vec2 cell_size) {
	if (atlas->ctx == NULL)
		ERROR("cannot add to batch atlas after it has been generated.\n");

	int this_index = atlas->ctx->index;
	int *ref_ptr;
	atlas_packet_t *bin = NULL;
	atlas_packet_t *packet = malloc(sizeof(*packet));
	atlas_packet_t *new_bins[2];

	// find best bin
	for (int i = 0; i < array_size(atlas->ctx->bins); ++i) {
		bin = array_get(atlas->ctx->bins, i);

		if (bin->size[0] >= tex->w && bin->size[1] >= tex->h) {
			bin = array_get(atlas->ctx->bins, i);
			array_del(atlas->ctx->bins, i);
			break;
		}
	}

	// store new packet
	*packet = (atlas_packet_t){
		.pos = {bin->pos[0], bin->pos[1]}, // cppcheck-suppress nullPointer
		.size = {tex->w, tex->h},
		.sheet = sheet,
		.cell_size = {cell_size[0], cell_size[1]}
	};

	array_push(atlas->ctx->packets, packet);
	array_push(atlas->ctx->textures, tex);

	ref_ptr = malloc(sizeof(*ref_ptr));
	*ref_ptr = this_index;

	hashmap_set(atlas->ref_map, (char *)name, ref_ptr);

	// increment index
	if (sheet) {
		glm_vec2_div(packet->size, packet->cell_size, packet->sheet_size);

		glm_vec2_copy(
			(vec2){(int)packet->sheet_size[0], (int)packet->sheet_size[1]},
			packet->sheet_size
		);

		atlas->ctx->index += packet->sheet_size[0] * packet->sheet_size[1];
	} else {
		++atlas->ctx->index;
	}

	// create new bins
	for (int i = 0; i < 2; ++i)
		new_bins[i] = malloc(sizeof(*new_bins[i]));

	if (bin->size[0] < bin->size[1]) {
		*new_bins[0] = (atlas_packet_t){
			.pos = {bin->pos[0] + packet->size[0], bin->pos[1]},
			.size = {bin->size[0] - packet->size[0], packet->size[1]}
		};
		*new_bins[1] = (atlas_packet_t){
			.pos = {bin->pos[0], bin->pos[1] + packet->size[1]},
			.size = {bin->size[0], bin->size[1] - packet->size[1]}
		};
	} else {
		*new_bins[0] = (atlas_packet_t){
			.pos = {bin->pos[0], bin->pos[1] + packet->size[1]},
			.size = {packet->size[0], bin->size[1] - packet->size[1]}
		};
		*new_bins[1] = (atlas_packet_t){
			.pos = {bin->pos[0] + packet->size[0], bin->pos[1]},
			.size = {bin->size[0] - packet->size[0], bin->size[1]}
		};
	}

	free(bin);

	// add new bins back to bins and sort
	for (int i = 0; i < 2; ++i)
		if (new_bins[i]->size[0] && new_bins[i]->size[1])
			array_push(atlas->ctx->bins, new_bins[i]);

	array_qsort(atlas->ctx->bins, bin_compare);

	return this_index;
}

int atlas_add_texture(atlas_t *atlas, const char *name, const char *filename) {
    return atlas_add_lower(atlas, name, texture_create(filename), false, GLM_VEC2_ZERO);
}

int atlas_add_sheet(atlas_t *atlas, const char *name, const char *filename, vec2 cell_size) {
	return atlas_add_lower(atlas, name, texture_create(filename), true, cell_size);
}

void atlas_generate(atlas_t *atlas) {
	if (!array_size(atlas->ctx->packets))
		ERROR("attempted to generate atlas with no textures added.\n");

	int i, x, y;
	int idx;
    vec2 atlas_size;
	texture_t *tex;
	atlas_packet_t *packet;

	// find size of atlas
	atlas_size[0] = atlas_size[1] = 0.0;

	for (i = 0; i < array_size(atlas->ctx->packets); ++i) {
		packet = array_get(atlas->ctx->packets, i);

		if (packet->pos[0] + packet->size[0] > atlas_size[0])
			atlas_size[0] = packet->pos[0] + packet->size[0];
		if (packet->pos[1] + packet->size[1] > atlas_size[1])
			atlas_size[1] = packet->pos[1] + packet->size[1];
	}

	atlas->texture = texture_create_empty(atlas_size[0], atlas_size[1]);

	// blit all textures to the atlas
    texture_fbo_generate(atlas->texture);
    gfx_bind_target(atlas->texture);

	for (i = 0; i < array_size(atlas->ctx->textures); ++i) {
		tex = array_get(atlas->ctx->textures, i);
        packet = array_get(atlas->ctx->packets, i);

        texture_fbo_generate(tex);
        gfx_blit(tex, packet->pos, packet->size);
        texture_fbo_delete(tex);
	}

    gfx_unbind_target();
    texture_fbo_delete(atlas->texture);

	// finalize and store references
	atlas->num_refs = atlas->ctx->index;
	atlas->refs = malloc(atlas->num_refs * sizeof(*atlas->refs));

	idx = 0;

	for (i = 0; i < array_size(atlas->ctx->packets); ++i) {
		packet = array_get(atlas->ctx->packets, i);

		if (packet->sheet) {
			for (y = 0; y < packet->sheet_size[1]; ++y) {
				for (x = 0; x < packet->sheet_size[0]; ++x) {
					glm_vec2_copy(packet->cell_size, atlas->refs[idx].pixel_size);

					glm_vec2_mul((vec2){x, y}, packet->cell_size, atlas->refs[idx].pos);
					glm_vec2_add(atlas->refs[idx].pos, packet->pos, atlas->refs[idx].pos);

					glm_vec2_div(packet->cell_size, atlas_size, atlas->refs[idx].size);
					glm_vec2_div(atlas->refs[idx].pos, atlas_size, atlas->refs[idx].pos);

					++idx;
				}
			}
		} else {
			glm_vec2_copy(packet->size, atlas->refs[idx].pixel_size);
			glm_vec2_div(packet->pos, atlas_size, atlas->refs[idx].pos);
			glm_vec2_div(packet->size, atlas_size, atlas->refs[idx].size);

			++idx;
		}
	}

	// clean up
	atlas_context_destroy(atlas->ctx);
	atlas->ctx = NULL;
}
