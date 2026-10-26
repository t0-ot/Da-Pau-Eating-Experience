#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../include/engine/canvas.h"

#define GAME_NAME "Da Pau Eating Experience"
#define HALF_GAMEVIEW_BOUNDARY (GAME_WIDTH / 2) - 1

struct termios original_term;

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
    Point *points;
    int16_t amt;
} ViewPointList;

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

void baseEat(void *fruit, snakeData *snake);
int objectPointResize(ViewPointList *data);
int collisionCheckOne(ViewPointList *objView1, ViewPointList *objView2, int countFrom, int checkIndexObj2);
int hiScoreCounter(int currentScore, int *highestScore);
void enable_raw_mode();
int movementInputHandler(Point prevDirInput[]);
int snakeDirectionHandler(Point snake[], Point lastDir[]);
void disable_raw_mode();
void spawnFruit(ViewPointList fruitView, int fruitNumber, ViewPointList snakeView, int spawnWidth, int spawnHeight);
void scoreDisplay(int currentPoints, int spacing);
int mainMenu(BaseText *gameTitle);
int menuInputHandler(int *menuChoice, int optionAmount);
void shiftSnakeSegments(snakeData *snakeInfo);
int checkLife(ViewPointList snakeView, int playWidthBoundary, int playHeightBoundary);
void snakeStamper(void *data, Canvas cellview[][GAME_WIDTH]);
void fruitStamper(void *data, Canvas cellview[][GAME_WIDTH]);
void handleFruitCollision(fruitData *collidedFruitData, snakeData *snakeInfo, int collidedFruitNum);
void handleMilestoneFruit(fruitData *fruitAwardTarget, ViewPointList *snakeScore);

int main(void) {
    srand(time(NULL));
    int currentHiScore = 0;
    Canvas cell[GAME_HEIGHT][GAME_WIDTH];

    BaseText titleText = {
        .text = GAME_NAME,
        .startX = HALF_GAMEVIEW_BOUNDARY - 11,
        .startY = 3,
        .color = {
            {.r = 255, .g = 105, .b = 180},
            {.r = 255, .g = 215, .b = 0}
        }
    };

    BaseText scoreTexts[2] = {
        [0].text = "-== SCORE ==-",
        [0].startX = 53,
        [0].startY = 9,
        [1].text = "-+== HI-SCORE ==+-",
        [1].startX = 50,
        [1].startY = 13,
        [0 ... 1].color = {
            [0].r = 255,
            [0].g = 215,
            [0].b = 0
        }
    };

    RenderObj scoreTextObj = {
        .data = &scoreTexts[0],
        .stamp = canvasTextStamper
    };
    RenderObj hiScoreTextObj = {
        .data = &scoreTexts[1],
        .stamp = canvasTextStamper
    };
    RenderObj titleTextObj = {
        .data = &titleText,
        .stamp = canvasTextStamperGradient
    };
    snakeData snake1 = {
        .pointList.amt = 2,
        .alive = 1
    };
    snake1.pointList.points = malloc(snake1.pointList.amt * sizeof(Point));
    snake1.pointList.points->posX = 20;
    snake1.pointList.points->posY = 10;
    ScoreText scoreNumbers[2] = {
        [0 ... 1].score = 0,
        [0].y = 8,
        [1].y = 12,
        [0 ... 1].x = 58
    };
    RenderObj scoreNumObj = {
        .data = &scoreNumbers[0],
        .stamp = canvasScoreStamper
    };
    RenderObj hiScoreNumObj = {
        .data = &scoreNumbers[1],
        .stamp = canvasScoreStamper
    };
    RenderObj snakeObj1 = {
        .data = &snake1,
        .stamp = snakeStamper
    };

    defaultFruit baseFruit = { .scoreAmt = 1};
    fruitData normalFruitSet = {
        .fruitType = &baseFruit,
        .pointList.amt = 1,
        .visuals = { .sym = 'P', .color.r =  0, .color.g = 255, .color.b = 0},
        .onEat = baseEat
    };
    normalFruitSet.pointList.points = malloc(normalFruitSet.pointList.amt * sizeof(Point));
    RenderObj normFruitSetObj = {
        .data = &normalFruitSet,
        .stamp = fruitStamper
    };
    RenderObj* screenRenderList[] = {
        &normFruitSetObj,
        &snakeObj1,
        &scoreTextObj,
        &hiScoreTextObj,
        &titleTextObj,
        &scoreNumObj,
        &hiScoreNumObj
    };
    int totalRenderList = sizeof(screenRenderList) / sizeof(screenRenderList[0]);
    Point lastDirInput[1] = {{0, 0}};
    printf("\033[?25l");
    switch (mainMenu(&titleText)) {
        case 1:
            return 0;
        case 0:
            enable_raw_mode();
            for (int i = 0; i < normalFruitSet.pointList.amt; i++)
                spawnFruit(normalFruitSet.pointList, i, snake1.pointList, GAME_WIDTH - 41, GAME_HEIGHT - 1);

            while (1) {
                int moveBlocked = movementInputHandler(lastDirInput);
                int collideWithFruit = collisionCheckOne(&normalFruitSet.pointList, &snake1.pointList, 0, 0);
                if (collideWithFruit > 0)
                    handleFruitCollision(&normalFruitSet, &snake1, collideWithFruit);

                shiftSnakeSegments(&snake1);
                snakeDirectionHandler(snake1.pointList.points, lastDirInput);
                snake1.alive = checkLife(snake1.pointList, GAME_WIDTH - 1, HALF_GAMEVIEW_BOUNDARY);

                if (!(snake1.alive) && !moveBlocked && !(lastDirInput[0].posX == 0 && lastDirInput[0].posY == 0)) {
                    snake1.pointList.points[0].posX = 20;
                    snake1.pointList.points[0].posY = 10;
                    lastDirInput[0].posY = 0;
                    lastDirInput[0].posX = 0;
                    snake1.pointList.amt = 2;
                    snake1.alive = 1;
                    normalFruitSet.pointList.amt = 1;
                    objectPointResize(&snake1.pointList);
                    objectPointResize(&normalFruitSet.pointList);
                }
                scoreNumbers[0].score = snake1.pointList.amt;
                scoreNumbers[1].score = hiScoreCounter(snake1.pointList.amt, &currentHiScore);

                printf("\033[H");
                canvasIntializer(cell, GAME_WIDTH - 41);

                for (int i = 0; i < totalRenderList; i++)
                    screenRenderList[i]->stamp(screenRenderList[i]->data, cell);
                canvasViewPrint(cell);
                printf("W - UP, S - DOWN, A - LEFT, D - RIGHT\n");

                fflush(stdout);
                usleep(100000);
            }
            free(snake1.pointList.points);
            free(normalFruitSet.pointList.points);
            disable_raw_mode();
            break;
    }
    printf("\033[?25h");
    return 0;
}
float ease(float time, float startValue, float change, float duration) {
     time /= duration / 2;
     if (time < 1)  {
          return change / 2 * time * time + startValue;
     }

     time--;
     return -change / 2 * (time * (time - 2) - 1) + startValue;
 };

