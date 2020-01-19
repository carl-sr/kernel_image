Make:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf
	./main.elf ./images/rachel.bmp gaussian -p
	feh ./parallel.bmp
