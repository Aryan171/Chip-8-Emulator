#include "emulator.h"
#include "chip8IO.h"

#define PRINT_INSTRUCTION
#define PRINT_SPRITE
/* when SLOW_EXECUTION is defined one instruction will be processed
 only after a key (0-9 or a-f) is pressed*/
//#define SLOW_EXECUTION
/* when FAST_EXECUTION is defined there is minimum amount of delay
 between the processing of two instructions*/
//#define FAST_EXECUTION
#define PRINT_REGISTERS


Emulator::Emulator(const uint8_t* program, int programLength) {
	// allocating 4kb of memory for the program to run
	memory = new uint8_t[4096];

	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}

	// loading the program in memory
	for (int i = 0; i < programLength; ++i) {
		memory[i + 512] = program[i];
	}

	// allocating memory for the V
	V = new uint8_t[16];

	for (int i = 0; i < 16; ++i) {
		V[i] = 0;
	}

	// allocating memory for the display
	display = new uint8_t*[displayX];

	for (int i = 0; i < displayX; ++i) {
		display[i] = new uint8_t[displayY];
		for (int j = 0; j < displayY; ++j) {
			display[i][j] = 0;
		}
	}

	// allocating memory for the keyboard keys
	keyboard = new uint8_t[16];

	for (int i = 0; i < 16; ++i) {
		keyboard[i] = 0;
	}

	// storing the sprites of hexadecimal digits in memory
	uint8_t digitSprites[] = { 0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80 };

	for (int i = 0; i < 80; ++i) {
		memory[i] = digitSprites[i];
	}

	// allocating memory for the stack
	stack = new uint16_t[50];
}

