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

int collsionCheck(Point object1[], Point object2[], int objCountAmt1, int objCountAmt2, int incrementBoth, int countFrom) {
    int i ;
    int k ;
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

void lastDirInputHandler(Point lastDir[], int numberDirection, int isX) {
    if (isX) {
        lastDir[0].posX = numberDirection;
        lastDir[0].posY = 0;
    } else {
        lastDir[0].posY = numberDirection;
        lastDir[0].posX = 0;
    }
}

int charInputHandler(Point prevDirInput[]) {
    char latest_ch = 0;
    char temp_ch;


    while (read(STDIN_FILENO, &temp_ch, 1) > 0) {
        latest_ch = temp_ch;
    }
    if (latest_ch != 0) {
        switch (latest_ch) {
            case 'w':
                if (prevDirInput[0].posY != 1) {
                    lastDirInputHandler(prevDirInput, -1, 0);
                    return 0;
                    }
            case 's':
                if (prevDirInput[0].posY != -1) {
                    lastDirInputHandler(prevDirInput, 1, 0);
                    return 0;
                }
            case 'a':
                if (prevDirInput[0].posX != 1) {
                    lastDirInputHandler(prevDirInput, -1, 1);
                    return 0;
                }
            case 'd':
                if (prevDirInput[0].posX != -1) {
                    lastDirInputHandler(prevDirInput, 1, 1);
                    return 0;
                }
        }
        return 1;
    }
}

int snakeDirectionHandler(Point snake[], Point lastDir[], int viewportWidth, int viewportHeight) {
    if (snake[0].posX + lastDir[0].posX > 1 && snake[0].posX + lastDir[0].posX < viewportWidth && snake[0].posY + lastDir[0].posY > 1 && snake[0].posY + lastDir[0].posY < viewportHeight) {
        snake[0].posX += lastDir[0].posX;
        snake[0].posY += lastDir[0].posY;
        return 1;
    } else
        return 0;
}

void printGradient(const char *text, int startR, int startG, int startB, int endR, int endG, int endB) {
    int len = strlen(text);
    int i = 0;
    if (len <= 1) {
        printf("\033[38;2;%d;%d;%d%s\033[0m", startR, startG, startB, text);
        return;
    }

    for (i = 0; i < len; i++) {
        int r = startR + (endR - startR) * i / (len - 1);
        int g = startG + (endG - startG) * i / (len - 1);
        int b = startB + (endB - startB) * i / (len - 1);

        printf("\033[38;2;%d;%d;%dm%c", r, g, b, text[i]);
    }
    printf("\033[0m");
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void spawnFruit(Point fruit[], int spawnWidth, int spawnHeight, int fruitNumber) {
    int i = 0;
    for (i = 0; i < fruitNumber; i++) {
        fruit[i].posX = (rand() % (spawnWidth - 2)) + 2;
        fruit[i].posY = (rand() % (spawnHeight - 2)) + 2;
    }
}

void scoreDisplay(int currentPoints, int spacing) {
    if (currentPoints - 2 < 10)
        printf("%*s00%d%*s", spacing, "", currentPoints - 2, spacing, "");
    else if (currentPoints - 2 >= 10 && currentPoints < 100)
        printf("%*s0%d%*s", spacing, "", currentPoints - 2, spacing, "");
    else if (currentPoints - 2 >= 100)
        printf("%*s%d%*s", spacing, "", currentPoints - 2, spacing, "");
}

void boardViewPort(Point snakeBody[], int snakeSegment, Point fruityCoords[], int fruitAmount, int hiScore) {
    int x = 1;
    int y = 1;
    int i = 0;
    int centeringSpaces = 18;

    for (y = 1; y <= GAME_HEIGHT; y++) {
        for (x = 1; x <= GAME_WIDTH; x++) {
            if (x == 1 || x == GAME_WIDTH - 40 || y == 1 || y == GAME_HEIGHT || x == GAME_WIDTH) {
                printf("\033[38;2;255;105;180m#\033[0m");
            } else if (x == 41 && y == 3){
                printGradient("    =+= Da Pau Eating Experience =+=   ", 255, 105, 180, 255, 215, 0);
            } else if (x == 41 && y == 12) {
                scoreDisplay(hiScore, centeringSpaces);

            } else if (x == 41 && y == 13) {
                printf("\033[38;2;255;215;0m          ---- HI-SCORE -----          \033[0m");
            } else if (x == 41 && y == 8) {
                scoreDisplay(snakeSegment, centeringSpaces);

            } else if (x == 41 &&  y == 9) {
                printf("             --- SCORE ---             ");
            } else {
                int isSnake = 0;
                int isFruit = 0;
                int isSnakeHead = 0;
                for (i = 0; i < snakeSegment; i++) {
                    if (snakeBody[i].posX == x && snakeBody[i].posY == y) {
                        isSnake = 1;
                    if (i == 0)
                        isSnakeHead = 1;
                        break;
                    }
                }
                for (i = 0; i < fruitAmount; i++) {
                    if (fruityCoords[i].posX == x && fruityCoords[i].posY == y) {
                        isFruit = 1;
                        break;
                    }
                }

                if (isSnake && !isSnakeHead)
                    printf("\033[38;2;0;255;0mO\033[0m");
                else if (isSnake && isSnakeHead)
                    printf("\033[38;2;0;255;0mM\033[0m");
                else if (isFruit)
                    printf("\033[38;2;0;255;0mP\033[0m");
                else if (x < GAME_WIDTH - 40)
                    printf("\033[38;2;255;182;193m.\033[0m");
                else if (x >= GAME_WIDTH - 40 && y != 3 && y != 12 && y != 13 && y != 9 && y != 8)
                    printf(" ");
            }
        }
        printf("\n");
    }
}


int main(void) {
    srand(time(NULL));
    int currentHiScore;
    Point snake[800];
    Point fruit[50];
    Point lastDirInput[1];
    lastDirInput[1].posX = 0;
    lastDirInput[1].posY = 0;
    int snakeLength = 2;
    int fruitAmount = 1;
    int i = 0;
    int menuChoice;
    snake[0].posX = 20;
    snake[0].posY = 10;
    char ch;
    int isDead = 0;
    printf("=== MARCUS SNAKE GAME ===\n");
    printf("1. Play\n 2.Quit\n");
    printf("What would you like to do?: ");
    scanf("%d", &menuChoice);
    switch (menuChoice) {
        case 1:
            enable_raw_mode();

            while (1) {
                ch = getchar();

                if (ch == 'q') {
                    break;
                }

                if (ch != -1 && ch != 255) {
                printf("%c", ch);
                fflush(stdout);
                }
            }
            disable_raw_mode();
            break;
        case 2:
            printf("Thanks for launching me\n");
            return 0;
        case 67:
            printf("Testmode\n");
            enable_raw_mode();

            spawnFruit(fruit, GAME_WIDTH - 40, GAME_HEIGHT, 1);
            while (1) {
                printf("\033[H");

                charInputHandler(lastDirInput);

                if (snake[0].posX == fruit[0].posX && snake[0].posY == fruit[0].posY) {
                    snakeLength++;
                    spawnFruit(fruit, GAME_WIDTH - 40, GAME_HEIGHT, 1);
                    if (collsionCheck(snake, fruit, snakeLength, fruitAmount, 1, 0))
                        spawnFruit(fruit, GAME_WIDTH - 40, GAME_HEIGHT, 1);
                }

                for (i = snakeLength - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }

                if (!(snakeDirectionHandler(snake, lastDirInput, GAME_WIDTH - 40, GAME_HEIGHT)) || (collsionCheck(snake, snake, snakeLength, 0, 0, 1) && !(charInputHandler(lastDirInput))))
                    isDead = 1;

                if (isDead) {
                    snake[0].posX = 20;
                    snake[0].posY = 10;
                    lastDirInput[0].posY = 0;
                    lastDirInput[0].posX = 0;
                    isDead = 0;
                    snakeLength = 2;
                    fruitAmount = 1;
                }

                boardViewPort(snake, snakeLength, fruit, 1, hiScoreCounter(snakeLength, &currentHiScore));
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
