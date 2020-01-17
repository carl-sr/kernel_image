#pragma once
#include "./kernel.hpp"
#include "./EasyBMP.h"
#include <iostream>
#include <string>
#include <vector>



struct State {
	std::string input_file;
	int args {0};
	BMP bmp;
	Kernel_type kern_process;
};


long sequential(State&);

long distributed(State&);

long parallel(State&);




void kernel_process(State&, int, int);
void kernel_pixel_process(int, int, Kernel&, State&);
