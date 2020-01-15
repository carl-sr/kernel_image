#pragma once
#include "./kernel.hpp"
#include <iostream>
#include <string>
#include <vector>

// bitmap parts 2 byte aligned
#pragma pack(2)
struct BITMAPFILEHEADER {
	u_int16_t bfType;
	u_int32_t bfSize;
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;
};
struct BITMAPINFOHEADER {
	u_int32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	u_int16_t biPlanes;
	u_int16_t biBitCount;
	u_int32_t biCompression;
	u_int32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	u_int32_t biClrUsed;
	u_int32_t biClrImportant;
};
#pragma pack()

struct Pixel {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};


class Bmp {
	private:
		//Bmp data
		BITMAPFILEHEADER bitmap_file_header;
		BITMAPINFOHEADER bitmap_info_header;

		std::vector<u_int8_t> color_pallet;

		std::vector<Pixel> pixels;

	public:
		std::string filename;
		~Bmp();
		int open(std::string);
		int write(std::string);

		int length();
		int height();
		int width();

		Pixel operator[](int);
		Pixel coordinate(int, int);

};

struct State {
	std::string input_file;
	int args {0};
	Bmp bmp;
	Kernel_type kern_process;
};


long sequential(State&);

long distributed(State&);

long parallel(State&);




void kernel_process(State&, Bmp&, int, int);