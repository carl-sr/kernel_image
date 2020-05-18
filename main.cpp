#include <iostream>
#include <string>
#include <cstring>

#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <unistd.h>
#include <ctime>



#include <thread>

#include "./kernel_process.hpp"

#define SEQUENTIAL 0b0001
#define DISTRIBUTED 0b0010
#define PARALLEL 0b0100
#define TEST 0b1000

#define STARTING_PORT 9000

int parse_flags(int argc, char* argv[], State&);
void help();
long socket_recv(int);


int main(int argc, char* argv[]) {
	// SetEasyBMPwarningsOff();
	srand(time(NULL));

	// if the help flag is present, call help and return
	for(int i = 0; i < argc; i++) {
		if(std::string(argv[i]) == "-h") {
			help();
			return 0;
		}
	}

	if(argc < 4) {
		std::cerr << "Too few arguments (-h for help)" << std::endl;
		return 1;
	}

	State state;
	
	if(parse_flags(argc, argv, state) == -1) {
		return 1;
	}

	if(state.args == 0 || state.args == 0b1000) {
		std::cerr << "No processing flags specified (-h for help)" << std::endl;
	}

	// find the number of processing cores user has not specified number of processes to use
	if(!state.threads) {
		state.threads = std::thread::hardware_concurrency();
	}

	if((state.args & TEST) == 0) {
		// run each process only once
		if(state.args & (SEQUENTIAL | PARALLEL)) {
			if(!state.bmp.ReadFromFile(argv[1])) {
				return 1;
			}

			if(state.args & SEQUENTIAL) {
				std::cout << "Time for sequential algorithm to complete: " << sequential(state) << "ms" << std::endl;
			}

			if(state.args & PARALLEL) {
				std::cout << "Time for parallel algorithm to complete with " << state.threads << " threads: " << parallel(state) << "ms" << std::endl;
			}
		}

		if(state.args & DISTRIBUTED) {
			// open socket for transmitting time values
			// random port STARTING_PORT + [0, 500) to avoid port conflicts
			int port = STARTING_PORT + (rand() % 500);
			// start the mpi process in the background
			std::string exec = "mpirun -np " + std::to_string(state.mpi_procs) + " ./mpi/mpi.elf " + argv[1] + " " + std::to_string(state.kern_process) + " " + std::to_string(port) + " &";
			std::system(exec.c_str());
			std::cout << "Time for distributed algorithm to complete with " << state.mpi_procs << " mpi processes: " << socket_recv(port) << "ms" << std::endl;
		}
	}
	else {
		// run each process n times, report the average
		if(state.args & (SEQUENTIAL | PARALLEL)) {
			if(!state.bmp.ReadFromFile(argv[1])) {
				return 1;
			}

			if(state.args & SEQUENTIAL) {
				long total {0};
				for(int i = 0; i < state.test_execs; i++) {
					total += sequential(state);
				}
				std::cout << "Average time for sequential algorithm to complete, (" << state.test_execs << " runs): " << total/state.test_execs << "ms, total time: " << total << "ms" << std::endl;
			}

			if(state.args & PARALLEL) {
				long total {0};
				for(int i = 0; i < state.test_execs; i++) {
					total += parallel(state);
				}
				std::cout << "Average time for parallel algorithm to complete with " << state.threads << " threads, (" << state.test_execs << " runs): " << total/state.test_execs << "ms, total time: " << total << "ms" << std::endl;
			}
		}

		if(state.args & DISTRIBUTED) {
			long total {0};
			// random port STARTING_PORT + [0, 500) to avoid port conflicts
			int port = STARTING_PORT + (rand() % 500);
			for(int i = 0; i < state.test_execs; i++) {
				// port becomes port + i to avoid attempting to use a port that has not yet been freed for use
				// start the mpi process in the background
				std::string exec = "mpirun -np " + std::to_string(state.mpi_procs) + " ./mpi/mpi.elf " + argv[1] + " " + std::to_string(state.kern_process) + " " + std::to_string(port + i) + " &";
				std::system(exec.c_str());
				total += socket_recv(port + i);
			}
			std::cout << "Average time for distributed algorithm to complete with " << state.mpi_procs << " mpi processes (" << state.test_execs << " runs): " << total/state.test_execs << "ms, total time: " << total << "ms" << std::endl;

		}
	}

	return 0;
}




