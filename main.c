#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

// Program entry point
int main(void) {
    srand((unsigned int)time(NULL)); // Seed the random generator once

    GameState gamestate = {0}; // Zero-initialize every field within the struct before use
    runGame(&gamestate); // game.c runs the whole simulation from here.

    return 0;
}
