#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/engine/terminal.h"
#include "../../include/engine/canvas.h"
#include "../../include/engine/utils.h"
#include "../../include/snake/snake_logic.h"

int movementInputHandler(Point prevDirInput[]) {
    int8_t dx = 0, dy = 0;
    char input = inputHandler();

    if (input == '\0') return 0;

    switch (input) {
        case 'w': dx =  0; dy = -1; break;
        case 's': dx =  0; dy =  1; break;
        case 'a': dx = -1; dy =  0; break;
        case 'd': dx =  1; dy =  0; break;
        default:  return 1;
    }

    if (prevDirInput[0].posX * -1 == dx && prevDirInput[0].posY * -1 == dy)
        return 1;

    prevDirInput[0].posX = dx;
    prevDirInput[0].posY = dy;
    return 0;
}

int snakeDirectionHandler(Point snake[], Point lastDir[]) {
    if (snake[0].posX + lastDir[0].posX > 0 && snake[0].posX + lastDir[0].posX < HALF_GAMEVIEW_BOUNDARY && snake[0].posY + lastDir[0].posY > 0 && snake[0].posY + lastDir[0].posY < GAME_HEIGHT - 1) {
        snake[0].posX += lastDir[0].posX;
        snake[0].posY += lastDir[0].posY;
        return 1;
    } else
        return 0;
}

void shiftSnakeSegments(snakeData *snakeInfo) {
    for (int i = snakeInfo->pointList.amt- 1; i > 0; i--) {
        snakeInfo->pointList.points[i] = snakeInfo->pointList.points[i-1];
    }
}

int checkLife(ViewPointList snakeView, int playWidthBoundary, int playHeightBoundary) {
    if (snakeView.points[0].posX == 0 || snakeView.points[0].posY == 0 || snakeView.points[0].posX == playWidthBoundary || snakeView.points[0].posY == playHeightBoundary)
        return 0;
    if (collisionCheckOne(&snakeView, &snakeView, 1, 0))
        return 0;
    return 1;
}

void handleDeath(snakeData *snake1, ViewPointList *fruit, Point *lastDirInput){
    snake1->pointList.points[0].posX = 20;
    snake1->pointList.points[0].posY = 10;
    lastDirInput[0].posY = 0;
    lastDirInput[0].posX = 0;
    snake1->pointList.amt = 2;
    snake1->alive = 1;
    fruit->amt = 1;
    objectPointResize(&snake1->pointList);
    objectPointResize(fruit);
    spawnFruit(*fruit,  0, snake1->pointList, HALF_GAMEVIEW_BOUNDARY, GAME_HEIGHT - 1);
}

void spawnFruit(ViewPointList fruitView, int fruitNumber, ViewPointList snakeView, int spawnWidth, int spawnHeight) {
    do {
        fruitView.points[fruitNumber].posX = (rand() % (spawnWidth - 2)) + 1;
        fruitView.points[fruitNumber].posY = (rand() % (spawnHeight - 2)) + 1;
    } while (collisionCheckOne(&snakeView, &fruitView, 0, fruitNumber) );
}

void baseEat(void *fruit, snakeData *snake) {
    snake->pointList.amt += ((defaultFruit *)((fruitData *)fruit)->fruitType)->scoreAmt;
}

void handleMilestoneFruit(fruitData *fruitAwardTarget, ViewPointList *snakeScore) {
    if (((snakeScore->amt -2) % 10) != 0)
        return;
    fruitAwardTarget->pointList.amt++;
    objectPointResize(&fruitAwardTarget->pointList);
    spawnFruit(fruitAwardTarget->pointList, fruitAwardTarget->pointList.amt-1, *snakeScore, HALF_GAMEVIEW_BOUNDARY, GAME_HEIGHT - 1);
}

void handleFruitCollision(fruitData *collidedFruitData, snakeData *snakeInfo, int collidedFruitNum) {
    collidedFruitData->onEat(collidedFruitData, snakeInfo);
    objectPointResize(&snakeInfo->pointList);
    handleMilestoneFruit(collidedFruitData, &snakeInfo->pointList);
    spawnFruit(collidedFruitData->pointList, collidedFruitNum-1, snakeInfo->pointList, HALF_GAMEVIEW_BOUNDARY, GAME_HEIGHT - 1);
}
