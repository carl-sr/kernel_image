#include <iostream>
#include <string>
#include <cstring>

#include <thread>

#include "./kernel_process.hpp"

#define SEQUENTIAL 0b001
#define DISTRIBUTED 0b010
#define PARALLEL 0b100

int parse_flags(int argc, char* argv[], State&);
void help();

int main(int argc, char* argv[]) {


	// ./main.elf filename kernel_process -s sequential  -d distributed  -p(x) parallel
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

	if(state.args == 0) {
		std::cerr << "No processing flags specified (-h for help)" << std::endl;
	}

	if(!state.bmp.ReadFromFile(argv[1])) {
		return 1;
	}

	if(state.args & SEQUENTIAL) {
		std::cout << "Time for sequential algorithm to complete: " << sequential(state) << "ms" << std::endl;
	}

	if(state.args & PARALLEL) {
		std::cout << "Time for parallel algorithm to complete " << parallel(state) << "ms" << std::endl;
	}

	if(state.args & DISTRIBUTED) {
		std::cout << "Time for distributed algorithm to complete: " << distributed(state) << "ms" << std::endl;
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
					break;
				case ('p'):
					args = args | PARALLEL;
					if(strlen(argv[i]) > 2) {
						std::string str = std::string(argv[i]);
						s.threads = std::stoi(str.substr(2, str.length()));
					}
					break;
				default:
					std::cerr << "Unrecognized flag: " << argv[i] << " (-h for help)" << std::endl;
					return -1;
			}
		}
	}
}

void help() {
	std::cout << "********************************************************" << std::endl;
	std::cout << "* imgkrn: Kernel Convolution Image Processing          *" << std::endl;
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
	std::cout << "*    - '-h' to print this help dialogue and exit.      *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "* Files are written as:                                *" << std::endl;
	std::cout << "*    - 'sequential.bmp'                                *" << std::endl;
	std::cout << "*    - 'parallel.bmp'                                  *" << std::endl;
	std::cout << "*    - 'distributed.bmp'                               *" << std::endl;
	std::cout << "*                                                      *" << std::endl;
	std::cout << "********************************************************" << std::endl;
}