#include <iostream>
#include <chrono>
#include <mpi.h>

#include "../kernel_process.hpp"
#include "../EasyBMP.h"

int main(int argc, char* argv[]) {
   

	int ierr;
    ierr = MPI_Init(&argc, &argv);

	int id, process_count;
    
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	if(id == 0) {
		// This is the master process
		// start processing
		BMP input_file;
		input_file.ReadFromFile(argv[1]);

		BMP output_file = input_file;

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		// actual processing	




	
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		
		// write file and display execution time
		output_file.WriteToFile("distributed.bmp");
		std::cout << "Time for distributed algorithm to complete: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "ms" << std::endl;
	}
	else {
		// Slave processes, wait to receive master process signals
	}


    ierr = MPI_Finalize();
}