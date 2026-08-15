#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

int main(void) {
    srand((unsigned int)time(NULL)); // seed once, only here

    GameState gamestate = {0}; // zero-initialize everything
    runGame(&gamestate);

    return 0;
}
