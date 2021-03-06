#ifndef BATCH_FONT_H
#define BATCH_FONT_H

#include <stddef.h>
#include <gglm/gglm.h>
#include <ghhgfx/texture.h>

typedef struct font_attrs {
    v4 color;
    float italicize, scale, waviness;
} font_attrs_t;

void batch_font_init(void);
void batch_font_quit(void);

// pos is relative to range [-1.0, 1.0] screen x, y values
// pass NULL for defaults
void batch_font_queue(int font_idx, v2 pos, const char *text, font_attrs_t *attrs);
void batch_font_draw();

int batch_font_get_ref(const char *name);

#endif
