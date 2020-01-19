#pragma once
#include <cstring>
#include <vector>



// identity
// edge1
// edge2
// edge3
// sharpen
// box
// gaussian
// unsharp

enum Kernel_type {
	IDENTITY,
	EDGE_DETECTION_1,
	EDGE_DETECTION_2,
	EDGE_DETECTION_3,
	SHARPEN,
	BOX_BLUR,
	GAUSSIAN_BLUR,
	UNSHARP_MASK
};

class Kernel {
	public:
		int multiplier {1};
		int dimension;
		int** grid;
		
		

		Kernel(Kernel_type k) {
			std::vector<int> seq;
			switch(k) {
				case IDENTITY: {
					dimension = 3;
					seq = {
						0, 0, 0,
						0, 1, 0,
						0, 0, 0
					};
					break;
				}
				case EDGE_DETECTION_1: {
					dimension = 3;
					seq = {
						1, 0, -1,
						0, 0, 0,
						-1, 0, 1
					};
					break;
				}
				case EDGE_DETECTION_2: {
					dimension = 3;
					seq = {
						0, 1, 0,
						1, 4, 1,
						0, 1, 0
					};
					break;
				}
				case EDGE_DETECTION_3: {
					dimension = 3;
					seq = {
						-1, -1, -1,
						-1, 8, -1,
						-1, -1, -1
					};
					break;
				}
				case SHARPEN: {
					dimension = 3;
					seq = {
						0, -1, 0,
						-1, 5, -1,
						0, -1, 0
					};
					break;
				}
				case BOX_BLUR: {
					multiplier = 1.0/9.0;
					dimension = 3;
					seq = {
						1, 1, 1,
						1, 1, 1,
						1, 1, 1
					};
					break;
				}
				case GAUSSIAN_BLUR: {
					multiplier = 1.0/256.0;
					dimension = 5;
					seq = {
						1, 4, 6, 4, 1,
						4, 16, 24, 16, 4,
						6, 24, 36, 24, 6,
						4, 16, 24, 16, 4,
						1, 4, 6, 4, 1
					};
					break;
				}
				case UNSHARP_MASK: {
					multiplier = -1.0/256.0;
					dimension = 5;
					seq = {
						1, 4, 6, 4, 1,
						4, 16, 24, 16, 4,
						6, 24, -476, 24, 6,
						4, 16, 24, 16, 4,
						1, 4, 6, 4, 1
					};
					break;
				}
			}

			grid = new int*[dimension];
			for(int i = 0; i < dimension; i++) {
				grid[i] = new int[dimension];
			}

			auto seq_iter = seq.begin();
			for(int i = 0; i < dimension; i++) {
				for(int j = 0; j < dimension; j++) {
					grid[i][j] = *seq_iter;
					seq_iter++;
				}
			}
			

			
		}
		~Kernel() {
			for(int i = 0; i < dimension; i++) {
				delete[] grid[i];
			}
			delete[] grid;
		}
};

