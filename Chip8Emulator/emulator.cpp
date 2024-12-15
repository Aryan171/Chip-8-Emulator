#include <iostream>

struct Emulator {
	uint8_t* memory;
	uint8_t* V;

	static const uint16_t hx0 = 0x000,
		hx1 = 0x001,
		hx2 = 0x002,
		hx3 = 0x003,
		hx4 = 0x004,
		hx5 = 0x005,
		hx6 = 0x006,
		hx7 = 0x007,
		hx8 = 0x008,
		hx9 = 0x009,
		hxA = 0x010,
		hxB = 0x011,
		hxC = 0x012,
		hxD = 0x013,
		hxE = 0x014,
		hxF = 0x015;

	/*program counter and stack pointer registers 
	program counter is set to 512 as most chip 8 
	programs start at memory location 512 */
	uint16_t PC = 512, SP = -1;

	uint16_t* stack;
	 
	/*delay time register and sound time register,
	both these registers are decremented at 60Hz*/
	uint16_t DT, ST;

	uint16_t I;

	uint8_t** display;

	int displayX = 64, displayY = 32;

	Emulator() {
		// allocating 4kb of memory for the program to run
		memory = new uint8_t[4096];

		// allocating memory for the V
		V = new uint8_t[16];

		// allocating memory for the display
		display = new uint8_t*[displayX];

		for (int i = 0; i < displayX; ++i) {
			display[i] = new uint8_t[displayY];
		}

		// allocating memory for the stack
		stack = new uint16_t[50];
	}

	void startEmulator() {
		while (true) {
			uint16_t ins = getInstruction(memory[PC], memory[PC + 1]);

			// finding the four nibbles of the instruction
			uint16_t a = getFirstNibble(ins),
				b = getSecondNibble(ins),
				c = getThirdNibble(ins),
				d = getFourthNibble(ins);

			switch (a) {
			case hx0:
				// 0nnn is ignored 

				// 00E0 - CLS
				// Clear the display.
				if (b == hx0 && c == hxE && d == hx0) {
					for (int i = 0; i < displayX; ++i) {
						for (int j = 0; j < displayY; ++j) {
							display[i][j] = 0;
						}
					}
					
					PC += 2;
				}

				// 00EE - RET
				// Return from a subroutine.
				// The interpreter sets the program counter to the address at the
				// top of the stack, then subtracts 1 from the stack pointer.

				else if (b == hx0 && c == hxE && d == hxE) {
					PC = stack[SP];
					SP--;
				}

				break;

			case hx1:
				// 1nnn - JP addr
				// Jump to location nnn.
				// The interpreter sets the program counter to nnn.

				PC = getLastThreeNibbles(ins);

				break;

			case hx2:
				// 2nnn - CALL addr
				// Call subroutine at nnn.
				// The interpreter increments the stack pointer, then puts the 
				// current PC on the top of the stack.The PC is then set to nnn.

				SP++;
				stack[SP] = PC;

				PC += 2;

				break;

			case hx3:
				// 3xkk - SE Vx, byte
				// Skip next instruction if Vx = kk.
				// The interpreter compares register Vx to kk, and if they are 
				// equal, increments the program counter by 2.

				if (V[b] == getLastTwoNibbles(ins)) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx4:
				// 4xkk - SNE Vx, byte
				// Skip next instruction if Vx != kk.
				// The interpreter compares register Vx to kk, and if they are 
				// not equal, increments the program counter by 2.

				if (V[b] != getLastTwoNibbles(ins)) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx5:
				// 5xy0 - SE Vx, Vy
				// Skip next instruction if Vx = Vy.
				// The interpreter compares register Vx to register Vy, and if 
				// they are equal, increments the program counter by 2.

				if (d == hx0 && V[b] == V[c]) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx6:
				// 6xkk - LD Vx, byte
				// Set Vx = kk.
				// The interpreter puts the value kk into register Vx.

				V[b] = getLastTwoNibbles(ins);

				PC += 2;

				break;

			case hx7:
				// 7xkk - ADD Vx, byte
				// Set Vx = Vx + kk.
				// Adds the value kk to the value of register Vx, then 
				// stores the result in Vx.

				V[b] += getLastTwoNibbles(ins);

				PC += 2;

				break;

			}
		}
	}

	inline uint16_t getFirstNibble(uint16_t instruction) {
		return instruction >> 12;
	}

	inline uint16_t getSecondNibble(uint16_t instruction) {
		return (instruction & 0x0FFF) >> 12;
	}

	inline uint16_t getThirdNibble(uint16_t instruction) {
		return (instruction & 0x00FF) >> 12;
	}

	inline uint16_t getFourthNibble(uint16_t instruction) {
		return (instruction & 0x000F) >> 12;
	}

	inline uint16_t getInstruction(uint8_t a, uint8_t b) {
		return static_cast<uint16_t>(a) | b;
	}

	inline uint16_t getLastThreeNibbles(uint16_t instruction) {
		return instruction & 0x0FFF;
	}

	inline uint8_t getLastTwoNibbles(uint16_t instruction) {
		return static_cast<uint8_t>(instruction & 0x00FF);
	}
};