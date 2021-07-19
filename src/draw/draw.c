#include "draw.h"
#include "batch.h"
#include "../gfx/gfx.h"

int font_ref;

void draw_load() {
    font_ref = batch_get_ref("font");

    printf("font ref %d\n", font_ref);
}

void draw_frame() {
    gfx_clear(0.0, 0.0, 0.0, 1.0);

    batch_queue_text(font_ref, (vec2){0.0, 0.0}, "this is a\nmultiline string!");

    batch_draw();
    gfx_flip();
}
