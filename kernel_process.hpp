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
	int threads {0};
};


long sequential(State&);

long distributed(State&);

long parallel(State&);


void kernel_process(BMP&, State&, int, int);
void kernel_pixel_process(BMP&, BMP&, int, int, Kernel&);
bool test_limit(BMP&, int, int);
