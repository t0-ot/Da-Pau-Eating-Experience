#ifndef GAME_H
#define GAME_H

#include "../../include/engine/canvas.h"
#include "../../include/snake/snake_logic.h"

typedef struct {
    Canvas cell[GAME_HEIGHT][GAME_WIDTH];
    BaseText titleText;
    BaseText scoreTexts[2];
    ScoreText scoreNumbers[2];
    RenderObj scoreTextObj;
    RenderObj scoreNumObj;
    RenderObj hiScoreTextObj;
    RenderObj hiScoreNumObj;
    RenderObj titleTextObj;
    snakeData snake1;
    defaultFruit baseFruit;
    fruitData normalFruitSet;
    RenderObj snakeObj1;
    RenderObj normFruitSetObj;
    RenderObj* screenRenderList[7];
    Point lastDirInput[1];
    int totalRenderList;
    int currentHiScore;
    int isRunning;
} GameSession;

int hiScoreCounter(int currentScore, int *highestScore);
int mainMenu(BaseText *gameTitle);
int menuInputHandler(int *menuChoice, int optionAmount);
void gameUpdateLogic(GameSession *session);
int gameInit(GameSession *session);
int gameRunLoop(GameSession *session);
int gameCleanUp(GameSession *session);

#endif // GAME_H
