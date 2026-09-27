#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define GAME_WIDTH 80
#define GAME_HEIGHT 20

struct termios original_term;

typedef struct {
    int posX;
    int posY;
} Point;

typedef struct {
    char sym;
    int r;
    int g;
    int b;
} Canvas;

int collsionCheck(Point object1[], Point object2[], int objCountAmt1, int objCountAmt2, int incrementBoth, int countFrom);
int hiScoreCounter(int currentScore, int *highestScore);
void enable_raw_mode();
int movementInputHandler(Point prevDirInput[]);
int snakeDirectionHandler(Point snake[], Point lastDir[]);
void printGradient(const char *text, int startR, int startG, int startB, int endR, int endG, int endB);
void disable_raw_mode();
void spawnFruit(Point fruit[], Point snake[], int snakeLength, int spawnWidth, int spawnHeight, int fruitNumber);
void scoreDisplay(int currentPoints, int spacing);
int mainMenu();
int menuInputHandler(int *menuChoice, int optionAmount);
int snakePlayer();
void canvasIntializer(Canvas cellView[][GAME_WIDTH], int dividerPos);
void canvasViewPrint(Canvas cellView[][GAME_WIDTH]);
void canvasStamper(Canvas cellView[][GAME_WIDTH], Point snake[], Point fruit[], int snakeLength, int fruitAmount);
void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b);
void canvasTextStamper(const char *text, Canvas cellView[][GAME_WIDTH], int x, int y, int startR, int startG, int startB, int endR, int endG, int endB);
void canvasScoreStamper(Canvas cellView[][GAME_WIDTH], int x, int y, int currentPoints);