int parse_flags(int argc, char* argv[], State& s) {
	s.input_file = argv[1];
	std::string k_type = argv[2];

	if(k_type == "identity") {
		s.kern_process = IDENTITY;
	}
	else if(k_type == "edge1") {
		s.kern_process = EDGE_DETECTION_1;
	}
	else if(k_type == "edge2") {
		s.kern_process = EDGE_DETECTION_2;
	}
	else if(k_type == "edge3") {
		s.kern_process = EDGE_DETECTION_3;
	}
	else if(k_type == "sharpen") {
		s.kern_process = SHARPEN;
	}
	else if(k_type == "box") {
		s.kern_process = BOX_BLUR;
	}
	else if(k_type == "gaussian") {
		s.kern_process = GAUSSIAN_BLUR;
	}
	else if(k_type == "unsharp") {
		s.kern_process = UNSHARP_MASK;
	}
	else {
		std::cerr << "unrecognized kernel process: " << k_type << " (-h for help)" <<std::endl;
		return -1;
	}



	int& args = s.args;

	for(int i = 2; i < argc; i++) {
		if(argv[i][0] == '-' && strlen(argv[i]) > 1) {
			switch (argv[i][1]) {
				case ('s'):
					args = args | SEQUENTIAL;
					break;
				case ('d'):
					args = args | DISTRIBUTED;
					if(strlen(argv[i]) > 2) {
						std::string str = std::string(argv[i]);
						s.mpi_procs = std::stoi(str.substr(2, str.length()));
						if(s.mpi_procs == 1) {
							s.mpi_procs = 2;
						}
					}
					break;
				case ('p'):
					args = args | PARALLEL;
					if(strlen(argv[i]) > 2) {
						std::string str = std::string(argv[i]);
						s.threads = std::stoi(str.substr(2, str.length()));
					}
					break;
				case ('t'):
					args = args | TEST;
					if(strlen(argv[i]) > 2) {
						std::string str = std::string(argv[i]);
						s.test_execs = std::stoi(str.substr(2, str.length()));
					}
					break;
				default:
					std::cerr << "Unrecognized flag: " << argv[i] << " (-h for help)" << std::endl;
					return -1;
			}
		}
	}
	return 0;
}

void help() {
	std::cout << "********************************************************" << std::endl;
	std::cout << "* imgkrn: Kernel Convolution Image Processing          *" << std::endl;
	std::cout << "* Made for Programming III                             *" << std::endl;
	std::cout << "* Rami Hansen                                          *" << std::endl;
	std::cout << "* ==================================================== *" << std::endl;
	std::cout << "* Launch the program from command line with            *" << std::endl;
	std::cout << "* the following arguments:                             *" << std::endl;
	std::cout << "* ./imgkrn.elf [input image] [kernel process] [flags]  *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "* Kernel processes are as follows:                     *" << std::endl;
	std::cout << "*    - identity                                        *" << std::endl;
	std::cout << "*    - edge1                                           *" << std::endl;
	std::cout << "*    - edge2                                           *" << std::endl;
	std::cout << "*    - edge3                                           *" << std::endl;
	std::cout << "*    - sharpen                                         *" << std::endl;
	std::cout << "*    - box                                             *" << std::endl;
	std::cout << "*    - gaussian                                        *" << std::endl;
	std::cout << "*    - unsharp                                         *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "* Flags are as follows:                                *" << std::endl;
	std::cout << "*    - '-s' to specify sequential processing.          *" << std::endl;
	std::cout << "*    - '-p' to specify parallel processing.            *" << std::endl;
	std::cout << "*      The number of threads to be used is specified   *" << std::endl;
	std::cout << "*      by an additional integer appended to the end of *" << std::endl;
	std::cout << "*      the '-p' flag: '-p8' for 8 thread processing    *" << std::endl;
	std::cout << "*    - '-d' to specify distributed processing.         *" << std::endl;
	std::cout << "*      The number of processes to be used is specified *" << std::endl;
	std::cout << "*      by an additional integer appended to the end of *" << std::endl;
	std::cout << "*      the '-d' flag: '-d8' for 8 processes.           *" << std::endl;
	std::cout << "*    - '-t' enable the testing mode. The average for   *" << std::endl;
	std::cout << "*      n executions (specified '-tn', default 2) will  *" << std::endl;
	std::cout << "*      be returned.                                    *" << std::endl;
	std::cout << "*    - '-h' to print this help dialogue and exit.      *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "* Files are written as:                                *" << std::endl;
	std::cout << "*    - 'sequential.bmp'                                *" << std::endl;
	std::cout << "*    - 'parallel.bmp'                                  *" << std::endl;
	std::cout << "*    - 'distributed.bmp'                               *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "********************************************************" << std::endl;
}

long socket_recv(int port) {
	// adapted from: https://www.geeksforgeeks.org/socket-programming-cc/
	// wait to receive a long int from given port
	long time {0};

    // Creating socket file descriptor 
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket Failed" << std::endl;
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt Error" << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int addrlen = sizeof(address);
       
    // Forcefully attaching socket to the port
    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    int new_socket;
    if ((new_socket = accept(server_fd, reinterpret_cast<sockaddr*>(&address), reinterpret_cast<socklen_t*>(&addrlen))) < 0) {
        std::cerr << "Accept Failed" << std::endl;
        exit(EXIT_FAILURE);
    }
	int valread = read(new_socket, &time, sizeof(time));

    return time;
}