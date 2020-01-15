#include "./kernel_process.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

Bmp::~Bmp() {
	if(pixels.size()) {
		pixels.clear();
	}
}
int Bmp::open(std::string in) {
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

	//Data has been read

	std::memcpy(&bitmap_file_header, data, 14);
	std::memcpy(&bitmap_info_header, data + 14, 40);

	for(int i = 54; i < bitmap_file_header.bfOffBits; i++) {
		color_pallet.push_back(data[i]);
	}

	for(int i = bitmap_file_header.bfOffBits; i < bitmap_file_header.bfSize; i+=3) {
		Pixel p;
		p.blue = data[i];
		p.green = data[i+1];
		p.red = data[i+2];
		pixels.push_back(p);
	}

	delete[] data;

	return 0;
}




int Bmp::write(std::string out) {
	std::ofstream f;
	f.open(out, std::fstream::binary | std::fstream::trunc);

	// write bitmap file header
	for(int i = 0; i < sizeof(bitmap_file_header); i++) {
		f << reinterpret_cast<u_int8_t*>(&bitmap_file_header)[i];
	}

	// write bitmap info header
	for(int i = 0; i < sizeof(bitmap_info_header); i++) {
		f << reinterpret_cast<u_int8_t*>(&bitmap_info_header)[i];
	}

	for(auto i = color_pallet.begin(); i != color_pallet.end(); i++) {
		f << *i;
	}

	// write pixel data

	for(auto i = pixels.begin(); i != pixels.end(); i++) {
		f << i->blue;
		f << i->green;
		f << i->red;
	}

	f.close();
	return 0;
}

Pixel Bmp::operator[](int i) {
	return pixels[i];
}
int Bmp::length() {
	return pixels.size();
}

int Bmp::height() {
	return bitmap_info_header.biHeight;
}

int Bmp::width() {
	return bitmap_info_header.biWidth;
}


Pixel Bmp::coordinate(int x, int y) {
	return pixels[x*bitmap_info_header.biWidth + y];
}



// return the timing for each function

long sequential(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	Bmp sequential_bmp = state.bmp;
	kernel_process(state, sequential_bmp, 0, 0);
	sequential_bmp.write("sequential.bmp");

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}



long parallel(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	Bmp parallel_bmp = state.bmp;
	// kernel_process(state.bmp, parallel_bmp, 0, state.bmp.height() - 1);
	parallel_bmp.write("parallel.bmp");

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}



long distributed(State& state) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();



	for(int i = 0; i < 10000000; i++);



	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void kernel_process(State& state, Bmp& destination, int start_row, int end_row) {
	Bmp& source = state.bmp;
	Kernel k = Kernel(state.kern_process);

	if(end_row == 0) {
		end_row = source.height();
	}

	for(int row = start_row; row < end_row; row++) {
		for(int col = 0; col < source.width(); col++) {
			// process per pixel
			Pixel p = source.coordinate(col, row);
			// printf("R: %x, G: %x, B: %x\n", p.red, p.green, p.blue);
		}
		
	}
}