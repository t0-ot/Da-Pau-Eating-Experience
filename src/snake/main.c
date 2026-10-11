#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define GAME_NAME "Da Pau Eating Experience"
#define MAX_STATES 2
#define GAME_WIDTH 80
#define GAME_HEIGHT 20
#define HALF_GAMEVIEW_BOUNDARY (GAME_WIDTH / 2) - 1

struct termios original_term;

typedef struct {
    int r, g, b;
} RGB;

typedef struct {
    int posX;
    int posY;
} Point;

typedef struct {
    char sym;
    RGB color;
} Canvas;

typedef struct {
    int scoreAmt;
} defaultFruit;

typedef struct {
    int points;
    int thickness;
} fatFruit;

typedef struct {
    int points;
    int *speed;
} speedFruit;

typedef struct {
    Point *points;
    int *amt;
} ViewPointList;

typedef struct {
    Point segments[800];
    int length;
    int alive;
} snakeData;

typedef struct {
    Point position[100];
    Canvas visuals;
    int quantity;
    void *fruitType;
    void (*onEat)(void *fruit, snakeData snake);
} fruitData;

typedef struct {
    char *text;
    int startX;
    int startY;
    RGB color[MAX_STATES];
} BaseText;

typedef struct {
    BaseText textData;
    int isSelected;
    int optionAmt;
} OptionText;

typedef void(*StampFunc)(void *data, Canvas cellView[][GAME_WIDTH]);

typedef struct {
    void *data;
    StampFunc stamp;
} RenderObj;

int collisionCheckBoth(ViewPointList objView1, ViewPointList objView2, int countFrom);
int collisionCheckOne(ViewPointList objView1, ViewPointList objView2, int countFrom, int checkIndexObj2);
int hiScoreCounter(int currentScore, int *highestScore);
void enable_raw_mode();
int movementInputHandler(Point prevDirInput[]);
int snakeDirectionHandler(Point snake[], Point lastDir[]);
void disable_raw_mode();
void spawnFruit(ViewPointList fruitView, int fruitNumber, ViewPointList snakeView, int spawnWidth, int spawnHeight);
void scoreDisplay(int currentPoints, int spacing);
int mainMenu(BaseText *gameTitle);
int menuInputHandler(int *menuChoice, int optionAmount);
void canvasIntializer(Canvas cellView[][GAME_WIDTH], int dividerPos);
void shiftSnakeSegments(snakeData *snakeInfo);
int checkLife(ViewPointList snakeView, int playWidthBoundary, int playHeightBoundary);
void snakeStamper(void *data, Canvas cellview[][GAME_WIDTH]);
void fruitStamper(void *data, Canvas cellview[][GAME_WIDTH]);
void optionChoiceStamper(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasViewPrint(Canvas cellView[][GAME_WIDTH]);
void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b);
void canvasTextStamperGradient(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasTextStamper(void *data, Canvas cellView[][GAME_WIDTH]);
void canvasScoreStamper(Canvas cellView[][GAME_WIDTH], int x, int y, int currentPoints);

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
    snakeData snake1 = {
        .segments[0] = (Point){20, 10},
        .length = 2,
        .alive = 1
    };
    ViewPointList snakeView = { .points = snake1.segments, .amt = &snake1.length};
    RenderObj snakeObj1 = {
        .data = &snake1,
        .stamp = snakeStamper
    };

    defaultFruit baseFruit = { .scoreAmt = 1};
    fruitData normalFruitSet = {
        .fruitType = &baseFruit,
        .quantity = 1,
        .visuals = { .sym = 'P', .color.r =  0, .color.g = 255, .color.b = 0}
    };
    ViewPointList normalFruitView = { .points = normalFruitSet.position, .amt = &normalFruitSet.quantity};
    RenderObj normFruitSetObj = {
        .data = &normalFruitSet,
        .stamp = fruitStamper
    };
    Point lastDirInput[1] = {{0, 0}};
    switch (mainMenu(&titleText)) {
        case 1:
            return 0;
        case 0:
            enable_raw_mode();
            for (int i = 0; i < normalFruitSet.quantity; i++) {
                spawnFruit(normalFruitView, i, snakeView, GAME_WIDTH - 41, GAME_HEIGHT - 1);
            }
            while (1) {
                printf("\033[H");

                int moveBlocked = movementInputHandler(lastDirInput);
                int colllideWithFruit = collisionCheckOne(normalFruitView, snakeView, 0, 0);
                if (colllideWithFruit > 0) {
                    snake1.length++;
                    if (((snake1.length - 2) % 10) == 0) {
                        normalFruitSet.quantity++;
                        spawnFruit(normalFruitView, normalFruitSet.quantity-1, snakeView, HALF_GAMEVIEW_BOUNDARY, GAME_HEIGHT - 1);
                    }
                    spawnFruit(normalFruitView, colllideWithFruit-1, snakeView, HALF_GAMEVIEW_BOUNDARY, GAME_HEIGHT - 1);
                }

                shiftSnakeSegments(&snake1);
                snakeDirectionHandler(snake1.segments, lastDirInput);
                snake1.alive = checkLife(snakeView, GAME_WIDTH - 1, HALF_GAMEVIEW_BOUNDARY);

                if (!(snake1.alive) && !moveBlocked) {
                    snake1.segments[0].posX = 20;
                    snake1.segments[0].posY = 10;
                    lastDirInput[0].posY = 0;
                    lastDirInput[0].posX = 0;
                    snake1.length = 2;
                    snake1.alive = 1;
                    normalFruitSet.quantity = 1;
                }

                canvasIntializer(cell, GAME_WIDTH - 41);
                normFruitSetObj.stamp(normFruitSetObj.data, cell);
                snakeObj1.stamp(snakeObj1.data, cell);
                canvasTextStamperGradient(&titleText, cell);
                canvasTextStamper(&scoreTexts[0], cell);
                canvasTextStamper(&scoreTexts[1], cell);
                canvasScoreStamper(cell, 58, 8, snake1.length);
                canvasScoreStamper(cell, 58, 12, hiScoreCounter(snake1.length, &currentHiScore));
                canvasViewPrint(cell);
                printf("W - UP, S - DOWN, A - LEFT, D - RIGHT\n");

                fflush(stdout);
                usleep(100000);
            }
            disable_raw_mode();
            break;
    }
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

int collisionCheckBoth(ViewPointList objView1, ViewPointList objView2, int countFrom) {
    for (int i = countFrom; i < *objView1.amt; i++) {
        for (int k = 0; k < *objView2.amt; k++) {
            if (objView1.points[i].posX == objView2.points[k].posX && objView1.points[i].posY == objView2.points[k].posY)
                return 1;
        }
    }
    return 0;
}

int collisionCheckOne(ViewPointList objView1, ViewPointList objView2, int countFrom, int checkIndexObj2) {
    for (int i = countFrom; i < *objView1.amt; i++) {
        if (objView1.points[i].posX == objView2.points[checkIndexObj2].posX && objView1.points[i].posY == objView2.points[checkIndexObj2].posY)
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
    int dx = 0, dy = 0;
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
    for (int i = snakeInfo->length - 1; i > 0; i--) {
        snakeInfo->segments[i] = snakeInfo->segments[i-1];
    }
}

int checkLife(ViewPointList snakeView, int playWidthBoundary, int playHeightBoundary) {
    if (snakeView.points[0].posX == 0 || snakeView.points[0].posY == 0 || snakeView.points[0].posX == playWidthBoundary || snakeView.points[0].posY == playHeightBoundary)
        return 0;
    if (collisionCheckOne(snakeView, snakeView, 1, 0))
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
    } while (collisionCheckOne(snakeView, fruitView, 0, fruitNumber));
}

void baseEat(void *fruit, snakeData *snake) {
    fruitData *fruitInfo = (fruitData *)fruit;
    defaultFruit *baseFruit = (defaultFruit *)fruitInfo->fruitType;
    snake->length++;
}

void canvasScoreStamper(Canvas cellView[][GAME_WIDTH], int x, int y, int currentPoints) {
    int score = currentPoints - 2;
    if (score < 0)
        score = 0;

    int hundreds = (score / 100) % 10;
    int tens     = (score / 10) % 10;
    int units    = score % 10;

    cellColorAssignment(cellView, x,     y, hundreds + '0', 255, 215, 0);
    cellColorAssignment(cellView, x + 1, y, tens     + '0', 255, 215, 0);
    cellColorAssignment(cellView, x + 2, y, units    + '0', 255, 215, 0);
}

void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b) {
    cellView[y][x].sym = ch;
    cellView[y][x].color.r = r;
    cellView[y][x].color.g = g;
    cellView[y][x].color.b = b;
}

