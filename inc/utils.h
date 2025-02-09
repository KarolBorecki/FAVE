#ifndef FAVE_UTILS_H
#define FAVE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "definitions.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// void getSciColor(float val, float minVal, float maxVal, float color[3]);
char *loadResourceFileContent(const char *filename);

double clampd(double d, double min, double max);
float clampf(float d, float min, float max);
int clamp(int d, int min, int max);

float random_float(float min, float max);

#endif // FAVE_UTILS_H