int mainMenu(BaseText *gameTitle) {
    Canvas menuCell[GAME_HEIGHT][GAME_WIDTH];
    OptionText optionChoices[2] = {
        [0].textData.text = "PLAY!",
        [0].textData.startX = HALF_GAMEVIEW_BOUNDARY - 2,
        [0].textData.startY = 10,
        [0].optionAmt = 2,
        [1].textData.text = "QUIT GAME",
        [1].textData.startX = HALF_GAMEVIEW_BOUNDARY - 4,
        [1].textData.startY = 12,
        [0 ... 1].textData.color = { {0, 255, 0}, {255, 255, 255} }
    };

    float time = 0.0f;
    float duration = 1.0f;
    float startPos = 0.0f;
    float targetPos = 40.0f;
    float change = targetPos - startPos;

    int menuChoice = 0;
    enable_raw_mode();
    while (1) {
        printf("\033[H");

        canvasIntializer(menuCell, startPos);
        if (menuInputHandler(&menuChoice, 2))
            break;
        for (int i = 0; i < optionChoices[0].optionAmt; i++) {
            optionChoices[i].isSelected = 0;
            if (i == menuChoice)
                optionChoices[i].isSelected = 1;
        }
        optionChoiceStamper(optionChoices, menuCell);
        canvasTextStamperGradient(gameTitle, menuCell);
        canvasViewPrint(menuCell);
        fflush(stdout);
        usleep(30000);
    }

    if (menuChoice == 1)
        return 1;

    while (time <= duration) {
        printf("\033[H");

        int currentPos = (int)ease(time, startPos, change, duration);
        gameTitle->startX = (int)ease(time, (HALF_GAMEVIEW_BOUNDARY - 11), 21, duration);

        canvasIntializer(menuCell, currentPos);
        canvasTextStamperGradient(gameTitle, menuCell);
        canvasViewPrint(menuCell);

        fflush(stdout);
        usleep(30000);
        time += 0.03f;
    }
    disable_raw_mode();
    return 0;
}

int objectPointResize(ViewPointList *data) {
    if (data->amt <= 0) {
        free(data->points);
        data->points = NULL;
        return 1;
    }

    Point *temp = realloc(data->points, data->amt * sizeof(Point));
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Out of memory during resize!\n");
        exit(EXIT_FAILURE);
    }
    data->points = temp;
    return 1;
}

int collisionCheckOne(ViewPointList *objView1, ViewPointList *objView2, int countFrom, int checkIndexObj2) {
    for (int i = countFrom; i < objView1->amt; i++) {
        if (objView1 == objView2 && i == checkIndexObj2)
            continue;
        if (objView1->points[i].posX == objView2->points[checkIndexObj2].posX && objView1->points[i].posY == objView2->points[checkIndexObj2].posY)
            return i+1;
    }
    return 0;
}

int hiScoreCounter(int currentScore, int *highestScore) {
    if (currentScore > *highestScore)
        *highestScore = currentScore;
    return *highestScore;
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_term);
    raw = original_term;

    raw.c_lflag &= ~(ICANON | ECHO);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

char inputHandler() {
    char latest_ch = 0, temp_ch;
    while (read(STDIN_FILENO, &temp_ch, 1) > 0) {
        latest_ch = temp_ch;
    }
    if (latest_ch == 0) return '\0';
    return latest_ch;
}

int menuInputHandler(int *menuChoice, int optionAmount) {
    char input = inputHandler();

    if (input == '\0') return 0;

    switch (input) {
        case 'w':
            if (*menuChoice - 1 >= 0)
                (*menuChoice)--;
            break;
        case 's':
            if (*menuChoice + 1 < optionAmount)
                (*menuChoice)++;
            break;
        case '\n':
            return 1;
    }
    return 0;
}

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

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
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
