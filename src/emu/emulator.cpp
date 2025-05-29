#include "emulator.h"

ASTRISC_16::ASTRISC_16() {
    memory.fill(0);
    generalRegisters.fill(0);
    specialRegisters.fill(0);
    running = false;
    debug = false;

    memory = {
        0b00001000, // LDI R0 7
        0b00000000,
        0b00000111,
        0b00001001, // LDI R1 2
        0b00000000,
        0b00000010,
        0b01001000, // ADD R0 R1 R2
        0b00101000, 
        0b11111000, // HALT

        /* 0b00001000, // LDI R0 255
        0b00000000,
        0b11111111,
        0b01000000, // MOV R0 R1
        0b00100000,
        0b11111000, // HALT */
        
        /*0b00100000, // CALL 4
        0b00000000,
        0b00000100,
        0b11111000, // HALT
        0b00001000, // LDI R0 32
        0b00000000,
        0b00100000,
        0b00110000, // PUSH R0
        0b00111001, // POP R1
        0b00101000, // RET */
        
        /*0b00001001, // LDI R1 255
        0b00000000,
        0b11111111,
        0b00001000, // LDI R0 20
        0b00000000,
        0b00010100, 
        0b00011000, // STR R0 R1
        0b00100000,
        0b00010010, // LOD R3 R1
        0b00100000,
        0b11111000  // HALT */
    };

    specialRegisters[SP] = 0xFFF0;
    specialRegisters[CSP] = 0xFBF0;
    skipRemainingMicroOps = false;

    microOps = {
        {"INC_PC", [this](int param) { microINCPC(param); }},
        {"NOP", [this](int param) { microNOP(param); }},
        {"PARAM_TO_BUS", [this](int param) { microPARAMTOBUS(param); }},
        {"BUS_TO_REG", [this](int param) { microBUSTOREG(param); }},
        {"REG_TO_BUS", [this](int param) { microREGTOBUS(param); }},
        {"REG_TO_ADDR_BUS", [this](int param) { microREGTOADDRBUS(param); }},
        {"RAM_WRITE", [this](int param) { microRAMWRITE(param); }},
        {"RAM_TO_BUS", [this](int param) { microRAMTOBUS(param); }},
        {"SP_TO_ADDR_BUS", [this](int param) { microSPTOADDRBUS(param); }},
        {"INC_SP", [this](int param) { microINCSP(param); }},
        {"DEC_SP", [this](int param) { microDECSP(param); }},
        {"CSP_TO_ADDR_BUS", [this](int param) { microCSPTOADDRBUS(param); }},
        {"INC_CSP", [this](int param) { microINCCSP(param); }},
        {"DEC_CSP", [this](int param) { microDECCSP(param); }},
        {"PC_TO_BUS", [this](int param) { microPCTOBUS(param); }},

        {"BUS_TO_ALU_SRC1", [this](int param) { microBUSTOALUSRC1(param); }},
        {"BUS_TO_ALU_SRC2", [this](int param) { microBUSTOALUSRC2(param); }},
        {"BUS_TO_ALU_MODE", [this](int param) { microBUSTOALUMODE(param); }},
        {"ALU_TO_BUS", [this](int param) { microALUTOBUS(param); }},

        {"HALT", [this](int param) { microHALT(param); }},
    };

	opcodeBytes = {
        {0, 1}, // NOP
        {1, 3}, // LDI
        {2, 2}, // LOD
        {3, 2}, // STR
        {4, 3}, // CALL
        {5, 1}, // RET
        {6, 1}, // PUSH
        {7, 1}, // POP
        {8, 2}, // MOV
        {9, 2}, // ADD
        {10, 2}, // SUB
        {11, 2}, // MUL
        {12, 2}, // DIV
        {13, 2}, // MOD
        {14, 2}, // INC
        {15, 2}, // DEC
        {16, 2}, // SHL
        {17, 2}, // SHR
        {18, 2}, // AND
        {19, 2}, // NAND
        {20, 2}, // OR
        {21, 2}, // NOR
        {22, 2}, // XOR
        {31, 1} // HALT
    };

    busRegister = 0;
}

void ASTRISC_16::startCpu() {
    running = true;

    while (running == true) {
        std::cout << unsigned(specialRegisters[PC]) << std::endl;
        skipRemainingMicroOps = false;
        fetchInstruction(specialRegisters[PC]);
        decodeInstruction();
        executeMicroOp();
        std::cout << std::endl;
    }
}

void ASTRISC_16::fetchInstruction(const uint16_t& programCounter) {
    int instructionIndex = memory[programCounter] >> 3; // First 5 bits of the instruction
    int instructionLength = opcodeBytes[instructionIndex];

    for (int i = 0; i < instructionLength; i++) {
        instructionRegister[i] = memory[programCounter + i];
    }
}

