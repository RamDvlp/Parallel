build:
	mpicxx -fopenmp -c program.c -o program.o -lm
	nvcc -I./inc -c cudaFuncs.cu -o cudaFuncs.o -lm
	mpicxx -fopenmp -o FinalProject program.o cudaFuncs.o /usr/local/cuda/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./FinalProject
