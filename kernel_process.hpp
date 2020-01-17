#pragma once
#include "./kernel.hpp"
#include <iostream>
#include <string>
#include <vector>

#pragma pack(2)
// struct PPM_HEADER {
// 	u_int16_t identifier;
// };
#pragma pack()

struct Pixel {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};


class Ppm {
	private:
		int w {0};
		int h {0};

		std::vector<Pixel> pixels;

	public:
		std::string filename;
		~Ppm();
		int open(std::string);
		int write(std::string);

		int height();
		int width();
		bool test(int, int);

		Pixel operator[](int);
		Pixel coordinate(int, int);
		void set_coordinate(int, int, Pixel);

};

struct State {
	std::string input_file;
	int args {0};
	Ppm ppm;
	Kernel_type kern_process;
};


long sequential(State&);

long distributed(State&);

long parallel(State&);




void kernel_process(State&, Ppm&, int, int);
Pixel kernel_pixel_process(int, int, Kernel&, State&);
