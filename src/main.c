#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define GHH_MEMCHECK_ENABLED
#include <ghh/memcheck.h>

#include "gfx/gfx.h"
#include "draw/draw.h"

#ifdef _WIN32
#define MAIN WinMain
#else
#define MAIN main
#endif

void init(void);
void cleanup(void);
void process_events(bool *quit);

int MAIN(int argc, char **argv) {
    init();

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
    memcheck_init();
    gfx_init("hello world!", 1280, 800);
    draw_init();
}

void cleanup() {
    draw_quit();
    gfx_quit();
    memcheck_quit();
}
