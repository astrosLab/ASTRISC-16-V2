#include "emulator.h"

struct ASTRISC_16::microOp {
	std::string name;
	std::function<void(int)> microOpFunc;
};

struct ASTRISC_16::opcode {
	std::string name;
	std::vector<microOp> microOps;
};

ASTRISC_16::ASTRISC_16() {
    memory.fill(0);
    generalRegisters.fill(0);
    specialRegisters.fill(0);
    running = false;
    debug = false;

    memory = {
        0b00001000,
        0b11111111,
        0b11111111
    };

	opcodes = {};
}

void ASTRISC_16::startCpu() {
    running = true;

    while (running == true) {
        fetchInstruction(specialRegisters[PC]); 
    }
}

void ASTRISC_16::fetchInstruction(const uint16_t& programCounter) {
    int instruction = memory[programCounter] >> 3; // First 5 bits of the instruction
}