void Emulator::startEmulator() {
	bool newKeyPressed = false;
	uint8_t newKeyCode;

	auto func = [&](uint8_t key) {
		newKeyPressed = true;
		newKeyCode = key;
		};

	Chip8IO io(display, keyboard, func);

	std::thread ioThread([&]() {
		io.startIO();
	});

	// timer used for delay timer and sound timer registers
	sf::Clock timer;

#ifdef PRINT_INSTRUCTION
	std::cout << "Location | Instruction" << std::endl;
#endif

	while (true) {
		// starting the timer
		sf::Clock clock;

		// forming the 16 bit instruction from two 8 bit numbers
		uint16_t ins = (static_cast<uint16_t>(memory[PC]) << 8) | memory[PC + 1];

		// finding the four nibbles of the instruction
		uint16_t a = ins >> 12,
			b = (ins & 0x0F00) >> 8,
			c = (ins & 0x00F0) >> 4,
			d = ins & 0x000F;

#ifdef PRINT_REGISTERS
		std::cout << "V[0-F] = ";
		for (int i = 0; i < 16; ++i) {
			std::cout << static_cast<int>(V[i]) << ", ";
		}

		std::cout << "I = " << I << " S = " << SP << " PC = " << PC << std::endl;
#endif

#ifdef PRINT_INSTRUCTION
		std::cout << PC << "|" << toHex(a) << toHex(b) << toHex(c) << toHex(d) << std::endl;
#endif
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
			else {
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
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

			PC = getLastThreeNibbles(ins);

			break;

		case hx3:
			// 3xkk - SE Vx, byte
			if (V[b] == getLastTwoNibbles(ins)) {
				// we increment by 4 instead of 2 as one instruction is two 
				// bytes long
				PC += 4;
			}
			else {
				PC += 2;
			}

			break;

		case hx4:
			// 4xkk - SNE Vx, byte
			if (V[b] != getLastTwoNibbles(ins)) {
				// we increment by 4 instead of 2 as one instruction is two 
				// bytes long
				PC += 4;
			}
			else {
				PC += 2;
			}

			break;

		case hx5:
			// 5xy0 - SE Vx, Vy
			if (d == hx0) {
				if (V[b] == V[c]) {
					// we increment by 4 instead of 2 as one instruction is two 
					// bytes long
					PC += 4;
				}
				else {
					PC += 2;
				}
			}
			else {
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
				std::cout << "|--------------\n";
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

			case hx4: {
				uint16_t sum = static_cast<uint16_t>(V[b]) + static_cast<uint16_t>(V[c]);

				if ((sum & 0xFFFF0000) == 0) {
					V[15] = 0;
				}
				else {
					V[15] = 1;
				}

				V[b] = static_cast<uint8_t>(sum & 0x0000FFFF);
				break;
			}

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
			default:
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
				std::cout << "|--------------\n";
				return;
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
			else {
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
				std::cout << "|--------------\n";
			}

			break;

		case hxA:
			// Annn - LD I, addr
			I = getLastThreeNibbles(ins);
			PC += 2;

			break;

		case hxB:
			// Bnnn - JP V0, addr
			PC = getLastThreeNibbles(ins) + V[0];

			break;

		case hxC: {
			// Cxkk - RND Vx, byte

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(0, 255);

			V[b] = getLastTwoNibbles(ins) & distrib(gen);

			PC += 2;

			break;
		}

		case hxD: {
			// Dxyn - DRW Vx, Vy, nibble

			V[15] = 0;
#ifdef PRINT_SPRITE
			std::cout << "Drawing sprite at " << static_cast<int>(b) << ", " 
				<< static_cast<int>(c) << std::endl;
#endif
			// looping through all the bytes that store the sprite
			for (uint16_t i = 0; i < d; ++i) {

#ifdef PRINT_SPRITE
				std::cout << toHex((static_cast<uint16_t>(memory[i + I]) & 0x00F0) >> 4) <<
					toHex(static_cast<uint16_t>(memory[i + I]) & 0x000F) << std::endl;
#endif

				// temporary variable used to disylay the byte
				uint8_t t = 0x80;

				// used to store the final state of the pixel
				uint8_t f;

				// used to store the original state of the pixel
				uint8_t o;

				// displaying one byte of the sprite
				for (uint16_t j = 0; j < 8; ++j) {
					int x = (b + j) % displayX, y = (c + i) % displayY;
					o = display[x][y];
					if ((memory[i + I] & t) != 0) {
						if (o == 1) {
							f = 0;
							V[15] = 1;
						}
						else {
							f = 1;
						}
					}
					else {
						if (o == 1) {
							f = 1;
						}
						else {
							f = 0;
						}
					}

					// setting the bit on or off according to a
					display[x][y] = f;
#ifdef PRINT_SPRITE
					if (f == 1) {
						std::cout << "& ";
					}
					else {
						std::cout << ". ";
					}
#endif
					t >>= 1;
				}
#ifdef PRINT_SPRITE
				std::cout << std::endl;
#endif
			}

			PC += 2;
			break;
		}

		case hxE:
			// Ex9E - SKP Vx

			if (c == hx9 && d == hxE && keyboard[V[b]] == 1) {
				PC += 4;
			}
			else if (c == hxA && d == hx1 && keyboard[V[b]] == 0) {
				PC += 4;
			}
			else {
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
				std::cout << "|--------------\n";
				return;
			}
			break;

		case hxF:
			switch (getLastTwoNibbles(ins)) {
			case 0x07:
				// Fx07 - LD Vx, DT

				V[b] = DT;
				break;

			case 0x0A:
				newKeyPressed = false;
				while(true) {
					if (newKeyPressed) {
						V[b] = newKeyCode; 
						break;
					}
				}
				break;

			case 0x15:
				DT = V[b];
				break;

			case 0x18:
				ST = V[b];
				break;

			case 0x1E:
				I += V[b];
				break;

			case 0x29:
				I = 5 * V[b];
				break;

			case 0x33:
				memory[I] = V[b] / 100;
				memory[I + 1] = (V[b] % 100) / 10;
				memory[I + 2] = V[b] % 10;
				break;

			case 0x55:
				for (int i = 0; i <= b; ++i) {
					memory[I + i] = V[i];
				}
				break;

			case 0x65:
				for (int i = 0; i <= b; ++i) {
					V[i] = memory[I + i];
				}
				break;

			default:
				std::cout << "^\n";
				std::cout << "|Unknown OPCODE\n";
				std::cout << "|--------------\n";
				return;
			}
			PC += 2;
		}

#ifndef FAST_EXECUTION
		while (clock.getElapsedTime().asMicroseconds() < 10000) {}
#endif
		if (timer.getElapsedTime().asMicroseconds() > 16667) {
			if (ST > 0) {
				ST--;
			}
			if (DT > 0) {
				DT--;
			}

			timer.restart();
		}

#ifdef SLOW_EXECUTION
		newKeyPressed = false;
		while (!newKeyPressed) {}
#endif
	}
}

Emulator::~Emulator() {
	delete[] stack;
	delete[] keyboard;

	for (int i = 0; i < displayX; ++i) {
		delete[] display[i];
	}
	delete[] display;
	delete[] memory;
	delete[] V;
}

inline char Emulator::toHex(uint16_t nibble) {
	if (nibble < 0x000A) {
		return static_cast<char>(nibble) + '0';
	}
	else {
		return static_cast<char>(nibble) + 'A' - 10;
	}
}

inline uint16_t Emulator::getLastThreeNibbles(uint16_t instruction) {
	return instruction & 0x0FFF;
}

inline uint8_t Emulator::getLastTwoNibbles(uint16_t instruction) {
	return static_cast<uint8_t>(instruction & 0x00FF);
}