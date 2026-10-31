#include <stdlib.h>
#include <time.h>
#include "../../include/snake/game.h"

int main(void) {
    srand(time(NULL));
    GameSession session;
    gameInit(&session);
    switch (mainMenu(&session.titleText)) {
        case 1:
            return 0;
        case 0:
            gameRunLoop(&session);
            gameCleanUp(&session);
            break;
    }
    return 0;
}
