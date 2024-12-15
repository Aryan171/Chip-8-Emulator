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
	uint16_t DT{}, ST{};

	uint16_t I{};

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
				if (b == hx0 && c == hxE && d == hx0) {
					for (int i = 0; i < displayX; ++i) {
						for (int j = 0; j < displayY; ++j) {
							display[i][j] = 0;
						}
					}
					
					PC += 2;
				}

				// 00EE - RET
				else if (b == hx0 && c == hxE && d == hxE) {
					PC = stack[SP];
					SP--;
				}

				break;

			case hx1:
				// 1nnn - JP addr
				PC = getLastThreeNibbles(ins);

				break;

			case hx2:
				// 2nnn - CALL addr
				SP++;
				stack[SP] = PC;

				PC += 2;

				break;

			case hx3:
				// 3xkk - SE Vx, byte
				if (V[b] == getLastTwoNibbles(ins)) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx4:
				// 4xkk - SNE Vx, byte
				if (V[b] != getLastTwoNibbles(ins)) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx5:
				// 5xy0 - SE Vx, Vy
				if (d == hx0 && V[b] == V[c]) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}

				break;

			case hx6:
				// 6xkk - LD Vx, byte
				V[b] = getLastTwoNibbles(ins);

				PC += 2;

				break;

			case hx7:
				// 7xkk - ADD Vx, byte
				V[b] += getLastTwoNibbles(ins);

				PC += 2;

				break;

			case hx8:
				switch (d) {
				case hx0:
					V[b] = V[c];
					break;
					
				case hx1:
					V[b] |= V[c];
					break;

				case hx2:
					V[b] &= V[c];
					break;

				case hx3:
					V[b] ^= V[c];
					break;

				case hx4:
					uint16_t sum = static_cast<uint16_t>(V[b]) + static_cast<uint16_t>(V[c]);

					if ((sum & 0xFFFF0000) == 0) {
						V[15] = 0;
					}
					else {
						V[15] = 1;
					}

					V[b] = static_cast<uint8_t>(sum & 0x0000FFFF);

					break;

				case hx5:
					if (V[b] >= V[c]) {
						V[15] = 1;
					}
					else {
						V[15] = 0;
					}

					V[b] -= V[c];

					break;

				case hx6:
					if ((V[b] & 0x0001) == 1) {
						V[15] = 1;
					}
					else {
						V[15] = 0;
					}

					// ik the compiler optimizes division like this but still ...
					V[b] >>= 1;

					break;

				case hx7:
					if (V[b] >= V[c]) {
						V[15] = 0;
					}
					else {
						V[15] = 1;
					}

					V[b] = V[c] - V[b];

					break;

				case hxE:
					if ((V[b] & 0x0001) == 1) {
						V[15] = 1;
					}
					else {
						V[15] = 0;
					}

					// ik the compiler optimizes division like this but still ...
					V[b] <<= 1;

					break;
				}

				PC += 2;

				break;

			case hx9:
				// 9xy0 - SNE Vx, Vy
				if (d == 0) {
					if (V[b] != V[c]) {
						PC += 4;
					}
					else {
						PC += 2;
					}
				}

				break;

			case hxA:
				// Annn - LD I, addr
				I = getLastThreeNibbles(ins);

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