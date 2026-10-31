#include <stdio.h>
#include <stdlib.h>
#include "../../include/engine/utils.h"

float ease(float time, float startValue, float change, float duration) {
     time /= duration / 2;
     if (time < 1)  {
          return change / 2 * time * time + startValue;
     }

     time--;
     return -change / 2 * (time * (time - 2) - 1) + startValue;
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
