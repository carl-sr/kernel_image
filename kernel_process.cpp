#include "./kernel_process.hpp"
#include "./EasyBMP.h"

#include <fstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <functional>


// return the timing for each function

long sequential(State& state) {
	BMP output_file = state.bmp;

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

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

void kernel_process(BMP& output, State& state, int start_row, int end_row) {
	BMP& input = state.bmp;
	Kernel k = Kernel(state.kern_process);

	if(end_row == 0) {
		end_row = input.TellHeight();
	}

	// for each pixel in the image, process
	for(int y = start_row; y < end_row; y++) {
		for(int x = 0; x < input.TellWidth(); x++) {
			kernel_pixel_process(output, input, x, y, k);
		}
	}
}

void kernel_pixel_process(BMP& output_file, BMP& input_file, int pixel_x, int pixel_y, Kernel& k) {
	
	// how many pixels were processed
	int pixel_count = 0;

	// keep running totals for all channels
	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;

	// the amount to offset the input image by in applying the convolution matrix
	int kern_offset = 0-(k.dimension/2);

	for(int kernel_y = 0; kernel_y < k.dimension; kernel_y++) {
		for(int kernel_x = 0; kernel_x < k.dimension; kernel_x++) {
			// calculate which pixels should be fetched from the input image based on which index of the convolution matrix is being applied
			// kern_offset + kernel_x gets how far away from the center of the matrix is being processed
			
			int adj_pix_x = pixel_x + kern_offset + kernel_x;
			int adj_pix_y = pixel_y + kern_offset + kernel_y;

			if(test_limit(input_file, adj_pix_x, adj_pix_y) && k.grid[kernel_x][kernel_y] != 0) {
				++pixel_count;

				RGBApixel* p = input_file(adj_pix_x, adj_pix_y);

				// apply weights from convolution matrix
				red += p->Red * k.grid[kernel_x][kernel_y];
				green += p->Green * k.grid[kernel_x][kernel_y];
				blue += p->Blue * k.grid[kernel_x][kernel_y];
				alpha += p->Alpha * k.grid[kernel_x][kernel_y];
			}
		}
	}

	// just in case to avoid floating point exception
	if(!pixel_count) {
		pixel_count = 1;
	}

	// if a multiplier is set
	if(k.multiplier) {
		red *= k.multiplier;
		blue *= k.multiplier;
		green *= k.multiplier;
		alpha *= k.multiplier;
	}
	else {
		// get the average
		red /= pixel_count;
		blue /= pixel_count;
		green /= pixel_count;
		alpha /= pixel_count;
	}
	RGBApixel* set = output_file(pixel_x, pixel_y);

	if(red < 0) {
		red = 0;
	}
	if(green < 0) {
		green = 0;
	}
	if(blue < 0) {
		blue = 0;
	}

	set->Red = red;
	set->Green = green;
	set->Blue = blue;
	set->Alpha = alpha;
}


bool test_limit(BMP& bmp, int x, int y) {
	// if the pixel at (x, y) is inside the bmp return true
	if(x < 0 || y < 0) {
		return false;
	}
	if(x > bmp.TellWidth()-1 || y > bmp.TellHeight()-1) {
		return false;
	}
	return true;
}