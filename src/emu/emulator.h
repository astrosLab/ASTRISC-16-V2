#ifndef A16_EMU_H
#define A16_EMU_H

#include <map>
#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <functional>

class ASTRISC_16 {
public:
    ASTRISC_16();

    enum specialRegisterNames {
        PC, // Program Counter
        EC, // Error Code
        FLAGS, // Flags
        INTERRUPT, // Interrupt Status
        SP, // Stack Pointer
        CSP, // Call Stack Pointer
        NUM_SPECIAL_REGISTERS 
    };

    void startCpu();
    void fetchInstruction(const uint16_t& programCounter);
    void decodeInstruction();
    void pushMicroOps();
    void executeMicroOp();
private:
    std::array<uint8_t, 65536> memory;
    std::array<uint16_t, 8> generalRegisters;
    std::array<uint16_t, NUM_SPECIAL_REGISTERS> specialRegisters;
    std::array<uint8_t, 3> instructionRegister;
    bool running;
    bool debug;
    int hertz;
	struct microOp;
	struct opcode;
	std::vector<opcode> opcodes;
};

#endif // A16_EMU_H
