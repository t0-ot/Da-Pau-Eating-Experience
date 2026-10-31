#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "../../include/engine/canvas.h"

void snakeStamper(void *data, Canvas cellview[][GAME_WIDTH]);
void fruitStamper(void *data, Canvas cellview[][GAME_WIDTH]);

#endif // GAME_RENDER_H
