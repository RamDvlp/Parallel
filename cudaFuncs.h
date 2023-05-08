#pragma once

#include "structs.h"
#define MAX_MAT_WIDTH 32

double cudaMatching(const int i, const int j, const struct pic_obj pic, const struct pic_obj obj);
void cudaFindMatch(const int match_val, const int numObjects, struct pic_obj pic, struct pic_obj *Objects, int **result);
