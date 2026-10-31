#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "../../include/snake/game.h"
#include "../../include/engine/canvas.h"
#include "../../include/engine/terminal.h"
#include "../../include/engine/utils.h"
#include "../../include/snake/game_render.h"

#define GAME_NAME "Da Pau Eating Experience"

int gameInit(GameSession *session) {
    *session = (GameSession){
        .titleText = {
            .text = GAME_NAME,
            .startX = HALF_GAMEVIEW_BOUNDARY - 11,
            .startY = 3,
            .color = {
                {.r = 255, .g = 105, .b = 180},
                {.r = 255, .g = 215, .b = 0}
            }
        },
        .scoreTexts = {
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
        },
        .scoreTextObj = {
            .stamp = canvasTextStamper
        },
        .hiScoreTextObj = {
            .stamp = canvasTextStamper
        },
        .titleTextObj = {
            .data = &session->titleText,
            .stamp = canvasTextStamperGradient
        },
        .snake1 = {
            .pointList.amt = 2,
            .alive = 1
        },
        .scoreNumbers = {
            [0 ... 1].score = 0,
            [0].y = 8,
            [1].y = 12,
            [0 ... 1].x = 58
        },
        .scoreNumObj = {
            .stamp = canvasScoreStamper
        },
        .hiScoreNumObj = {
            .stamp = canvasScoreStamper
        },
        .snakeObj1 = {
            .stamp = snakeStamper
        },
        .baseFruit = { .scoreAmt = 1},
        .normalFruitSet = {
            .pointList.amt = 1,
            .visuals = { .sym = 'P', .color.r =  0, .color.g = 255, .color.b = 0},
            .onEat = baseEat
        },
        .normFruitSetObj = {
            .stamp = fruitStamper
        },
        .lastDirInput = {{0, 0}},
        .currentHiScore = 0,
        .isRunning = 1
    };

    session->scoreTextObj.data = &session->scoreTexts[0];
    session->hiScoreTextObj.data = &session->scoreTexts[1];
    session->titleTextObj.data = &session->titleText;
    session->scoreNumObj.data = &session->scoreNumbers[0];
    session->hiScoreNumObj.data = &session->scoreNumbers[1];
    session->snakeObj1.data = &session->snake1;
    session->normFruitSetObj.data = &session->normalFruitSet;

    session->screenRenderList[0] = &session->normFruitSetObj;
    session->screenRenderList[1] = &session->snakeObj1;
    session->screenRenderList[2] = &session->scoreTextObj;
    session->screenRenderList[3] = &session->hiScoreTextObj;
    session->screenRenderList[4] = &session->titleTextObj;
    session->screenRenderList[5] = &session->scoreNumObj;
    session->screenRenderList[6] = &session->hiScoreNumObj;
    session->totalRenderList = sizeof(session->screenRenderList) / sizeof(session->screenRenderList[0]);

    session->snake1.pointList.points = malloc(session->snake1.pointList.amt * sizeof(Point));
    session->snake1.pointList.points->posX = 20;
    session->snake1.pointList.points->posY = 10;

    session->normalFruitSet.fruitType = &session->baseFruit;
    session->normalFruitSet.pointList.points = malloc(session->normalFruitSet.pointList.amt * sizeof(Point));
    for (int i = 0; i < session->normalFruitSet.pointList.amt; i++)
        spawnFruit(session->normalFruitSet.pointList, i, session->snake1.pointList, GAME_WIDTH - 41, GAME_HEIGHT - 1);
    enable_raw_mode();
    return 0;
}

int gameRunLoop(GameSession *session) {
    printf("\033[?25l");
    while (session->isRunning) {
        gameUpdateLogic(session);
        renderFrame(session->cell, session->screenRenderList, session->totalRenderList);
        printf("W - UP, S - DOWN, A - LEFT, D - RIGHT\n");
        fflush(stdout);
        usleep(100000);
    }
    return 0;
}

int gameCleanUp(GameSession *session) {
    free(session->snake1.pointList.points);
    free(session->normalFruitSet.pointList.points);
    disable_raw_mode();
    printf("\033[?25h");
    return 0;
}

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

int hiScoreCounter(int currentScore, int *highestScore) {
    if (currentScore > *highestScore)
        *highestScore = currentScore;
    return *highestScore;
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

void gameUpdateLogic(GameSession *session) {
    int moveBlocked = movementInputHandler(session->lastDirInput);
    int collideWithFruit = collisionCheckOne(&session->normalFruitSet.pointList, &session->snake1.pointList, 0, 0);
    if (collideWithFruit > 0)
        handleFruitCollision(&session->normalFruitSet, &session->snake1, collideWithFruit);

    shiftSnakeSegments(&session->snake1);
    snakeDirectionHandler(session->snake1.pointList.points, session->lastDirInput);
    session->snake1.alive = checkLife(session->snake1.pointList, GAME_WIDTH - 1, HALF_GAMEVIEW_BOUNDARY);

    if (!(session->snake1.alive) && !moveBlocked && !(session->lastDirInput->posX == 0 && session->lastDirInput->posY == 0))
        handleDeath(&session->snake1, &session->normalFruitSet.pointList, session->lastDirInput);
    session->scoreNumbers[0].score = session->snake1.pointList.amt;
    session->scoreNumbers[1].score = hiScoreCounter(session->snake1.pointList.amt, &session->currentHiScore);
}
