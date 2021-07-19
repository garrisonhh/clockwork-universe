#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "gfx/gfx.h"
#include "draw/draw.h"
#include "draw/batch.h"

#ifdef _WIN32
#define MAIN WinMain
#else
#define MAIN main
#endif

void init(void);
void load(void);
void cleanup(void);
void process_events(bool *quit);

int MAIN(int argc, char **argv) {
    init();
    load();

    bool quit = false;

    while (!quit) {
        process_events(&quit);
        draw_frame();
    }

    cleanup();

    return 0;
}

void process_events(bool *quit) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            *quit = true;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                *quit = true;
                break;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                gfx_on_resize();
            break;
        }
    }
}

void init() {
    gfx_init("hello world!", 1280, 800);
    batch_init(4096);
}

void load() {
    // atlas
    batch_atlas_add_sheet("font", "res/fonts/CGA8x8thick.png", (vec2){8.0, 8.0});
    batch_atlas_generate();

    // uses atlas
    draw_load();
}

void cleanup() {
    batch_quit();
    gfx_quit();
}
