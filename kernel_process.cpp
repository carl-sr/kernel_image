#include "./kernel_process.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

int Ppm::open(std::string in) {
	filename = in;
	std::ifstream f;
	f.open(in, std::fstream::binary | std::fstream::ate);
	if(!f.is_open()) {
		std::cerr << "Error opening file " << in << std::endl;
		return 1;
	}


	int data_length = f.tellg();

	f.seekg(std::fstream::beg);

	uint8_t* data = new uint8_t[data_length];

	for(int i = 0; i < data_length; i++) {
		data[i] = f.get();
	}
	f.close();

	// check the magic number
	if(data[0] != 'P' | data[1] != '6') {
		std::cerr << "Unsupported file. File must be in raw PPM format." << std::endl;
		delete[] data;
		exit(1);
	}

	u_int8_t bit_offset = data[3];
	bit_offset += 3;

	for(; bit_offset < data_length; bit_offset += 3) {
		
	}


	delete[] data;

	return 0;
}

Ppm::~Ppm() {

}




int Ppm::write(std::string out) {
	std::ofstream f;
	f.open(out, std::fstream::binary | std::fstream::trunc);

	f.close();
	return 0;
}

int Ppm::height() {
	return h;
}

int Ppm::width() {
	return w;
}

bool Ppm::test(int x, int y) {
	if(x < 0 || y < 0) {
		return false;
	}
	if(x > w || y > h) {
		return false;
	}
	return true;
}



Pixel Ppm::coordinate(int x, int y) {
	return pixels[y*w + x];
}

void Ppm::set_coordinate(int x, int y, Pixel p) {
	pixels[y*w + x] = p;
}


// return the timing for each function

long sequential(State& state) {
	std::cout << std::endl;
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	for(int y = 0; y < state.ppm.height(); y++) {
		for(int x = 0; x < state.ppm.width(); x++) {
			Pixel p = state.ppm.coordinate(x, y);
			printf("%3x%3x%3x  ", p.red, p.green, p.blue);
		}
		printf("\n");
	}

	Ppm sequential_ppm = state.ppm;
	kernel_process(state, sequential_ppm, 0, 0);
	sequential_ppm.write("sequential.ppm");

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}



long parallel(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	Ppm parallel_ppm = state.ppm;
	// kernel_process(state.ppm, parallel_ppm, 0, state.ppm.height() - 1);
	parallel_ppm.write("parallel.ppm");

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}



long distributed(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();



	for(int i = 0; i < 10000000; i++);



	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void kernel_process(State& state, Ppm& destination, int start_row, int end_row) {
	Ppm& source = state.ppm;
	Kernel k = Kernel(state.kern_process);

	if(end_row == 0) {
		end_row = source.height();
	}


	for(int y = start_row; y < end_row; y++) {
		for(int x = 0; x < source.width(); x++) {
			// destination.set_coordinate(x, y, kernel_pixel_process(x, y, k, state));
		}
		
	}
}

Pixel kernel_pixel_process(int pixel_x, int pixel_y, Kernel& k, State& state) {
	
	int pixel_count = 0;
	int red = 0;
	int green = 0;
	int blue = 0;


	for(int kernel_y = 0; kernel_y < k.dimension; kernel_y++) {
		for(int kernel_x = 0; kernel_x < k.dimension; kernel_x++) {
			if(state.ppm.test(pixel_x + (kernel_x-(k.dimension/2)), pixel_y + (kernel_y-(k.dimension/2)))) {
				pixel_count++;
				Pixel tmp = state.ppm.coordinate(pixel_x + (kernel_x-(k.dimension/2)), pixel_y + (kernel_y-(k.dimension/2)));
				red += tmp.red * k.grid[kernel_x][kernel_y];
				green += tmp.red * k.grid[kernel_x][kernel_y];
				blue += tmp.red * k.grid[kernel_x][kernel_y];
			}
		}
	}
	Pixel p;

	p.red = red/pixel_count;
	p.green = green/pixel_count;
	p.blue = blue/pixel_count;

	return p;
}