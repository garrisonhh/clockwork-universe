#include <stdlib.h>
#include <ghh/gtimer.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>
#include <ghhgfx/gfx.h>

#include "batch_font.h"
#include "batch3d.h"
#include "../algo/noise.h"

int font_ref, block_ref, bdepth_ref, bnorm_ref;
gtimer_t *fps_timer, *draw_timer, *logic_timer;

void draw_init() {
    // sub inits
    batch_font_init();
    batch3d_init();

    fps_timer = gtimer_create(120);
    draw_timer = gtimer_create(120);
    logic_timer = gtimer_create(120);

    // load
    font_ref = batch_font_get_ref("font");
    block_ref = batch3d_get_ref("blocktest");
    bdepth_ref = batch3d_get_ref("blockdepth");
    bnorm_ref = batch3d_get_ref("blocknormal");
}

void draw_quit() {
    gtimer_destroy(fps_timer);
    gtimer_destroy(draw_timer);
    gtimer_destroy(logic_timer);

    batch3d_quit();
    batch_font_quit();
}

void draw_frame(v3 test) {
    v2 topleft = v2_muls(gfx_get_size(), -0.5);

    // fps + text
    gtimer_tick(fps_timer);
    float fps = gtimer_get_fps(fps_timer);
    char fps_text[100];

    sprintf(
        fps_text,
        "%6.2f fps\n- %.2f%% rendering\n- %.2f%% logic\n\n%6.2f %6.2f %6.2f\n",
        fps,
        (fps * gtimer_get_avg_tick(draw_timer)) * 100.0,
        (fps * gtimer_get_avg_tick(logic_timer)) * 100.0,
        v3_EXPAND(test)
    );

    // draw
    gtimer_tick(logic_timer);
    gtimer_tick(draw_timer);
    gtimer_pop_tick(draw_timer); // pops logic tick time

    gfx_clear(0.1, 0.1, 0.1, 1.0);

    // we can get to 32,768 (2 ** 15) blocks before it's too much. I think it's a heap
    // memory thing. TODO memory optimization for batching, ig!
    double dims = 32.0, value;
    v3 pos;

    FOR_CUBE(pos.x, pos.y, pos.z, 0.0, dims) {
        value = perlin3(v3_muls(pos, 2.0 * (1.0 / dims)));
        value *= ((pos.y / (dims - 1.0)) - 0.5);

        if (value > 0.1)
            batch3d_queue(block_ref, bdepth_ref, bnorm_ref, pos, v2_(-16.0, -20.0));
    }

    FOR_CUBE(pos.x, pos.y, pos.z, 0, 3) {
        batch3d_queue(
            block_ref, bdepth_ref, bnorm_ref,
            v3_add(pos, test), v2_(-16.0, -20.0)
        );
    }

    batch3d_draw(1, 50.0, v3_(0.6, 0.8, 1.0));

    batch_font_queue(font_ref, topleft, fps_text, NULL);
    batch_font_draw();

    gtimer_tick(draw_timer);

    gfx_flip(); // this is where vsync happens (I think), not included in frametimes

    gtimer_tick(logic_timer);
    gtimer_pop_tick(logic_timer); // pops draw tick time
}
