#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "structs.h"

#define INPUT_FILE_NAME "./input __ NxN lines with 1 member.txt"
#define OUTPUT_FILE_NAME "./output.txt"
#define MASTER 0
#define SLAVE 1

void readFromFile(double* match_val, int* numPictures, int* numObjects, struct pic_obj** Pictures, struct pic_obj** Objects);
void readPicsOrObjsFromFile(FILE* fp, int* numOf, struct pic_obj** lst);
void readPicOrObjFromFile(FILE* fp, struct pic_obj* p_o);
void printResultToFile(const int picId, const int objId, const int i, const int j);
void printAll(const int numPics, const int numObjs, const struct pic_obj* Pics, const struct pic_obj* Objs);
double Matching(const int i, const int j, const struct pic_obj pic, const struct pic_obj obj);
void seq(const int match_val, const int numPictures, const int numObjects, const struct pic_obj *Pictures, const struct pic_obj *Objects);

void readFromFile(double* match_val, int* numPictures, int* numObjects, struct pic_obj** Pictures, struct pic_obj** Objects) {
	FILE* fp;

	if ((fp = fopen(INPUT_FILE_NAME, "r")) == 0) { // Open file
		printf("cannot open file %s for reading\n", INPUT_FILE_NAME);
		exit(0);
	}
	
	fscanf(fp, "%lf", match_val);
	
	readPicsOrObjsFromFile(fp, numPictures, Pictures);
	readPicsOrObjsFromFile(fp, numObjects, Objects);
	
	fclose(fp);
}

void readPicsOrObjsFromFile(FILE* fp, int* numOf, struct pic_obj** lst) {
	fscanf(fp, "%d", numOf);	
	*lst = (struct pic_obj*)malloc((*numOf) * sizeof(struct pic_obj));
	if (*lst == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	for(int i = 0; i < *numOf; i++)
		readPicOrObjFromFile(fp, *lst + i);
}

void readPicOrObjFromFile(FILE* fp, struct pic_obj* p_o) {
	int id, N, *mat;
	
	fscanf(fp, "%d", &id);
	p_o->id = id;
	
	fscanf(fp, "%d", &N);
	p_o->N = N;
	
	mat = (int*)malloc(N*N * sizeof(int));
	if (mat == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	
	for(int i = 0; i < N*N; i++)
		fscanf(fp, "%d", mat+i);
	p_o->mat = mat;
}

void printResultToFile(const int picId, const int objId, const int i, const int j) {
	FILE* fp;
	
	if ((fp = fopen(OUTPUT_FILE_NAME, "a")) == 0) { // Open file
		printf("cannot open file %s for writing\n", OUTPUT_FILE_NAME);
		exit(0);
	}
	
	if(objId == -1) {
		printf("Picture %d no Objects were found\n", picId);
		fprintf(fp, "Picture %d no Objects were found\n", picId);
	}
	else {
	 	printf("Picture %d found Object %d in Position(%d,%d)\n", picId, objId, i, j);
		fprintf(fp, "Picture %d found Object %d in Position(%d,%d)\n", picId, objId, i, j);
	}
	
	fclose(fp);
}

void printAll(const int numPics, const int numObjs, const struct pic_obj* Pics, const struct pic_obj* Objs) {
	printf("%d Pictures:\n", numPics);
	printf("\t[\n");
	for(int img = 0; img < numPics; img++) {
		printf("\t\tPicture id=%d: N=%d\n", Pics[img].id, Pics[img].N);
		printf("\t\t[\n");
		for(int i = 0; i < Pics[img].N; i++) {
			printf("\t\t\t");
			for(int j = 0; j < Pics[img].N; j++)
				printf("%d ", Pics[img].mat[i*Pics[img].N + j]);
			printf("\n");
			break;
		}
		
		printf("\t\t]\n");	
	}
	printf("\t]\n\n");
	
	printf("%d Objects:\n", numObjs);
	printf("\t[\n");
	for(int obj = 0; obj < numObjs; obj++) {
		printf("\t\tObject id=%d: N=%d\n", Objs[obj].id, Objs[obj].N);
		printf("\t\t[\n");
		for(int i = 0; i < Objs[obj].N; i++) {
			printf("\t\t\t");
			for(int j = 0; j < Objs[obj].N; j++)
				printf("%d ", Objs[obj].mat[i*Objs[obj].N + j]);
			printf("\n");
		}
		
		printf("\t\t]\n");	
	}
	printf("\t]\n\n");
}

double Matching(const int i, const int j, const struct pic_obj pic, const struct pic_obj obj) {
	if(i+obj.N > pic.N || j+obj.N > pic.N)
		return -1;
	
	double sum = 0;
	for(int k = 0; k < obj.N; k++)
		for(int t = 0; t < obj.N; t++)
			sum += fabs((double)(pic.mat[(i+k)*pic.N + j+t] - obj.mat[k*obj.N + t]) / (double)pic.mat[(i+k)*pic.N + j+t]);
	
	return sum;
}

void seq(const int match_val, const int numPictures, const int numObjects, const struct pic_obj *Pictures, const struct pic_obj *Objects) {	
	int flag;
	double res;
	for(int img = 0; img < numPictures; img++) {
		flag = 0;
		for(int obj = 0; obj < numObjects; obj++) {
			for(int i = 0; i < Pictures[img].N && !flag; i++) {
				for(int j = 0; j < Pictures[img].N && !flag; j++) {
					res = Matching(i, j, Pictures[img], Objects[obj]);
					if(res <= match_val && res >= 0) {
						printf("Picture %d found Object %d in Position(%d,%d)\n", Pictures[img].id, Objects[obj].id, i, j);
						flag = 1;
					}
				}
			}			
		}
		
		if(!flag)
			printf("Picture %d no Objects were found\n", Pictures[img].id);
	}
}