int main(void) {
    srand(time(NULL));
    int currentHiScore;
    Canvas cell[GAME_HEIGHT][GAME_WIDTH];
    Point snake[800] = {{20, 10}};
    Point fruit[50] = {0};
    Point lastDirInput[1] = {{0, 0}};
    int snakeLength = 2, fruitAmount = 1, i = 0;
    switch (mainMenu()) {
        case 1:
            return 0;
        case 0:
            enable_raw_mode();
            for (i = 0; i < fruitAmount; i++) {
                spawnFruit(fruit, snake, snakeLength, GAME_WIDTH - 41, GAME_HEIGHT - 1, i);
            }
            while (1) {
                printf("\033[H");

                int moveBlocked = movementInputHandler(lastDirInput);
                for (i = 0; i < fruitAmount; i++) {
                    if (snake[0].posX == fruit[i].posX && snake[0].posY == fruit[i].posY) {
                        snakeLength++;
                        if (((snakeLength - 2) % 10) == 0) {
                            fruitAmount++;
                            spawnFruit(fruit, snake, snakeLength, GAME_WIDTH -41, GAME_HEIGHT - 1, fruitAmount-1);
                        }
                        spawnFruit(fruit, snake, snakeLength, GAME_WIDTH - 41, GAME_HEIGHT - 1, i);
                    }
                }

                for (i = snakeLength - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }

                if (!(snakeDirectionHandler(snake, lastDirInput)) || (collsionCheck(snake, snake, snakeLength, 0, 0, 1) && !moveBlocked)) {
                    snake[0].posX = 20;
                    snake[0].posY = 10;
                    lastDirInput[0].posY = 0;
                    lastDirInput[0].posX = 0;
                    snakeLength = 2;
                    fruitAmount = 1;
                }

                canvasIntializer(cell, GAME_WIDTH - 41);
                canvasStamper(cell, snake, fruit, snakeLength, fruitAmount);
                canvasTextStamper("Da Pau Eating Experience", cell, 48, 3, 255, 105, 180, 255, 215, 0);
                canvasTextStamper("-== SCORE ==-", cell, 53, 9, 255, 215, 0, 255, 215, 0);
                canvasTextStamper("-+== HI-SCORE ==+-", cell, 50, 13, 255, 215, 0, 255, 215, 0);
                canvasScoreStamper(cell, 58, 8, snakeLength);
                canvasScoreStamper(cell, 58, 12, hiScoreCounter(snakeLength, &currentHiScore));
                canvasViewPrint(cell);
                printf("W - UP, S - DOWN, A - LEFT, D - RIGHT");

                fflush(stdout);
                usleep(100000);
            }
            disable_raw_mode();

            break;
        default:
            printf("Bad input! Please Enter a valid number");
            return 0;

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

int mainMenu() {
    Canvas menuCell[GAME_HEIGHT][GAME_WIDTH];
    float time = 0.0f;
    float duration = 1.0f;
    float startPos = 0.0f;
    float targetPos = 40.0f;
    float change = targetPos - startPos;

    int isStart = 0;
    int menuChoice = 0;
    int input = 0;
    enable_raw_mode();
    while (1) {
        printf("\033[H");

        canvasIntializer(menuCell, startPos);
        if (menuInputHandler(&menuChoice, 2))
            break;
        switch (menuChoice) {
            case 0:
                canvasTextStamper("-->", menuCell, GAME_WIDTH / 2 - 11, 10, 255, 255, 255, 255, 255, 255);
                canvasTextStamper("PLAY!", menuCell, GAME_WIDTH / 2 - 3, 10, 255, 255, 255, 255, 255, 255);
                canvasTextStamper("QUIT GAME", menuCell, GAME_WIDTH / 2 - 5, 12, 0, 255, 0, 0, 255, 0);
                break;
            case 1:
                canvasTextStamper("-->", menuCell, GAME_WIDTH / 2 - 11, 12, 255, 255, 255, 255, 255, 255);
                canvasTextStamper("PLAY!", menuCell, GAME_WIDTH / 2 - 3, 10, 0, 255, 0, 0, 255, 0);
                canvasTextStamper("QUIT GAME", menuCell, GAME_WIDTH / 2 - 5, 12, 255, 255, 255, 255, 255, 255);
                break;
        }
        canvasTextStamper("Da Pau Eating Experience", menuCell, GAME_WIDTH / 2 - 12, 3, 255, 105, 180, 255, 215, 0);
        canvasViewPrint(menuCell);
        fflush(stdout);
        usleep(30000);
    }

    if (menuChoice == 1)
        return 1;

    while (time <= duration) {
        printf("\033[H");

        int currentPos = (int)ease(time, startPos, change, duration);

        canvasIntializer(menuCell, currentPos);
        canvasTextStamper("Da Pau Eating Experience", menuCell, (int)ease(time, (GAME_WIDTH / 2 - 12), 21, duration), 3, 255, 105, 180, 255, 215, 0);
        canvasViewPrint(menuCell);

        fflush(stdout);
        usleep(30000);
        time += 0.03f;
    }
    disable_raw_mode();
    return 0;
}

int collsionCheck(Point object1[], Point object2[], int objCountAmt1, int objCountAmt2, int incrementBoth, int countFrom) {
    int i, k;
    if (incrementBoth) {
        for (i = countFrom; i < objCountAmt1; i++) {
            for (k = 0; k < objCountAmt2; k++) {
                if (object1[i].posX == object2[k].posX && object1[i].posY == object2[k].posY)
                    return 1;
            }
        }
    } else {
        for (i = countFrom; i < objCountAmt1; i++) {
            if (object1[i].posX == object2[objCountAmt2].posX && object1[i].posY == object2[objCountAmt2].posY)
                return 1;
        }
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

int menuInputHandler(int *menuChoice, int optionAmount) {
    char latest_ch = 0, temp_ch;
    while (read(STDIN_FILENO, &temp_ch, 1) > 0) {
        latest_ch = temp_ch;
    }

    if (latest_ch == 0) return 0;

    switch (latest_ch) {
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
    char latest_ch = 0, temp_ch;
    int dx = 0, dy = 0;

    while (read(STDIN_FILENO, &temp_ch, 1) > 0) {
        latest_ch = temp_ch;
    }

    if (latest_ch == 0) return 0;

    switch (latest_ch) {
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
    if (snake[0].posX + lastDir[0].posX > 0 && snake[0].posX + lastDir[0].posX < GAME_WIDTH - 41 && snake[0].posY + lastDir[0].posY > 0 && snake[0].posY + lastDir[0].posY < GAME_HEIGHT - 1) {
        snake[0].posX += lastDir[0].posX;
        snake[0].posY += lastDir[0].posY;
        return 1;
    } else
        return 0;
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void spawnFruit(Point fruit[], Point snake[], int snakeLength, int spawnWidth, int spawnHeight, int fruitNumber) {
    do {
        fruit[fruitNumber].posX = (rand() % (spawnWidth - 2)) + 1;
        fruit[fruitNumber].posY = (rand() % (spawnHeight - 2)) + 1;
    } while (collsionCheck(snake, fruit, snakeLength, fruitNumber, 0, 0));
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

int snakePlayer() {

}

void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b) {
    cellView[y][x].sym = ch;
    cellView[y][x].r = r;
    cellView[y][x].g = g;
    cellView[y][x].b = b;
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

void canvasStamper(Canvas cellView[][GAME_WIDTH], Point snake[], Point fruit[], int snakeLength, int fruitAmount) {
    int i;

    for (i = 0; i < snakeLength; i++) {
        if (i == 0)
            cellColorAssignment(cellView, snake[i].posX, snake[i].posY, 'M', 0, 255, 0);
        else
            cellColorAssignment(cellView, snake[i].posX, snake[i].posY, 'O', 0, 255, 0);
    }
    for (i = 0; i < fruitAmount; i++) {
        cellColorAssignment(cellView, fruit[i].posX, fruit[i].posY, 'P', 0, 255, 0);
    }
}

void canvasTextStamper(const char *text, Canvas cellView[][GAME_WIDTH], int x, int y, int startR, int startG, int startB, int endR, int endG, int endB) {
    int len = strlen(text), i = 0;

    for (i = 0; i < len; i++) {
        int r = startR + (endR - startR) * i / (len - 1);
        int g = startG + (endG - startG) * i / (len - 1);
        int b = startB + (endB - startB) * i / (len - 1);

        cellColorAssignment(cellView, x+i, y, text[i], r, g, b);
    }
}

void canvasViewPrint(Canvas cellView[][GAME_WIDTH]) {
    int x, y;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            printf("\033[38;2;%d;%d;%dm%c\033[0m", cellView[y][x].r, cellView[y][x].g, cellView[y][x].b, cellView[y][x].sym);
        }
        printf("\n");
    }
}