void canvasIntializer(Canvas cellView[][GAME_WIDTH], int dividerPos) {
    int x, y;

    for (y = 0; y < GAME_HEIGHT; y++) {
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
}

void snakeStamper(void *data, Canvas cellview[][GAME_WIDTH]) {
    snakeData *p = (snakeData *)data;
    for (int i = p->length - 1; i >= 0; i--) {
        cellview[p->segments[i].posY][p->segments[i].posX].color.r = 0;
        cellview[p->segments[i].posY][p->segments[i].posX].color.g = 255;
        cellview[p->segments[i].posY][p->segments[i].posX].color.b = 0;
        if (i == 0)
            cellview[p->segments[i].posY][p->segments[i].posX].sym = 'M';
        else
            cellview[p->segments[i].posY][p->segments[i].posX].sym = 'O';
    }
}

void fruitStamper(void *data, Canvas cellview[][GAME_WIDTH]) {
    fruitData *p = (fruitData *)data;
    for (int i = 0; i < p->quantity; i++) {
        cellview[p->position[i].posY][p->position[i].posX].sym = p->visuals.sym;
        cellview[p->position[i].posY][p->position[i].posX].color.r = p->visuals.color.r;
        cellview[p->position[i].posY][p->position[i].posX].color.g = p->visuals.color.g;
        cellview[p->position[i].posY][p->position[i].posX].color.b = p->visuals.color.b;
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
    int x, y;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            printf("\033[38;2;%d;%d;%dm%c\033[0m", cellView[y][x].color.r, cellView[y][x].color.g, cellView[y][x].color.b, cellView[y][x].sym);
        }
        printf("\n");
    }
}
