# Kernel Image Processing
Final project for Programming III - Concurrent Programming. Demonstration of sequential, parallel, and distributed kernel image processing with reported execution times. Any combination (including none of) the algorithm types can be specified. The number of parallel or mpi processes can be specified. A testing flag allows for the collection of average and total times for a number of runs.

[easyBMP](http://easybmp.sourceforge.net/) is used for bmp file input/output.

------
## Usage
```bash
./main.elf [image file path] [kernel algorithm] [flags]
```
Available kernel algorithms are as follows:

* identity (produces an identical image)
* edge1 (edge detection 1)
* edge2 (edge detection 2)
* edge3 (edge detection 3)
* sharpen (sharp max)
* box (box blur)
* gaussian (gaussian blur)
* unsharp (unsharp mask)

The input image will be processed by the kernel algorithm and saved according to the indicated processing method: (sequential.bmp, parallel.bmp, distributed.bmp).

To specify that sequential processing should be used:
```bash
-s
```

Parallel processing can be specified with or without a set number of processes to use. The number of processes to be used is specified by appending an integer directly after the flag. In the case that no process count is specified the number of processing cores on the machine will determine how many processes will be used.
```bash
-p
-p8 # eight cores
```

Distributed processing is specified with the '-d' flag. By default 4 mpi processes (including the master process) are used. Any other number can be specified by appending an integer directly after the flag.
```bash
-d
-d5 # five mpi processes
```

Time tests can be run with the '-t' flag. This option runs the algorithms a specified (default 2) number of times and reports the average and total time in milliseconds. The number of times to run the algorithm is specified by appending an integer directly after the flag.
```bash
-t
-t4 # four rounds
```

A help dialogue explaining the usage of the program can be accessed using the '-h' flag. In the event that this flag is set, the help dialogue will be printed and the program will exit regardless of any additional processing flags that are set.
```bash
-h
```

------
## Under the Hood

### Program State:
```cpp
struct State {
	std::string input_file;
	int args {0};
	BMP bmp;
	Kernel_type kern_process;
	int threads {0};
	int mpi_procs {4};
	int test_execs {2};
};
```
The State struct holds various objects relevant to the execution of the program. ```args``` holds a value based on which command line flags are provided. During evaluation of ```argv[]```, ```args``` is bitwise OR'ed with the following defined constants:
```cpp
#define SEQUENTIAL 0b0001
#define DISTRIBUTED 0b0010
#define PARALLEL 0b0100
#define TEST 0b1000
```
```bmp``` is the input ```BMP``` structure. ```kern_process``` is an object that defines which kernel processing algorithm will be used. ```threads``` indicates the number of processing threads to be used in parallel execution. If no other value is specified in the command line arguments, the number of hardware threads is used. ```mpi_procs``` defines the number of processes to be used in distributed execution. The default value is 4. If defined in command line arguments to be less than 1, the value of 2 is used instead. MPI processing in this program uses a slave/master pattern that requires at least 2 processes. ```test_execs``` indicates the number of times the processing time should be collected when finding the average execution times.

### Kernel Class:
```cpp
class Kernel {
	public:
		float multiplier {0};
		int dimension;
		int** grid;
		Kernel(Kernel_type);
		~Kernel()
}
```
The Kernel class represents the convolution grid to be used in processing the input image. ```multiplier```, ```dimension```, and ```grid``` are set by the constructor dependant on the type of convolution specified in command line arguments.

### Kernel Processing:
```cpp
void kernel_process(BMP& output, State& state, int start_row, int end_row)
```
The kernel_process function reads the BMP structure ```state.bmp``` from ```start_row``` to ```end_row``` writing the resulting pixels to the ```output``` BMP structure. During processing, the ```kernel_pixel_process()``` function is called on each pixel:

```cpp
void kernel_pixel_process(BMP& output_file, BMP& input_file, int pixel_x, int pixel_y, Kernel& k)
```
This function loops through each value in the kernel grid ```k.grid``` and multiplies it by the corresponding pixel at (```pixel_x```, ```pixel_y```) from ```input_file```. These values are totaled for each pixel and either multiplied by ```k.multiplier``` or averaged. The pixel at (```pixel_x```, ```pixel_y```) in ```output_file``` is then set.

### Parallel Processing:
Unless otherwise specified in command line arguments, the number of threads to be used in processing is set by:
```cpp
std::thread::hardware_concurrency()
```
Each thread is spawned and its id stored in a ```std::thread``` vector. The parent process uses this vector to retreive all of the child threads once processing is complete. Each thread begins execution at the ```kernel_process()``` function with appropriate start and end processing rows based on the number of threads to be used and the height of the input image.

### Distributed Processing:
The mpi program is executed separately from the main progam:
```cpp
std::system("mpirun -np [process count] ./mpi/mpi.elf [input image] [kernel algorithm] [communication port] &");
```
The communication port is set randomly to a number between 9000 and 9500. If time testing is being run the communication port will be incremented by one at every round to avoid port conflicts. The port is used to create a communication socket for transmitting the processing time between the mpi program and the main executable.

The lower and upper processing bounds for each slave process is sent by the master process based on the number of slave processes and the height of the input image. As each slave finishing processing its portion of the image it sends pixel data back to the master process in the form of dedicated pixel structs:
```cpp
struct mpi_pixel {
	int x {0};
	int y {0};
	int red;
	int green;
	int blue;
};
```
These pixels are received by the master process:
```cpp
MPI_Recv(&recv_pixel, 5, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)
```
As the pixels are received they are written into the output image at the specified (x, y) coordinates. Once all pixels have been received, the total execution time is communicated to the main executable and the output image is written to disk.