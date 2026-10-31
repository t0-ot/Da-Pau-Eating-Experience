#ifndef UTILS_H
#define UTILS_H

#include "../../include/engine/canvas.h"

float ease(float time, float startValue, float change, float duration);
int objectPointResize(ViewPointList *data);
int collisionCheckOne(ViewPointList *objView1, ViewPointList *objView2, int countFrom, int checkIndexObj2);

#endif // UTILS_H
