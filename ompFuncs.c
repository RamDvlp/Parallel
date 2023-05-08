#pragma once

#include <omp.h>
#include "funcs.c"
#include "structs.h"

void ompFindMatch(const int match_val, const int numObjects, struct pic_obj pic, struct pic_obj *Objects, int **result) {
	int flag = 0, obj;
	(*result)[0] = pic.id; (*result)[1] = -1; (*result)[2] = -1; (*result)[3] = -1;	
	double matchRes;
	
	#pragma omp parallel private(obj, matchRes)
	{	
		#pragma omp for
		for(obj = 0; obj < numObjects; obj++) {
			for(int i = 0; i < pic.N && !flag; i++) {
				for(int j = 0; j < pic.N && !flag; j++) {
					matchRes = Matching(i, j, pic, Objects[obj]);
					if(matchRes <= match_val && matchRes >= 0) {
						#pragma omp critical
						{
							if(!flag) {
								(*result)[1] = Objects[obj].id; (*result)[2] = i; (*result)[3] = j;
								flag = 1;
							}
						}
					}
				}
			}
		}
	}
}
