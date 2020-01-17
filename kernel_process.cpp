#include "./kernel_process.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

// return the timing for each function

long sequential(State& state) {
	std::cout << std::endl;
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();


	kernel_process(state, 0, 0);

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}



long parallel(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	BMP output_file = state.bmp;
	kernel_process(output_file, state, 0, 0);

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
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

	if(end_row == 0) {
		end_row = input.TellHeight();
	}
}

void kernel_pixel_process(int pixel_x, int pixel_y, Kernel& k, State& state) {

}