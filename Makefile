Make:
	g++ -pthread kernel_process.cpp EasyBMP.cpp main.cpp -g -o imgkrn.elf
	mpic++ ./mpi/mpi.cpp EasyBMP.cpp kernel_process.cpp -g -o ./mpi/mpi.elf