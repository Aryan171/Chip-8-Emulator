#pragma once

#include <iostream>
#include <random>
#include <thread>
#include <functional>

class Emulator {
public:
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
		hxA = 0x00A,
		hxB = 0x00B,
		hxC = 0x00C,
		hxD = 0x00D,
		hxE = 0x00E,
		hxF = 0x00F;

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

	// 1 means pressed 0 means not pressed
	uint8_t* keyboard;

	int displayX = 64, displayY = 32;

	Emulator();
	void startEmulator();
	inline uint16_t getLastThreeNibbles(uint16_t instruction);
	inline uint8_t getLastTwoNibbles(uint16_t instruction);
};