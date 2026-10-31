#ifndef SNAKE_LOGIC_H
#define SNAKE_LOGIC_H

typedef struct {
    int8_t scoreAmt;
} defaultFruit;

typedef struct {
    int8_t points;
    int8_t thickness;
} fatFruit;

typedef struct {
    int8_t *speed;
    int8_t points;
} speedFruit;

typedef struct {
    ViewPointList pointList;
    bool alive;
} snakeData;

typedef struct {
    void *fruitType;
    void (*onEat)(void *fruit, snakeData *snake);
    ViewPointList pointList;
    Canvas visuals;
} fruitData;

int movementInputHandler(Point prevDirInput[]);
int snakeDirectionHandler(Point snake[], Point lastDir[]);
void shiftSnakeSegments(snakeData *snakeInfo);
int checkLife(ViewPointList snakeView, int playWidthBoundary, int playHeightBoundary);
void handleDeath(snakeData *snake1, ViewPointList *fruit, Point *lastDirInput);
void spawnFruit(ViewPointList fruitView, int fruitNumber, ViewPointList snakeView, int spawnWidth, int spawnHeight);
void baseEat(void *fruit, snakeData *snake);
void handleMilestoneFruit(fruitData *fruitAwardTarget, ViewPointList *snakeScore);
void handleFruitCollision(fruitData *collidedFruitData, snakeData *snakeInfo, int collidedFruitNum);

#endif // SNAKE_LOGIC_H
