#ifndef ATLAS_H
#define ATLAS_H

#include <cglm/cglm.h>
#include <ghh/hashmap.h>

#include "texture.h"

typedef struct atlas_context atlas_context_t;

// atlas-texture-relative rectangle
typedef struct atlas_ref {
	vec2 pos, size, pixel_size;
} atlas_ref_t;

typedef struct atlas {
	atlas_context_t *ctx;
	texture_t *texture;

	// can access refs through these
	atlas_ref_t *refs;
	size_t num_refs;
	hashmap_t *ref_map;
} atlas_t;

void atlas_construct(atlas_t *);
void atlas_destruct(atlas_t *);

int atlas_add_texture(atlas_t *, const char *name, const char *path);
// generates all possible refs, returns index of first ref
int atlas_add_sheet(atlas_t *, const char *name, const char *path, vec2 cell_size);
void atlas_generate(atlas_t *);

#endif
