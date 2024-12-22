#include "emulator.h"
#include <iostream>
#include <fstream>
#include <string>

//#define PRINT_PROGRAM

int loadIntoMemory(std::string filename, uint8_t*& memory) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return -1;
    }

    // Get the file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Ensure the program fits into CHIP-8 memory
    if (fileSize > (3585)) {
        std::cerr << "Error: File is too large for CHIP-8 memory." << std::endl;
        return -1;
    }

    memory = new uint8_t[fileSize];

    // Read the file into the memory buffer starting at address 0x200
    if (!file.read(reinterpret_cast<char*>(memory), fileSize)) {
        std::cerr << "Error: Failed to read the file into memory." << std::endl;
        return -1;
    }

    std::cout << "File successfully loaded into memory!" << std::endl;
    return fileSize;
}

inline char toHex(uint8_t nibble) {
    if (nibble < 0x0A) {
        return static_cast<char>(nibble) + '0';
    }
    else {
        return static_cast<char>(nibble) + 'A' - 10;
    }
}

int main(int argc, char* argv[]) {
    /*
	if (argc < 2) {
		std::cerr << "file path not provided. Please drag a CHIP-8 " <<
			"program onto the.exe file of the emulator" << std::endl;
		return 1;
	}*/

    std::string path = "C:\\Users\\aryan\\Downloads\\pumpkindressup.ch8";//argv[1];
	uint8_t* program = nullptr;

    int programSize = loadIntoMemory(path, program);

	if (programSize == -1) {
        return 1;
	}

    std::cout << "Program has a size of " << programSize << " bytes" << std::endl;
#ifdef PRINT_PROGRAM
    std::cout << "Program -\nS.no | Location | Instruction" << std::endl;

    for (int i = 0; i < programSize; i += 2) {
        std::cout << (i / 2) + 1 << "|" << i + 512 << "|" << toHex((program[i] & 0xF0) >> 4) << toHex(program[i] & 0x0F) <<
            toHex((program[i + 1] & 0xF0) >> 4) << toHex(program[i + 1] & 0x0F) << std::endl;
    }
#endif

	Emulator e = Emulator(program, programSize);
    delete[] program;
	e.startEmulator();
}