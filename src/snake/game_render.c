#include "../../include/snake/game_render.h"
#include "../../include/snake/snake_logic.h"

void snakeStamper(void *data, Canvas cellview[][GAME_WIDTH]) {
    snakeData *p = (snakeData *)data;
    for (int i = p->pointList.amt - 1; i >= 0; i--) {
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.r = 0;
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.g = 255;
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.b = 0;
        if (i == 0)
            cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].sym = 'M';
        else
            cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].sym = 'O';
    }
}

void fruitStamper(void *data, Canvas cellview[][GAME_WIDTH]) {
    fruitData *p = (fruitData *)data;
    for (int i = 0; i < p->pointList.amt; i++) {
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].sym = p->visuals.sym;
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.r = p->visuals.color.r;
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.g = p->visuals.color.g;
        cellview[p->pointList.points[i].posY][p->pointList.points[i].posX].color.b = p->visuals.color.b;
    }
}
