#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <ghh/vector.h>
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
bool process_events(void);

int MAIN(int argc, char **argv) {
    init();

    while (process_events()) {
        // apply game logic()
        draw_frame();
    }

    cleanup();

    return 0;
}

bool process_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                gfx_on_resize();
            break;
        }
    }

    return true;
}

void init() {
    memcheck_init();
    gfx_init("hello world!", 1280, 800);
    draw_init();
}

void cleanup() {
    draw_quit();
    gfx_quit();
    memcheck_quit(true);
}
