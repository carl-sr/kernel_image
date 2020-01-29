# Kernel Image Processing
Final project for Programming III - Concurrent Programming. Sequential, parallel, and distributed kernel image processing. [easyBMP](http://easybmp.sourceforge.net/) is used for bmp file input/output.

## Usage
```bash
./main.elf [image file path] [kernel algorithm] [flags]
```
The input image will be run through the kernel algorithm and saved according to the processing method (sequential.bmp, parallel.bmp, distributed.bmp).

To specify that sequential processing should be used:
```bash
-s
```

To specify that parallel processing should be used:
```bash
-p
```
The number of threads to be used for processing (8 in this example) can also be specified:
```bash
-p8
```
To specify that distributed processing should be used:
```bash
-d
```