void ASTRISC_16::decodeInstruction() {
    int instruction = instructionRegister[0] >> 3;

    switch (instruction) {
        case 0: // NOP
            microOpQueue.push_back({microOps["INC_PC"], 1});
            microOpQueue.push_back({microOps["HALT"], 0});
            break;
        case 1: // LDI
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["INC_PC"], 3});
            break;
        case 2: // LOD
            microOpQueue.push_back({microOps["REG_TO_ADDR_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["RAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["INC_PC"], 2});
            break;
        case 3: // STR
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["REG_TO_ADDR_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["RAM_WRITE"], 0});
            microOpQueue.push_back({microOps["INC_PC"], 2});
            break;
        case 4: // CALL
            microOpQueue.push_back({microOps["INC_PC"], 3});
            microOpQueue.push_back({microOps["DEC_CSP"], 0});
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["CSP_TO_ADDR_BUS"], 0});
            microOpQueue.push_back({microOps["RAM_WRITE"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 5: // RET
            microOpQueue.push_back({microOps["INC_PC"], 1});
            microOpQueue.push_back({microOps["CSP_TO_ADDR_BUS"], 0});
            microOpQueue.push_back({microOps["RAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["INC_CSP"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 6: // PUSH
            microOpQueue.push_back({microOps["INC_PC"], 1});
            microOpQueue.push_back({microOps["DEC_SP"], 0});
            microOpQueue.push_back({microOps["SP_TO_ADDR_BUS"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["RAM_WRITE"], 0});
            break;
        case 7: // POP
            microOpQueue.push_back({microOps["INC_PC"], 1});
            microOpQueue.push_back({microOps["SP_TO_ADDR_BUS"], 0});
            microOpQueue.push_back({microOps["RAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["INC_SP"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[0] & 7});
            break;
        case 8: // MOV
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["INC_PC"], 2});
            break;
        case 9: // ADD
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 10: // SUB
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 1});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 11: // MUL
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 2});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 12: // DIV
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 3});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 13: // MOD
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 4}); // HERE
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 14: // INC
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 15: // DEC
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 6});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 16: // SHL
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] >> 1) & 15});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 17: // SHR
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] >> 1) & 15});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 8});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 18: // AND 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 9});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 19: // NAND 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 10});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 20: // OR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 11});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 21: // NOR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 12});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 22: // XOR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_SRC2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 13});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 31: // HALT
            microOpQueue.push_back({microOps["HALT"], 0});
            break;
    }
}

void ASTRISC_16::executeMicroOp() {
    for (auto& [opcode, param] : microOpQueue) {
        opcode(param);
        if (skipRemainingMicroOps == true) {
            if (debug == true) 
                std::cout << "Clearing remaining micro ops" << std::endl;
            break;
        }
    }
    microOpQueue.clear();
}

void ASTRISC_16::microINCPC(const int& param) {
    if (debug == true)
        std::cout << "Increasing program counter by " << param << std::endl;
     
    specialRegisters[PC] += param;
}

void ASTRISC_16::microNOP(const int& param) {
    if (debug == true)
        std::cout << "No operation" << std::endl;
}

void ASTRISC_16::microPARAMTOBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing " << param << " to data bus" << std::endl;

    busRegister = param;
}

void ASTRISC_16::microBUSTOREG(const int& param) {
    if (debug == true)
        std::cout << "Writing " << busRegister << " to register " << param << std::endl;

    generalRegisters[param] = busRegister;
}

void ASTRISC_16::microREGTOBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing value in register " << param
        << " (" << generalRegisters[param] << ") to data bus" << std::endl;

    busRegister = generalRegisters[param];
}

void ASTRISC_16::microREGTOADDRBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing value in register " << param 
        << " (" << generalRegisters[param] << ") to address bus" << std::endl;

    addrBusRegister = generalRegisters[param];
}

void ASTRISC_16::microRAMWRITE(const int& param) {
    if (debug == true)
        std::cout << 
            "Writing to RAM using data bus (" << busRegister <<
            ") and address bus (" << addrBusRegister << ")" << std::endl;

    memory[addrBusRegister] = busRegister;
}

void ASTRISC_16::microRAMTOBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing RAM value at " << addrBusRegister
        << " (" << unsigned(memory[addrBusRegister]) << ") to data bus" << std::endl;

    busRegister = memory[addrBusRegister];
}

void ASTRISC_16::microSPTOADDRBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing SP (" << specialRegisters[SP] << ") to address bus" << std::endl;

    addrBusRegister = specialRegisters[SP];
}

void ASTRISC_16::microINCSP(const int& param) {
    if (debug == true)
        std::cout << "Incrementing SP by 1 (" << specialRegisters[SP] + 1 << ")" << std::endl;

    if (specialRegisters[SP] != 0xFFF0) {
        specialRegisters[SP]++;
    } else {
        if (debug == true)
            std::cout << "Stack underflow, not incrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 2;
    }
}

