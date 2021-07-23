#include <ghh/gtimer.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "draw.h"
#include "batch_font.h"
#include "batch3d.h"
#include "../gfx/gfx.h"

#include <SDL2/SDL.h>

int font_ref, block_ref;
gtimer_t *fps_timer, *draw_timer, *logic_timer;

void draw_init() {
    // sub inits
    batch_font_init(1024);
    batch3d_init(256);

    fps_timer = gtimer_create(120);
    draw_timer = gtimer_create(120);
    logic_timer = gtimer_create(120);

    // load
    font_ref = batch_font_get_ref("font");
    block_ref = batch3d_get_ref("testblock");
}

void draw_quit() {
    gtimer_destroy(fps_timer);
    gtimer_destroy(draw_timer);
    gtimer_destroy(logic_timer);

    batch3d_quit();
    batch_font_quit();
}

void draw_frame() {
    const int scale = 2;
    const float goal_fps = 120.0;
    const font_attrs_t font_attrs = {
        .color = {1.0, 0.0, 1.0, 1.0},
        .italicize = 0.25,
        .scale = 2.0,
        .waviness = 1.0
    };
    char fps_text[100];
    vec3 pos;
    vec2 topleft;

    gfx_get_size(topleft);
    glm_vec2_scale(topleft, -0.5, topleft);

    // fps
    gtimer_tick(fps_timer);
    sprintf(
        fps_text,
        "%6.2f fps\n"
        "\n"
        "frametime for goal %.1f fps:\n"
        "%.2f%% rendering\n"
        "%.2f%% logic\n",
        gtimer_get_fps(fps_timer),
        goal_fps,
        (goal_fps * gtimer_get_avg_tick(draw_timer)) * 100.0,
        (goal_fps * gtimer_get_avg_tick(logic_timer)) * 100.0
    );

    // draw
    gtimer_tick(logic_timer);
    gtimer_tick(draw_timer);
    gtimer_pop_tick(draw_timer); // pops logic tick time

    gfx_clear(0.1, 0.1, 0.1, 1.0);

    /*
    // we can get to 32,768 (2 ** 15) blocks before it's too much. I think it's a heap
    // memory thing, not an opengl thing. TODO memory optimization for batching, ig!
    FOR_CUBE(pos[0], pos[1], pos[2], 0, 4)
        batch3d_queue(block_ref, pos, (vec2){-16.0, -18.0});

    batch3d_draw(scale);
    */

    batch_font_queue(font_ref, (vec2){-200.0, -100.0}/*topleft*/, fps_text, &font_attrs);
    batch_font_draw();

    gtimer_tick(draw_timer);

    gfx_flip(); // this is where vsync happens, not included in frametimes

    gtimer_tick(logic_timer);
    gtimer_pop_tick(logic_timer); // pops draw tick time
}
