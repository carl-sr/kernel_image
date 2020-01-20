Make:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf
	./main.elf ./images/rachel.bmp gaussian -p
	feh ./parallel.bmp

compile:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf

run:
	./main.elf ./images/dot.bmp box -s
	# feh ./parallel.bmp
	feh ./sequential.bmp