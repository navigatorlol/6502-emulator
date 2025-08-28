#include "6502.cpp"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
	bool wait = false;

	// Loop through all arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if (arg == "-wait") {
			wait = true;
		}
	}


	MEM mem;
	CPU cpu;
	cpu.Reset( mem );	
	//import bin file:
	std::ifstream rf("binary/rom.bin");
	for (unsigned int i=0; i<1024*64; i++){
		rf.read((char *) &mem[i], sizeof(unsigned char));
	}
	rf.close();
	if(!rf.good()) {
		printf("Fehler");
		return 1;
	}
	// CPU execute n cycles
	cpu.Execute(100, mem, wait);
	mem.printMem();	
	cpu.printRegister();
	return 0;
}
