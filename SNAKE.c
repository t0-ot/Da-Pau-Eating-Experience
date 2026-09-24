#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#define GAME_WIDTH 80
#define GAME_HEIGHT 20

DWORD original_console_mode;

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
int charInputHandler(Point prevDirInput[]);
int snakeDirectionHandler(Point snake[], Point lastDir[]);
void printGradient(const char *text, int startR, int startG, int startB, int endR, int endG, int endB);
void disable_raw_mode();
void spawnFruit(Point fruit[], int spawnWidth, int spawnHeight, int fruitNumber);
void scoreDisplay(int currentPoints, int spacing);
int mainMenu();
int snakePlayer();
void canvasIntializer(Canvas cellView[][GAME_WIDTH]);
void canvasViewPrint(Canvas cellView[][GAME_WIDTH]);
void canvasStamper(Canvas cellView[][GAME_WIDTH], Point snake[], Point fruit[], int snakeLength, int fruitAmount);
void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b);
void canvasTextStamper(const char *text, Canvas cellView[][GAME_WIDTH], int x, int y, int startR, int startG, int startB, int endR, int endG, int endB);
void canvasScoreStamper(Canvas cellView[][GAME_WIDTH], int x, int y, int currentPoints);

int main(void) {
    srand((unsigned int)time(NULL));
    int currentHiScore = 0;
    Canvas cell[GAME_HEIGHT][GAME_WIDTH];
    Point snake[800] = {{20, 10}};
    Point fruit[50];
    Point lastDirInput[1] = {{0, 0}};
    int snakeLength = 2;
    int fruitAmount = 1;
    int i = 0;

    switch (mainMenu()) {
        case 2:
            printf("Thanks for launching me\n");
            return 0;
        case 67:
            enable_raw_mode();

            spawnFruit(fruit, GAME_WIDTH - 41, GAME_HEIGHT - 1, 1);
            while (1) {
                printf("\033[H");

                charInputHandler(lastDirInput);
                for (i = 0; i < fruitAmount; i++) {
                    if (snake[0].posX == fruit[i].posX && snake[0].posY == fruit[i].posY) {
                        snakeLength++;
                        spawnFruit(fruit, GAME_WIDTH - 41, GAME_HEIGHT - 1, fruitAmount);
                        if (collsionCheck(snake, fruit, snakeLength, fruitAmount, 1, 0))
                            spawnFruit(fruit, GAME_WIDTH - 41, GAME_HEIGHT - 1, fruitAmount);
                    }
                }

                for (i = snakeLength - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }

                if (!(snakeDirectionHandler(snake, lastDirInput)) || (collsionCheck(snake, snake, snakeLength, 0, 0, 1) && !(charInputHandler(lastDirInput)))) {
                    snake[0].posX = 20;
                    snake[0].posY = 10;
                    lastDirInput[0].posY = 0;
                    lastDirInput[0].posX = 0;
                    snakeLength = 2;
                    fruitAmount = 1;
                }

                canvasIntializer(cell);
                canvasStamper(cell, snake, fruit, snakeLength, fruitAmount);
                canvasTextStamper("Da Pau Eating Experience", cell, 48, 3, 255, 105, 180, 255, 215, 0);
                canvasTextStamper("-== SCORE ==-", cell, 53, 9, 255, 215, 0, 255, 215, 0);
                canvasTextStamper("-+== HI-SCORE ==+-", cell, 50, 13, 255, 215, 0, 255, 215, 0);
                canvasScoreStamper(cell, 58, 8, snakeLength);
                canvasScoreStamper(cell, 58, 12, hiScoreCounter(snakeLength, &currentHiScore));
                canvasViewPrint(cell);
                printf("W - UP, S - DOWN, A - LEFT, D - RIGHT");

                fflush(stdout);
                Sleep(50);
            }
            disable_raw_mode();

            break;
        default:
            printf("Bad input! Please Enter a valid number\n");
            return 0;
    }
    return 0;
}

int mainMenu() {
    int menuChoice;
    printf("=== Da Pau Eating Experience ===\n");
    printf("67. Play Snake\n");
    printf("2. Quit\n");
    printf("What would you like to do?: ");
    if (scanf("%d", &menuChoice) != 1) {
        return 0;
    }
    return menuChoice;
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
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

    // Enable ANSI escape sequences for Windows Console
    DWORD outMode = 0;
    GetConsoleMode(hOut, &outMode);
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, outMode);

    // Disable line buffering and echo on input
    GetConsoleMode(hIn, &original_console_mode);
    DWORD inMode = original_console_mode;
    inMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hIn, inMode);
}

void disable_raw_mode() {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hIn, original_console_mode);
}

int charInputHandler(Point prevDirInput[]) {
    char latest_ch = 0;
    int dx = 0, dy = 0;

    // Flush key buffer and grab the latest key pressed
    while (_kbhit()) {
        latest_ch = (char)_getch();
    }

    if (latest_ch == 0) return 0;

    switch (latest_ch) {
        case 'w': case 'W': dx =  0; dy = -1; break;
        case 's': case 'S': dx =  0; dy =  1; break;
        case 'a': case 'A': dx = -1; dy =  0; break;
        case 'd': case 'D': dx =  1; dy =  0; break;
        default:  return 1;
    }

    if (prevDirInput[0].posX * -1 == dx && prevDirInput[0].posY * -1 == dy)
        return 1;

    prevDirInput[0].posX = dx;
    prevDirInput[0].posY = dy;
    return 0;
}

int snakeDirectionHandler(Point snake[], Point lastDir[]) {
    if (snake[0].posX + lastDir[0].posX > 0 && snake[0].posX + lastDir[0].posX < GAME_WIDTH - 41 &&
        snake[0].posY + lastDir[0].posY > 0 && snake[0].posY + lastDir[0].posY < GAME_HEIGHT - 1) {
        snake[0].posX += lastDir[0].posX;
        snake[0].posY += lastDir[0].posY;
        return 1;
    } else {
        return 0;
    }
}

void spawnFruit(Point fruit[], int spawnWidth, int spawnHeight, int fruitNumber) {
    int i = 0;
    for (i = 0; i < fruitNumber; i++) {
        fruit[i].posX = (rand() % (spawnWidth - 2)) + 1;
        fruit[i].posY = (rand() % (spawnHeight - 2)) + 1;
    }
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
    return 0;
}

void cellColorAssignment(Canvas cellView[][GAME_WIDTH], int x, int y, char ch, int r, int g, int b) {
    cellView[y][x].sym = ch;
    cellView[y][x].r = r;
    cellView[y][x].g = g;
    cellView[y][x].b = b;
}

void canvasIntializer(Canvas cellView[][GAME_WIDTH]) {
    int x, y;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            if (x == 0 || x == GAME_WIDTH - 41 || y == 0 || y == GAME_HEIGHT - 1 || x == GAME_WIDTH - 1) {
                cellColorAssignment(cellView, x, y, '#', 255, 105, 180);
            } else if (x < GAME_WIDTH - 41) {
                cellColorAssignment(cellView, x, y, '.', 255, 182, 193);
            } else if (x >= GAME_WIDTH - 41) {
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
    int len = (int)strlen(text);
    int i = 0;

    for (i = 0; i < len; i++) {
        int r = startR + (endR - startR) * i / (len > 1 ? len - 1 : 1);
        int g = startG + (endG - startG) * i / (len > 1 ? len - 1 : 1);
        int b = startB + (endB - startB) * i / (len > 1 ? len - 1 : 1);

        cellColorAssignment(cellView, x + i, y, text[i], r, g, b);
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
