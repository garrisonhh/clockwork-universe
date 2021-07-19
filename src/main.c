#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define MAIN WinMain
#else
#define MAIN main
#endif

int MAIN(int argc, char **argv) {
    printf("hello world\n");

    return 0;
}
