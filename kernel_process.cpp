#include "./kernel_process.hpp"
#include "./EasyBMP.h"

#include <fstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <functional>

bool test_limit(BMP&, int, int);

// return the timing for each function

long sequential(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	BMP output_file = state.bmp;

	kernel_process(output_file, state, 0, 0);

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	output_file.WriteToFile("sequential.bmp");
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


long parallel(State& state) {

	BMP output_file = state.bmp;
	std::vector<std::thread> worker_threads;

	// find the number of supported concurrent threads if a number of threads has not been specified
	int thread_count;
	if(!state.threads) {
		thread_count = std::thread::hardware_concurrency();
	}
	else {
		thread_count = state.threads;
	}

	std::cout << "with " << thread_count << " threads: ";


	// the interval with which to divide image processing
	int interval = output_file.TellHeight()/thread_count;


	// start actual processing
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	// start the first n-1 threads in a loop
	int i = 0;
	for(; i < thread_count-1; i++) {
		std::thread th(kernel_process, std::ref<BMP>(output_file), std::ref<State>(state), interval*i, interval*(i+1));
		worker_threads.push_back(move(th));
	}

	// start the last thread, must process to the end of the bmp so end_row = 0
	std::thread th(kernel_process, std::ref<BMP>(output_file), std::ref<State>(state), interval*i, 0);

	worker_threads.push_back(move(th));


	// join all threads
	for(auto i = worker_threads.begin(); i != worker_threads.end(); i++) {
		i->join();
	}

	// get time once all actual processing is completed
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	output_file.WriteToFile("parallel.bmp");

	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

long distributed(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();



	for(int i = 0; i < 10000000; i++);



	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void kernel_process(BMP& output, State& state, int start_row, int end_row) {
	BMP& input = state.bmp;
	Kernel k = Kernel(state.kern_process);

	if(end_row == 0) {
		end_row = input.TellHeight();
	}

	for(int y = start_row; y < end_row; y++) {
		for(int x = 0; x < input.TellWidth(); x++) {
			kernel_pixel_process(output, input, x, y, k);
		}
	}
}

void kernel_pixel_process(BMP& output_file, BMP& input_file, int pixel_x, int pixel_y, Kernel& k) {
	
	int pixel_count = 0;

	int red = 0;
	int green = 0;
	int blue = 0;

	for(int kernel_y = 0; kernel_y < k.dimension; kernel_y++) {
		for(int kernel_x = 0; kernel_x < k.dimension; kernel_x++) {
			if(test_limit(input_file, pixel_x + (kernel_x-(k.dimension/2)), pixel_y + (kernel_y-(k.dimension/2))) && k.grid[kernel_x][kernel_y]) {
				pixel_count++;
				RGBApixel* pixel = input_file(pixel_x, pixel_y);

				// add with weight
				red += pixel->Red * k.grid[kernel_x][kernel_y];
				green += pixel->Green * k.grid[kernel_x][kernel_y];
				blue += pixel->Blue * k.grid[kernel_x][kernel_y];
			}
		}
	}
	RGBApixel* set = output_file(pixel_x, pixel_y);

	if(k.multiplier == 1) {
		pixel_count = 1;
	}

	set->Red = red/pixel_count;
	set->Green = green/pixel_count;
	set->Blue = blue/pixel_count;

	// set->Red = 0xff;
	// set->Green = 0;
	// set->Blue = 0;
}



bool test_limit(BMP& bmp, int x, int y) {

	// if the pixel at (x, y) is inside the bmp
	if(x < 0 || y < 0) {
		return false;
	}
	if(x > bmp.TellWidth() || y > bmp.TellHeight()) {
		return false;
	}
	return true;
}
