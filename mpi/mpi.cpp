#include <iostream>
#include <chrono>
#include <mpi.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include "../kernel_process.hpp"
#include "../EasyBMP.h"

void mpi_master(BMP&, State&);
void mpi_slave(BMP&, State&);

int socket_send(int, long);

// used for transmitting a processed pixel back to the master process
#pragma pack(2)
struct mpi_pixel {
	int x {0};
	int y {0};
	int red;
	int green;
	int blue;
};
#pragma pack()


// Each process opens their own copy of the bitmap file.
// Slave processes send processed pixels back to the master
// process to be compiled into a complete image and saved.

int main(int argc, char* argv[]) {
	SetEasyBMPwarningsOff();
	int ierr = MPI_Init(&argc, &argv);

	int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
	long time {0};

	int port = std::stoi(argv[3]);

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

		// communicate execution time
		if(socket_send(port, time)) {
			return -1;
		}
	}
	else {
		// This is a slave process
		State s;
		if(!s.bmp.ReadFromFile(argv[1])) {
			return 1;
		}

		s.kern_process = static_cast<Kernel_type>(std::stoi(argv[2]));

		BMP output_file = s.bmp;
		mpi_slave(output_file, s);
	}
    ierr = MPI_Finalize();
	return static_cast<int>(time);
}

void mpi_master(BMP& output_file, State& s) {
	int process_count;
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	int interval = output_file.TellHeight()/(process_count-1);

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


	// recv mpi pixel structs from anywhere
	for(int y = 0; y < output_file.TellHeight(); y++) {
		for(int x = 0; x < output_file.TellWidth(); x++) {
			// recv
			mpi_pixel recv_pixel;
			MPI_Recv(&recv_pixel, 5, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			RGBApixel* p = output_file(recv_pixel.x, recv_pixel.y);
			p->Red = static_cast<u_int8_t>(recv_pixel.red);
			p->Green = static_cast<u_int8_t>(recv_pixel.green);
			p->Blue = static_cast<u_int8_t>(recv_pixel.blue);
		}
	}
}

void mpi_slave(BMP& output_file, State& s) {

	int start_row;
	MPI_Recv(&start_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	int end_row;
	MPI_Recv(&end_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


	// kernel processing
	kernel_process(output_file, s, start_row, end_row);

	// set the end row, works better for the send loop
	if(end_row == 0) {
		end_row = output_file.TellHeight();
	}

	// send pixel data to master process
	for(int y = start_row; y < end_row; y++) {
		for(int x = 0; x < output_file.TellWidth(); x++) {
			RGBApixel* p = output_file(x, y);
			mpi_pixel send_pixel;
			send_pixel.x = x;
			send_pixel.y = y;
			send_pixel.red = p->Red;
			send_pixel.green = p->Green;
			send_pixel.blue = p->Blue;

			// send the struct as if it were an array of ints
			MPI_Send(&send_pixel, 5, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}
}


int socket_send(int port, long time) {
	// adapted from: https://www.geeksforgeeks.org/socket-programming-cc/
	// send given long int on given port
	int sock = 0;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "Socket Error" << std::endl;
		return -1;
	}
   
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	   
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
		std::cerr << "Invalid Address" << std::endl;
		return -1;
	}
   
	if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
		std::cerr << "Connect failed" << std::endl;
		return -1;
	}
	
	send(sock , &time, sizeof(time), 0);
	return 0;
}