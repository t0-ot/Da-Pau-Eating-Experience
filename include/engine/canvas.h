#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>
#include <string.h>
#define GAME_WIDTH 80
#define GAME_HEIGHT 20
#define MAX_STATES 2

typedef struct {
    uint8_t r, g, b;
} RGB;

typedef struct {
    int8_t posX;
    int8_t posY;
} Point;

typedef struct {
    char sym;
    RGB color;
} Canvas;

typedef void(*StampFunc)(void *data, Canvas cellView[][GAME_WIDTH]);

typedef struct {
    void *data;
    StampFunc stamp;
} RenderObj;

typedef struct {
    char *text;
    int8_t startX;
    int8_t startY;
    RGB color[MAX_STATES];
} BaseText;

typedef struct {
    int16_t score;
    int8_t x;
    int8_t y;
} ScoreText;

typedef struct {
    BaseText textData;
    bool isSelected;
    int8_t optionAmt;
} OptionText;

void canvasIntializer(Canvas cellView[][GAME_WIDTH], int dividerPos);
void optionChoiceStamper(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasTextStamperGradient(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasTextStamper(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasScoreStamper(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasViewPrint(Canvas cellView[][GAME_WIDTH]);
void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, uint8_t r, uint8_t g, uint8_t b);

#endif // CANVAS_H
