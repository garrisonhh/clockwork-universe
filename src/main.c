#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

vec3 pos = {20.0, 20.0, 20.0}, move = {0};
const uint8_t *keyboard = NULL;

void init(void);
void cleanup(void);
bool process_events(void);

int MAIN(int argc, char **argv) {
    init();

    keyboard = SDL_GetKeyboardState(NULL);

    while (process_events()) {
        // apply game logic()
        draw_frame(pos);
    }

    cleanup();

    return 0;
}

bool process_events() {
    SDL_Event event;

    glm_vec3_copy((vec3){0}, move);

    // polled
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

    // state
    if (keyboard[SDL_SCANCODE_W])
        glm_vec3_add(move, (vec3){-1.0, -1.0,  0.0}, move);
    if (keyboard[SDL_SCANCODE_S])
        glm_vec3_add(move, (vec3){ 1.0,  1.0,  0.0}, move);
    if (keyboard[SDL_SCANCODE_A])
        glm_vec3_add(move, (vec3){-1.0,  1.0,  0.0}, move);
    if (keyboard[SDL_SCANCODE_D])
        glm_vec3_add(move, (vec3){ 1.0, -1.0,  0.0}, move);
    if (keyboard[SDL_SCANCODE_LSHIFT])
        glm_vec3_add(move, (vec3){ 0.0,  0.0, -1.0}, move);
    if (keyboard[SDL_SCANCODE_SPACE])
        glm_vec3_add(move, (vec3){ 0.0,  0.0,  1.0}, move);

    glm_vec3_scale(move, 0.05, move);
    glm_vec3_add(pos, move, pos);

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
