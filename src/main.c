#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "gfx/gfx.h"

#ifdef _WIN32
#define MAIN WinMain
#else
#define MAIN main
#endif

int MAIN(int argc, char **argv) {
    gfx_init("hello world!", 640, 480);

    SDL_Delay(2000);

    gfx_quit();

    return 0;
}