void ASTRISC_16::microDECSP(const int& param) {
    if (debug == true)
        std::cout << "Decrementing SP by 1 (" << specialRegisters[SP] - 1 << ")" << std::endl;

    if (specialRegisters[SP] != 0xFBEF) {
        specialRegisters[SP]--;
    } else {
        if (debug == true)
            std::cout << "Stack overflow, not decrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 1;
    }
}

void ASTRISC_16::microCSPTOADDRBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing CSP (" << specialRegisters[CSP] << ") to address bus" << std::endl;

    addrBusRegister = specialRegisters[CSP];
}

void ASTRISC_16::microINCCSP(const int& param) {
    if (debug == true)
        std::cout << "Incrementing CSP by 1 (" << specialRegisters[CSP] + 1 << ")" << std::endl;

    if (specialRegisters[CSP] != 0xFBF0) {
        specialRegisters[CSP]++;
    } else {
        if (debug == true)
            std::cout << "Call stack underflow, not incrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 4;
    }
}

void ASTRISC_16::microDECCSP(const int& param) {
    if (debug == true)
        std::cout << "Decrementing CSP by 1 (" << specialRegisters[CSP] - 1 << ")" << std::endl;

    if (specialRegisters[CSP] != 0xF7EF) {
        specialRegisters[CSP]--;
    } else {
        if (debug == true)
            std::cout << "Call stack overflow, not decrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 3;
    }
}
    
void ASTRISC_16::microPCTOBUS(const int& param) {
    if (debug == true)
        std::cout << "Writing PC (" << specialRegisters[PC] << ") to data bus" << std::endl;

    busRegister = specialRegisters[PC];
}

void ASTRISC_16::microBUSTOPC(const int& param) {
    if (debug == true)
        std::cout << "Writing data bus (" << busRegister << ") to PC" << std::endl;

    specialRegisters[PC] = busRegister;
}

void ASTRISC_16::microBUSTOALUSRC1(const int& param) {
    if (debug == true)
        std::cout << "Writing data bus (" << busRegister << ") to ALU SRC 1" << std::endl;

    aluSrc1 = busRegister;
}

void ASTRISC_16::microBUSTOALUSRC2(const int& param) {
    if (debug == true)
        std::cout << "Writing data bus (" << busRegister << ") to ALU SRC 2" << std::endl;

    aluSrc2 = busRegister;
}

void ASTRISC_16::microBUSTOALUMODE(const int& param) {
    if (debug == true)
        std::cout << "Writing data bus (" << busRegister << ") to ALU MODE" << std::endl;

    aluMode = busRegister & 15;
}

void ASTRISC_16::microALUTOBUS(const int& param) {
    uint16_t result;

    switch (aluMode) {
        case 0: // ADD
            result = aluSrc1 + aluSrc2;
            break;
        case 1: // SUB
            result = aluSrc1 - aluSrc2;
            break;
        case 2: // MUL
            result = aluSrc1 * aluSrc2;
            break;
        case 3: // DIV
            if (aluSrc2 == 0) {
                if (debug == true)
                    std::cout << "Division by zero" << std::endl;

                skipRemainingMicroOps = true;
                specialRegisters[EC] = 5;
                break;
            }
            result = floor(aluSrc1 / aluSrc2);
            break;
        case 4: // MOD
            if (aluSrc2 == 0) {
                if (debug == true)
                    std::cout << "Division by zero" << std::endl;

                skipRemainingMicroOps = true;
                specialRegisters[EC] = 5;
                break;
            }
            result = aluSrc1 % aluSrc2;
            break;
        case 5: // INC
            result = aluSrc1 + 1;
            break;
        case 6: // DEC
            result = aluSrc1 - 1;
            break;
        case 7: // SHL
            result = aluSrc1 << aluSrc2;
            break;
        case 8: // SHR
            result = aluSrc1 >> aluSrc2;
            break;
        case 9: // AND
            result = aluSrc1 & aluSrc2;
            break;
        case 10: // NAND
            result = ~(aluSrc1 & aluSrc2);
            break;
        case 11: // OR
            result = aluSrc1 | aluSrc2;
            break;
        case 12: // NOR
            result = ~(aluSrc1 | aluSrc2);
            break;
        case 13: // XOR
            result = aluSrc1 ^ aluSrc2;
            break;
    };

    if (debug == true)
        std::cout << "Writing ALU output (" << result << ") to data bus" << std::endl;

    busRegister = result;
}

void ASTRISC_16::microHALT(const int& param) {
    if (debug == true)
        std::cout << "Halting CPU" << std::endl;
    
    running = false;
}

