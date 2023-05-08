#include <mpi.h>
#include "funcs.c"
#include "mpiFuncs.c"

int main(int argc, char *argv[]) {
	int rank, size, numPictures, numObjects, choice=0;
	double match_val, start;
	struct pic_obj *Pictures, *Objects;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if(size != 2) {
		printf("launch 2 processes only\n");
		MPI_Abort(MPI_COMM_WORLD, 0);
	}
	
	if(rank == MASTER)
		readFromFile(&match_val, &numPictures, &numObjects, &Pictures, &Objects);
	
	while(choice != 4) {
		if(rank == MASTER) {
			printf( "Hi, Welcome\nEnter your choice:\n\t1 - sequential\n\t2 - MPI + OpenMP\n\t3 - MPI + OpenMP + CUDA\n\t4 - Exit\nChoice: ");
			scanf("%d", &choice); 
			printf("\n");
		}
		
		MPI_Bcast(&choice, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		
		if(rank == MASTER)
			start = MPI_Wtime();
			
		if(choice == 1)	{
			if(rank == MASTER) {
				seq(match_val, numPictures, numObjects, Pictures, Objects);
				printf("time: %f sec\n\n", MPI_Wtime() - start);
			}
		}
		else if(choice == 2) {
			mpi_omp_cuda(rank, match_val, numPictures, numObjects, Pictures, Objects, 0);
			if(rank == MASTER)
				printf("time: %f sec\n\n", MPI_Wtime() - start);
		}
		else if(choice == 3) {
			mpi_omp_cuda(rank, match_val, numPictures, numObjects, Pictures, Objects, 1);
			if(rank == MASTER)
				printf("time: %f sec\n\n", MPI_Wtime() - start);
		}
		else if(choice == 4) {
			if(rank == MASTER)
				printf("Goodbye and have a nice day:)\n");
		}
		else if(rank == MASTER)
			printf("Wrong Input! Try Again..\n\n");
	}
	
	MPI_Finalize();
	return 0;
}
