#include "draw.h"
#include "batch2d.h"
#include "../gfx/gfx.h"

int font_ref;

void draw_init() {
    // sub inits
    batch2d_init(256);

    // load
    font_ref = batch2d_get_ref("font");
}

void draw_quit() {
    batch2d_quit();
}

void draw_frame() {
    gfx_clear(0.0, 0.0, 0.0, 1.0);

    batch2d_queue_text(font_ref, (vec2){0.0, 0.0}, "this is a\nmultiline string!\nfat wins.");

    batch2d_draw();
    gfx_flip();
}
