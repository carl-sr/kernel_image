#include <iostream>
#include <chrono>
#include <mpi.h>

#include "../kernel_process.hpp"
#include "../EasyBMP.h"

void mpi_master(BMP&, State&);
void mpi_slave();


int main(int argc, char* argv[]) {
	int ierr = MPI_Init(&argc, &argv);

	int id;
    
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	


	long time {0};

	if(id == 0) {
		// This is the master process
		State s;
		if(!s.bmp.ReadFromFile(argv[1])) {
			return 1;
		}

		s.kern_process = static_cast<Kernel_type>(std::stoi(argv[2]));

		BMP output_file = s.bmp;

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		mpi_master(output_file, s);
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		
		// write file and display execution time
		output_file.WriteToFile("distributed.bmp");
		long time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Time for distributed algorithm to complete: " << time << "ms" << std::endl;
	}
	else {
		mpi_slave();
	}
    ierr = MPI_Finalize();
	return static_cast<int>(time);
}

void mpi_master(BMP& output_file, State& s) {
	int process_count;
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	int interval = output_file.TellHeight()/(process_count-1);

	// send each slave the state and input bmp
	// for(int i = 1; i < process_count; i++) {
	// 	MPI_Send(&output_file, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	// 	MPI_Send(&s, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	// }

	// send each slave its processing segment
	int start_row;
	int end_row;
	for(int i = 1; i < process_count-1; i++) {
		start_row = interval*(i-1);
		end_row = interval*i;
		MPI_Send(&start_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(&end_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	}

	// last section needs end_row of 0
	start_row = interval*(process_count-2);
	end_row = 0;
	MPI_Send(&start_row, 1, MPI_INT, process_count-1, 0, MPI_COMM_WORLD);
	MPI_Send(&end_row, 1, MPI_INT, process_count-1, 0, MPI_COMM_WORLD);
}

void mpi_slave() {
	BMP* output_file_point;
	// receive output_file address
	// MPI_Recv(&output_file_point, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	BMP& output_file = *output_file_point;

	State* state_point;
	// receive state address - maybe figure out how to send/receive entire data
	// MPI_Recv(&state_point, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	State& state = *state_point;

	int start_row;
	MPI_Recv(&start_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	int end_row;
	MPI_Recv(&end_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	std::cout << "Slave received start_row: " << start_row << " end_row: " << end_row << std::endl;
	
	// kernel_process(output_file, state, start_row, end_row);
}