#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

int main(void) {
    srand((unsigned int)time(NULL));   // ONE-TIME seed — never call srand() again anywhere else

    GameState gamestate = {0};                // zero-initialize everything
    runGame(&gamestate);

    return 0;
}
