#ifndef A16_EMU_H
#define A16_EMU_H

#include <map>
#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <functional>
#include <cmath>
#include <thread>

class ASTRISC_16 {
public:
    ASTRISC_16();

    bool debug = false;
    int hertz = 1000;
    enum specialRegisterNames {
        PC, // Program Counter
        EC, // Error Code
        FLAGS, // Flags, Bit Order: [ ZERO, SIGNED, CARRY, OVERFLOW ]
        INTERRUPT, // Interrupt Status
        SP, // Stack Pointer
        CSP, // Call Stack Pointer
        NUM_SPECIAL_REGISTERS 
    };

    void startCpu();
    void fetchInstruction(const uint16_t& programCounter);
    void decodeInstruction();
    void executeMicroOp();

    void microINCPC(const int& param);
    void microNOP(const int& param);
    void microPARAMTOBUS(const int& param);
    void microBUSTOREG(const int& param);
    void microREGTOBUS(const int& param);
    void microREGTOADDRBUS(const int& param);
    void microRAMWRITE(const int& param);
    void microRAMTOBUS(const int& param);
    void microSPTOADDRBUS(const int& param);
    void microINCSP(const int& param);
    void microDECSP(const int& param);
    void microCSPTOADDRBUS(const int& param);
    void microINCCSP(const int& param);
    void microDECCSP(const int& param);
    void microPCTOBUS(const int& param);
    void microBUSTOPC(const int& param);
    void microBUSTOARG1(const int& param);
    void microBUSTOARG2(const int& param);
    void microBUSTOALUMODE(const int& param);
    void microALUTOBUS(const int& param);
    void microCMPTOFLAGS(const int& param); 
    void microCONTINUEIFFLAG(const int& param);
    void microHALT(const int& param);
private:
    std::array<uint8_t, 65536> memory;
    std::array<uint16_t, 8> generalRegisters;
    std::array<uint16_t, NUM_SPECIAL_REGISTERS> specialRegisters;
    std::array<uint8_t, 3> instructionRegister;
    bool running;
	struct opcode {
    	std::string name;
        int bytes;
    	std::vector<std::function<void(int)>> microOps;
    };
    std::map<std::string, std::function<void(int)>> microOps;
    std::vector<std::pair<std::function<void(int)>, int>> microOpQueue;
	std::unordered_map<int, int> opcodeBytes;

    // Registers/Flags
    uint16_t busRegister;
    uint16_t addrBusRegister;
    bool skipRemainingMicroOps;
    uint16_t arg1;
    uint16_t arg2;
    uint8_t aluMode;
};

#endif // A16_EMU_H
