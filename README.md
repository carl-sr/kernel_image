# Kernel Image Processing
Final project for Programming III - Concurrent Programming. Domonstration of sequential, parallel, and distributed kernel image processing with reported execution times. [easyBMP](http://easybmp.sourceforge.net/) is used for bmp file input/output.

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
