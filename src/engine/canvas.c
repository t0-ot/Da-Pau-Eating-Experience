#include <stdio.h>
#include "../../include/engine/canvas.h"

void canvasScoreStamper(void *data, Canvas cellView[][GAME_WIDTH]) {
    ScoreText *scoreDat = (ScoreText *)data;
    int score = scoreDat->score - 2;
    if (score < 0)
        score = 0;

    int hundreds = (score / 100) % 10;
    int tens     = (score / 10) % 10;
    int units    = score % 10;

    cellColorAssignment(cellView, scoreDat->x,     scoreDat->y, hundreds + '0', 255, 215, 0);
    cellColorAssignment(cellView, scoreDat->x + 1, scoreDat->y, tens     + '0', 255, 215, 0);
    cellColorAssignment(cellView, scoreDat->x + 2, scoreDat->y, units    + '0', 255, 215, 0);
}

void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, uint8_t r, uint8_t g, uint8_t b) {
    cellView[y][x].sym = ch;
    cellView[y][x].color.r = r;
    cellView[y][x].color.g = g;
    cellView[y][x].color.b = b;
}

void canvasIntializer(Canvas cellView[][GAME_WIDTH], int dividerPos) {
    int x, y;

    for (y = 0; y < GAME_HEIGHT; y++)
        for (x = 0; x < GAME_WIDTH; x++) {
            if (x == 0 || x == dividerPos || y == 0 || y == GAME_HEIGHT - 1 || x == GAME_WIDTH - 1) {
                cellColorAssignment(cellView, x, y, '#', 255, 105, 180);
            } else if (x < dividerPos) {
                cellColorAssignment(cellView, x, y, '.', 255, 182, 193);
            } else {
                cellColorAssignment(cellView, x, y, ' ', 255, 255, 255);
            }
        }
}

void optionChoiceStamper(void *data, Canvas cellView[][GAME_WIDTH]) {
    OptionText *optionChoice = (OptionText *)data;

    for (int k = 0; k < optionChoice->optionAmt; k++) {
        int len = strlen(optionChoice[k].textData.text);
        int hoverState = optionChoice[k].isSelected;
        int r = optionChoice[k].textData.color[hoverState].r;
        int g = optionChoice[k].textData.color[hoverState].g;
        int b = optionChoice[k].textData.color[hoverState].b;
        for (int i = 0; i < len; i++)
            cellColorAssignment(cellView, optionChoice[k].textData.startX+i, optionChoice[k].textData.startY, optionChoice[k].textData.text[i], r, g, b);
        if (!(hoverState))
            continue;
        BaseText hoverArrow[2] = {
            [0].text = ">",
            [1].text = "<",
            [0 ... 1].color = { {255, 255, 255} },
            [0 ... 1].startX = optionChoice[k].textData.startX,
            [0 ... 1].startY = optionChoice[k].textData.startY
        };
        hoverArrow[0].startX -= 3;
        hoverArrow[1].startX += len+2;
        for (int j = 0; j < 2; j++)
            canvasTextStamper(&hoverArrow[j], cellView);
    }
}

void canvasTextStamperGradient(void *data, Canvas cellView[][GAME_WIDTH]) {
    BaseText *text = (BaseText *)data;
    int len = strlen(text->text);

    for (int i = 0; i < len; i++) {
        int r = text->color[0].r + (text->color[1].r - text->color[0].r) * i / (len - 1);
        int g = text->color[0].g + (text->color[1].g - text->color[0].g) * i / (len - 1);
        int b = text->color[0].b + (text->color[1].b - text->color[0].b) * i / (len - 1);
        cellColorAssignment(cellView, text->startX+i, text->startY, text->text[i], r, g, b);
    }
}

void canvasTextStamper(void *data, Canvas cellView[][GAME_WIDTH]) {
    BaseText *text = (BaseText *)data;
    int len = strlen(text->text);

    for (int i = 0; i < len; i++)
        cellColorAssignment(cellView, text->startX+i, text->startY, text->text[i], text->color[0].r, text->color[0].g, text->color[0].b);
}

void canvasViewPrint(Canvas cellView[][GAME_WIDTH]) {
    int bufferPos = 0;
    char buffer[(GAME_WIDTH * 30) * GAME_HEIGHT];
    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++)
            bufferPos += sprintf(buffer + bufferPos, "\033[38;2;%d;%d;%dm%c\033[0m", cellView[y][x].color.r, cellView[y][x].color.g, cellView[y][x].color.b, cellView[y][x].sym);
        sprintf(buffer + bufferPos, "\n");
        bufferPos++;
    }
    fwrite(buffer, sizeof(char), bufferPos, stdout);
}
