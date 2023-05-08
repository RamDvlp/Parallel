#pragma once

#include <mpi.h>
#include "funcs.c"
#include "ompFuncs.c"
#include "cudaFuncs.h"
#include "structs.h"

void mpi_omp_cuda(const int rank, double match_val, int numPictures, int numObjects, struct pic_obj *Pictures, struct pic_obj* Objects, int isMatchWithCuda);
void mpiMaster(const int match_val, const int numPictures, const int numObjects, struct pic_obj *Pictures, struct pic_obj *Objects, MPI_Status status, int isMatchWithCuda);
int* mpiSlave(const int match_val, const int numPictures, const int numObjects, struct pic_obj *Objects, MPI_Status status, int isMatchWithCuda);
void sendPicOrObjToSlave(struct pic_obj pic_obj);
void recvPicOrObjFromMaster(struct pic_obj *pic_obj, MPI_Status status);

void mpi_omp_cuda(const int rank, double match_val, int numPictures, int numObjects, struct pic_obj *Pictures, struct pic_obj* Objects, int isMatchWithCuda) {	
	int* slaveResult;
	MPI_Status status;
		
	MPI_Bcast(&match_val, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&numPictures, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&numObjects, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	if(rank == MASTER) {
		for(int i = 0; i < numObjects; i++)
			sendPicOrObjToSlave(Objects[i]);
		
		mpiMaster(match_val, numPictures, numObjects, Pictures, Objects, status, isMatchWithCuda);
	}
	else { 		
		Objects = (struct pic_obj*)malloc(numObjects * sizeof(struct pic_obj));
		if (Objects == NULL) {
			printf("Problem to allocate memory\n");
			exit(0);
		}
		
		for(int i = 0; i < numObjects; i++)
			recvPicOrObjFromMaster(Objects+i, status);
		
		slaveResult = mpiSlave(match_val, numPictures, numObjects, Objects, status, isMatchWithCuda);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == MASTER) {
		slaveResult = (int*)malloc((numPictures - numPictures/2)*4 * sizeof(int));
		if (slaveResult == NULL) {
			printf("Problem to allocate memory\n");
			exit(0);
		}	

		MPI_Recv(slaveResult, (numPictures - numPictures/2)*4, MPI_INT, SLAVE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // receive result from slave
		
		for(int i = 0; i < numPictures - numPictures/2; i++, slaveResult += 4)
			printResultToFile(slaveResult[0], slaveResult[1], slaveResult[2], slaveResult[3]);
	}
	else
		MPI_Send(slaveResult, (numPictures - numPictures/2)*4, MPI_INT, MASTER, 0, MPI_COMM_WORLD); // Send sub result to Master
}

void mpiMaster(const int match_val, const int numPictures, const int numObjects, struct pic_obj *Pictures, struct pic_obj *Objects, MPI_Status status, int isMatchWithCuda) {	
	int *result;
	
	for(int i = numPictures/2; i < numPictures; i++)
		sendPicOrObjToSlave(Pictures[i]);
	
	result = (int*)malloc(4 * sizeof(int));
	if (result == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	
	for(int i = 0; i < numPictures/2; i++) {	
		if(isMatchWithCuda && i%2 == 0)
			cudaFindMatch(match_val, numObjects, Pictures[i], Objects, &result);
		else
			ompFindMatch(match_val, numObjects, Pictures[i], Objects, &result);
		
		ompFindMatch(match_val, numObjects, Pictures[i], Objects, &result);
		printResultToFile(result[0], result[1], result[2], result[3]);
	}
}

int* mpiSlave(const int match_val, const int numPictures, const int numObjects, struct pic_obj *Objects, MPI_Status status, int isMatchWithCuda) {
	int* result;
	struct pic_obj* subPictures;
	
	result = (int*)malloc((numPictures - numPictures/2)*4 * sizeof(int));
	if (result == NULL) {
			printf("Problem to allocate memory\n");
			exit(0);
	}
	
	subPictures = (struct pic_obj*)malloc((numPictures - numPictures/2) * sizeof(struct pic_obj));
	if (subPictures == NULL) {
			printf("Problem to allocate memory\n");
			exit(0);
	}
	
	for(int i = 0; i < numPictures - numPictures/2; i++)
		recvPicOrObjFromMaster(subPictures+i, status);
	
	for(int i = 0; i < numPictures - numPictures/2; i++, result += 4) {
		if(isMatchWithCuda && i%2 == 0)
			cudaFindMatch(match_val, numObjects, subPictures[i], Objects, &result);
		else
			ompFindMatch(match_val, numObjects, subPictures[i], Objects, &result);
	}
	
	result -= (numPictures - numPictures/2)*4;
	
	return result;
}

void sendPicOrObjToSlave(struct pic_obj pic_obj) {
	MPI_Send(&(pic_obj.id), 1, MPI_INT, SLAVE, 0, MPI_COMM_WORLD);
	MPI_Send(&(pic_obj.N), 1, MPI_INT, SLAVE, 1, MPI_COMM_WORLD);
	MPI_Send(pic_obj.mat, pic_obj.N * pic_obj.N, MPI_INT, SLAVE, 2, MPI_COMM_WORLD);
}

void recvPicOrObjFromMaster(struct pic_obj *pic_obj, MPI_Status status) {
	MPI_Recv(&(pic_obj->id), 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	MPI_Recv(&(pic_obj->N), 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
	pic_obj->mat = (int*)malloc(pic_obj->N*pic_obj->N * sizeof(int));		
	if (pic_obj->mat == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
		
	MPI_Recv(pic_obj->mat, pic_obj->N*pic_obj->N, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
}
