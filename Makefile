Make:
	g++ kernel_process.cpp main.cpp -g -o main.elf
	./main.elf ./images/blue.bmp identity -s
