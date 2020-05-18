Make:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf
	./main.elf ./images/MARBLES.BMP gaussian -p

compile:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf

compile_all:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o main.elf
	mpic++ ./mpi/mpi.cpp EasyBMP.cpp kernel_process.cpp -g -o ./mpi/mpi.elf

compile_mpi:
	mpic++ ./mpi/mpi.cpp EasyBMP.cpp kernel_process.cpp -g -o ./mpi/mpi.elf

run:
	./main.elf ./images/MARBLES.BMP box

run_s:
	./main.elf ./images/MARBLES.BMP box -s

run_p:
	./main.elf ./images/MARBLES.BMP box -p

run_d:
	./main.elf ./images/MARBLES.BMP box -d

run_mpi:
	mpirun -np 4 ./mpi/mpi.elf ./images/MARBLES.BMP 1