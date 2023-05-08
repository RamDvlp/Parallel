#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <string.h>
#include "cudaFuncs.h"

__global__ void calc_ij_kernel(const int picN, const int objN, const int* picMat, const int* objMat, double* resMat) {
	for(int i = threadIdx.x; i < picN; i += MAX_MAT_WIDTH) {
		for(int j = threadIdx.y; j < picN; j += MAX_MAT_WIDTH) {
	
			if(i+objN > picN || j+objN > picN)
				resMat[i*picN + j] = -1;	
			else {
				double sum = 0;
				for(int k = 0; k < objN; k++)
					for(int t = 0; t < objN; t++)
						sum += fabs((double)(picMat[(i+k)*picN + j+t] - objMat[k*objN + t]) / (double)picMat[(i+k)*picN + j+t]);
					
				resMat[i*picN + j] = sum;
			}
		}
	}
}

void handleError(cudaError_t err, int msgChoice) {
    	if (err != cudaSuccess) {
	    	if(msgChoice == 1)
        		fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        	if(msgChoice == 2)
        		fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        	if(msgChoice == 3)
	        	fprintf(stderr, "Failed to launch kernel -  %s\n", cudaGetErrorString(err));
        	if(msgChoice == 4)
        		fprintf(stderr, "Failed to copy result array from device to host -%s\n", cudaGetErrorString(err));
        	else
        		fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    	}
}

void cudaFindMatch(const int match_val, const int numObjects, struct pic_obj pic, struct pic_obj *Objects, int **result) {
	int gridSize, flag = 0;
	(*result)[0] = pic.id; (*result)[1] = -1; (*result)[2] = -1; (*result)[3] = -1;
	
    	// Error code to check return values for CUDA calls
	cudaError_t err = cudaSuccess;
  
    	// Allocate memory on GPU to copy the data from the host
    	int *d_picMat;
    	//double *d_matRes, *matRes;
    	    	
    	err = cudaMalloc((void **)&d_picMat, pic.N*pic.N * sizeof(int));
    	handleError(err, 1);
    	    	
    	////////////////////////////////////////////////////////
    	
    	// Copy data from host to the GPU memory
    	err = cudaMemcpy(d_picMat, pic.mat, pic.N*pic.N * sizeof(int), cudaMemcpyHostToDevice);
    	handleError(err, 2);
    	
    	for(int obj = 0; obj < numObjects; obj++) {
    		int *d_objMat;
	    	double *d_matRes, *matRes;
	    	
    		err = cudaMalloc((void **)&d_objMat, Objects[obj].N*Objects[obj].N * sizeof(int));
    		handleError(err, 1);
    		
    		err = cudaMalloc((void **)&d_matRes, pic.N*pic.N * sizeof(double));
	    	handleError(err, 1);
    	
	    	err = cudaMemcpy(d_objMat, Objects[obj].mat, Objects[obj].N*Objects[obj].N * sizeof(int), cudaMemcpyHostToDevice);
	    	handleError(err, 2);

	    	// Launch the Kernel
	    	if(pic.N <= MAX_MAT_WIDTH) {
	    		dim3 dimBlock(pic.N, pic.N);
		    	calc_ij_kernel<<<1, dimBlock>>>(pic.N, Objects[obj].N, d_picMat, d_objMat, d_matRes);
		    	handleError(cudaGetLastError(), 3);
		}
		else {
		    	//gridSize = (pic.N + MAX_MAT_WIDTH - 1) / MAX_MAT_WIDTH;
		    	gridSize = pic.N % MAX_MAT_WIDTH ? pic.N / MAX_MAT_WIDTH + 1 : pic.N / MAX_MAT_WIDTH;
		    	dim3 dimGrid(gridSize, gridSize);
		    	dim3 dimBlock(MAX_MAT_WIDTH, MAX_MAT_WIDTH);
		    	calc_ij_kernel<<<1, dimBlock>>>(pic.N, Objects[obj].N, d_picMat, d_objMat, d_matRes);
		    	handleError(cudaGetLastError(), 3);
	    	}
	    	
	    	matRes = (double*)malloc(pic.N*pic.N * sizeof(double));
	    	if (matRes == NULL) {
			printf("Problem to allocate memory\n");
			exit(0);
		}
    		err = cudaMemcpy(matRes, d_matRes, pic.N*pic.N * sizeof(double), cudaMemcpyDeviceToHost);
		handleError(err, 4);
		
		for(int i = 0; i < pic.N && !flag; i++) {
			for(int j = 0; j < pic.N && !flag; j++) {
				if(matRes[i*pic.N + j] <= match_val && matRes[i*pic.N + j] >= 0) {
					(*result)[1] = Objects[obj].id; (*result)[2] = i; (*result)[3] = j;
					flag = 1;
				}
			}
		}
	    	
	        err = cudaFree(d_objMat);
	        handleError(err, 5);
	        
	        err = cudaFree(d_matRes);
	        handleError(err, 5);
	        
	        free(matRes);
	        
	        if(flag)
	        	break;
	}	 
	
    	// Free allocated memory on GPU
    	err = cudaFree(d_picMat);
    	handleError(err, 5);
}
