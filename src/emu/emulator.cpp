#include "emulator.h"

ASTRISC_16::ASTRISC_16() {
    memory.fill(0);
    generalRegisters.fill(0);
    specialRegisters.fill(0);
    running = false;
    debug = false;
    hertz = 1000;
    haltNop = false;

    memory = {
        0b11000000, // JMP 17
        0b00000000,
        0b00010001,
        0b01001000, // H
        0b01100101, // e
        0b01101100, // l
        0b01101100, // l
        0b01101111, // o
        0b00101100, // ,
        0b00100000, // 
        0b01010111, // W
        0b01101111, // o
        0b01110010, // r
        0b01101100, // l
        0b01100100, // d
        0b00100001, // !
        0b00001010, // \n
        0b00001000, // LDI R0 3
        0b00000000,
        0b00000011,
        0b00001001, // LDI R1 17
        0b00000000,
        0b00010001,
        0b00001010, // LDI R2 0xFFF0
        0b11111111,
        0b11110000,
        0b00010011, // LOD R3 R0
        0b00000000,
        0b00011011, // STR R3 R2
        0b01000000,
        0b01110000, // INC R0 -> R0
        0b00000000,
        0b10111000, // CMP R0 R1
        0b00100000,
        0b11010000, // BS -11
        0b11111111,
        0b11110101,
        0b11111000, // HALT
    };

    specialRegisters[SP] = 0xFFF0;
    specialRegisters[CSP] = 0xFBF0;
    skipRemainingMicroOps = false;

    microOps = {
        {"INC_PC", [this](int param) { microINCPC(param); }},
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
        {"BUS_TO_PC", [this](int param) { microBUSTOPC(param); }},
        {"BUS_TO_ARG1", [this](int param) { microBUSTOARG1(param); }},
        {"BUS_TO_ARG2", [this](int param) { microBUSTOARG2(param); }},
        {"BUS_TO_ALU_MODE", [this](int param) { microBUSTOALUMODE(param); }},
        {"ALU_TO_BUS", [this](int param) { microALUTOBUS(param); }},
        {"CMP_TO_FLAGS", [this](int param) { microCMPTOFLAGS(param); }},
        {"CONTINUE_IF_FLAG", [this](int param) { microCONTINUEIFFLAG(param); }},
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
        {23, 3}, // CMP
        {24, 3}, // JMP
        {25, 3}, // BZ
        {26, 3}, // BS
        {27, 3}, // BC
        {28, 3}, // BO
        {29, 1}, // NONE
        {30, 1}, // NONE
        {31, 1} // HALT
    };

    busRegister = 0;
}

void ASTRISC_16::startCpu() {
    int cycleTime = 1000 / hertz;
    running = true;

    while (running == true) {
        if (debug)
            std::cout << unsigned(specialRegisters[PC]) << std::endl;

        updateSpecialRegisters();
        skipRemainingMicroOps = false;
        fetchInstruction(specialRegisters[PC]);
        decodeInstruction();
        executeMicroOp();

        if (memory[0xFFF0]) {
            std::cout << (char)memory[0xFFF0];
            memory[0xFFF0] = 0;
        }

        if (debug)
            std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
    }
}

void ASTRISC_16::updateSpecialRegisters() {
    // Program Counter
    memory[0xF7E5] = (specialRegisters[PC] & 0xFF00) >> 8;
    memory[0xF7E6] = specialRegisters[PC] & 0xFF;
    // Error Code
    memory[0xF7E7] = specialRegisters[EC];
    // Flags
    memory[0xF7E8] = specialRegisters[FLAGS];
    // Interrupt Status
    memory[0xF7E9] = specialRegisters[I_STATUS];
    // Interrupt Address Location
    memory[0xF7EA] = (specialRegisters[I_ADDR] & 0xFF00) >> 8;
    memory[0xF7EB] = specialRegisters[I_ADDR] & 0xFF;
    // Stack Pointer
    memory[0xF7EC] = (specialRegisters[SP] & 0xFF00) >> 8;
    memory[0xF7ED] = specialRegisters[SP] & 0xFF;
    // Call Stack Pointer
    memory[0xF7EE] = (specialRegisters[CSP] & 0xFF00) >> 8;
    memory[0xF7EF] = specialRegisters[CSP] & 0xFF;
}

void ASTRISC_16::checkInterrupt() {
    if (specialRegisters[I_STATUS] == 1 && interruptPin == true) {
        microOpQueue.push_back({microOps["INC_PC"], 3});
        microOpQueue.push_back({microOps["DEC_CSP"], 0});
        microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
        microOpQueue.push_back({microOps["CSP_TO_ADDR_BUS"], 0});
        microOpQueue.push_back({microOps["RAM_WRITE"], 0});
        microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
        microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
        executeMicroOp();
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
            if (haltNop)
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
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 10: // SUB
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 1});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 11: // MUL
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 2});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 12: // DIV
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 3});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 13: // MOD
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 4}); // HERE
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 14: // INC
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 5});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 15: // DEC
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 6});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 16: // SHL
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] >> 1) & 15});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 7});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 17: // SHR
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] >> 1) & 15});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 8});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], instructionRegister[1] >> 5});
            break;
        case 18: // AND 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 9});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 19: // NAND 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 10});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 20: // OR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 11});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 21: // NOR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 12});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 22: // XOR 
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 13});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_REG"], (instructionRegister[1] >> 2) & 7});
            break;
        case 23: // CMP
            microOpQueue.push_back({microOps["INC_PC"], 2});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[0] & 7});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["REG_TO_BUS"], instructionRegister[1] >> 5});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["CMP_TO_FLAGS"], 0});
            break;
        case 24: // JMP
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 25: // BZ
            microOpQueue.push_back({microOps["INC_PC"], 3});
            microOpQueue.push_back({microOps["CONTINUE_IF_FLAG"], 0b1000});
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 26: // BS 
            microOpQueue.push_back({microOps["INC_PC"], 3});
            microOpQueue.push_back({microOps["CONTINUE_IF_FLAG"], 0b0100});
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 27: // BC
            microOpQueue.push_back({microOps["INC_PC"], 3});
            microOpQueue.push_back({microOps["CONTINUE_IF_FLAG"], 0b0010});
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 28: // BO
            microOpQueue.push_back({microOps["INC_PC"], 3});
            microOpQueue.push_back({microOps["CONTINUE_IF_FLAG"], 0b0001});
            microOpQueue.push_back({microOps["PC_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ARG1"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], (instructionRegister[1] << 8) + instructionRegister[2]});
            microOpQueue.push_back({microOps["BUS_TO_ARG2"], 0});
            microOpQueue.push_back({microOps["PARAM_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_ALU_MODE"], 0});
            microOpQueue.push_back({microOps["ALU_TO_BUS"], 0});
            microOpQueue.push_back({microOps["BUS_TO_PC"], 0});
            break;
        case 29: // NONE / NOP
        case 30: 
            microOpQueue.push_back({microOps["INC_PC"], 1});
            microOpQueue.push_back({microOps["HALT"], 0});
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
            if (debug) 
                std::cout << "Clearing remaining micro ops" << std::endl;
            break;
        }
    }
    microOpQueue.clear();
}

void ASTRISC_16::microINCPC(const int& param) {
    if (debug)
        std::cout << "Increasing program counter by " << param << std::endl;
     
    specialRegisters[PC] += param;
}

void ASTRISC_16::microNOP(const int& param) {
    if (debug)
        std::cout << "No operation" << std::endl;
}

void ASTRISC_16::microPARAMTOBUS(const int& param) {
    if (debug)
        std::cout << "Writing " << param << " to data bus" << std::endl;

    busRegister = param;
}

void ASTRISC_16::microBUSTOREG(const int& param) {
    if (debug)
        std::cout << "Writing " << busRegister << " to register " << param << std::endl;

    generalRegisters[param] = busRegister;
}

void ASTRISC_16::microREGTOBUS(const int& param) {
    if (debug)
        std::cout << "Writing value in register " << param
        << " (" << generalRegisters[param] << ") to data bus" << std::endl;

    busRegister = generalRegisters[param];
}

void ASTRISC_16::microREGTOADDRBUS(const int& param) {
    if (debug)
        std::cout << "Writing value in register " << param 
        << " (" << generalRegisters[param] << ") to address bus" << std::endl;

    addrBusRegister = generalRegisters[param];
}

void ASTRISC_16::microRAMWRITE(const int& param) {
    if (debug)
        std::cout << 
            "Writing to RAM using data bus (" << busRegister <<
            ") and address bus (" << addrBusRegister << ")" << std::endl;

    switch (addrBusRegister) {
        case 0xF7E5:
            specialRegisters[PC] = ((busRegister & 0xFF) << 8) | (specialRegisters[PC] & 0x00FF);
            break;
        case 0xF7E6:
            specialRegisters[PC] = (specialRegisters[PC] & 0xFF00) | (busRegister & 0xFF);
            break;
        case 0xF7E7:
            specialRegisters[EC] = busRegister & 0xFF;
            break;
        case 0xF7E8:
            specialRegisters[FLAGS] = busRegister & 0xFF;
            break;
        case 0xF7E9:
            specialRegisters[I_STATUS] = busRegister & 0xFF;
            break;
        case 0xF7EA:
            specialRegisters[I_ADDR] = ((busRegister & 0xFF) << 8) | (specialRegisters[I_ADDR] & 0x00FF);
            break;
        case 0xF7EB:
            specialRegisters[I_ADDR] = (specialRegisters[I_ADDR] & 0xFF00) | (busRegister & 0xFF);
            break;
        case 0xF7EC:
            specialRegisters[SP] = ((busRegister & 0xFF) << 8) | (specialRegisters[SP] & 0x00FF);
            break;
        case 0xF7ED:
            specialRegisters[SP] = (specialRegisters[SP] & 0xFF00) | (busRegister & 0xFF);
            break;
        case 0xF7EE:
            specialRegisters[CSP] = ((busRegister & 0xFF) << 8) | (specialRegisters[CSP] & 0x00FF);
            break;
        case 0xF7EF:
            specialRegisters[CSP] = (specialRegisters[CSP] & 0xFF00) | (busRegister & 0xFF);
            break;
        default:
            memory[addrBusRegister] = busRegister;
            break;
    }
}

void ASTRISC_16::microRAMTOBUS(const int& param) {
    if (debug)
        std::cout << "Writing RAM value at " << addrBusRegister
                  << " (" << unsigned(memory[addrBusRegister]) << ") to data bus" << std::endl;

    switch (addrBusRegister) {
        case 0xF7E5:
            busRegister = (specialRegisters[PC] & 0xFF00) >> 8;
            break;
        case 0xF7E6:
            busRegister = specialRegisters[PC] & 0x00FF;
            break;
        case 0xF7E7:
            busRegister = specialRegisters[EC] & 0xFF;
            break;
        case 0xF7E8:
            busRegister = specialRegisters[FLAGS] & 0xFF;
            break;
        case 0xF7E9:
            busRegister = specialRegisters[I_STATUS] & 0xFF;
            break;
        case 0xF7EA:
            busRegister = (specialRegisters[I_ADDR] & 0xFF00) >> 8;
            break;
        case 0xF7EB:
            busRegister = specialRegisters[I_ADDR] & 0x00FF;
            break;
        case 0xF7EC:
            busRegister = (specialRegisters[SP] & 0xFF00) >> 8;
            break;
        case 0xF7ED:
            busRegister = specialRegisters[SP] & 0x00FF;
            break;
        case 0xF7EE:
            busRegister = (specialRegisters[CSP] & 0xFF00) >> 8;
            break;
        case 0xF7EF:
            busRegister = specialRegisters[CSP] & 0x00FF;
            break;
        default:
            busRegister = memory[addrBusRegister];
            break;
    }
}

void ASTRISC_16::microSPTOADDRBUS(const int& param) {
    if (debug)
        std::cout << "Writing SP (" << specialRegisters[SP] << ") to address bus" << std::endl;

    addrBusRegister = specialRegisters[SP];
}

void ASTRISC_16::microINCSP(const int& param) {
    if (debug)
        std::cout << "Incrementing SP by 1 (" << specialRegisters[SP] + 1 << ")" << std::endl;

    if (specialRegisters[SP] != 0xFFF0) {
        specialRegisters[SP]++;
    } else {
        if (debug)
            std::cout << "Stack underflow, not incrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 2;
    }
}

void ASTRISC_16::microDECSP(const int& param) {
    if (debug)
        std::cout << "Decrementing SP by 1 (" << specialRegisters[SP] - 1 << ")" << std::endl;

    if (specialRegisters[SP] != 0xFBF0) {
        specialRegisters[SP]--;
    } else {
        if (debug)
            std::cout << "Stack overflow, not decrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 1;
    }
}

void ASTRISC_16::microCSPTOADDRBUS(const int& param) {
    if (debug)
        std::cout << "Writing CSP (" << specialRegisters[CSP] << ") to address bus" << std::endl;

    addrBusRegister = specialRegisters[CSP];
}

void ASTRISC_16::microINCCSP(const int& param) {
    if (debug)
        std::cout << "Incrementing CSP by 1 (" << specialRegisters[CSP] + 1 << ")" << std::endl;

    if (specialRegisters[CSP] != 0xFBF0) {
        specialRegisters[CSP]++;
    } else {
        if (debug)
            std::cout << "Call stack underflow, not incrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 4;
    }
}

void ASTRISC_16::microDECCSP(const int& param) {
    if (debug)
        std::cout << "Decrementing CSP by 1 (" << specialRegisters[CSP] - 1 << ")" << std::endl;

    if (specialRegisters[CSP] != 0xF7F0) {
        specialRegisters[CSP]--;
    } else {
        if (debug)
            std::cout << "Call stack overflow, not decrementing" << std::endl;

        skipRemainingMicroOps = true;
        specialRegisters[EC] = 3;
    }
}
    
void ASTRISC_16::microPCTOBUS(const int& param) {
    if (debug)
        std::cout << "Writing PC (" << specialRegisters[PC] << ") to data bus" << std::endl;

    busRegister = specialRegisters[PC];
}

void ASTRISC_16::microBUSTOPC(const int& param) {
    if (debug)
        std::cout << "Writing data bus (" << busRegister << ") to PC" << std::endl;

    specialRegisters[PC] = busRegister;
}

void ASTRISC_16::microBUSTOARG1(const int& param) {
    if (debug)
        std::cout << "Writing data bus (" << busRegister << ") to ARG 1" << std::endl;

    arg1 = busRegister;
}

void ASTRISC_16::microBUSTOARG2(const int& param) {
    if (debug)
        std::cout << "Writing data bus (" << busRegister << ") to ARG 2" << std::endl;

    arg2 = busRegister;
}

void ASTRISC_16::microBUSTOALUMODE(const int& param) {
    if (debug)
        std::cout << "Writing data bus (" << busRegister << ") to ALU MODE" << std::endl;

    aluMode = busRegister & 15;
}

void ASTRISC_16::microALUTOBUS(const int& param) {
    uint16_t result = 0;

    switch (aluMode) {
        case 0: // ADD
            result = arg1 + arg2;
            break;
        case 1: // SUB
            result = arg1 - arg2;
            break;
        case 2: // MUL
            result = arg1 * arg2;
            break;
        case 3: // DIV
            if (arg2 == 0) {
                if (debug)
                    std::cout << "Division by zero" << std::endl;

                skipRemainingMicroOps = true;
                specialRegisters[EC] = 5;
                break;
            }
            result = floor(arg1 / arg2);
            break;
        case 4: // MOD
            if (arg2 == 0) {
                if (debug)
                    std::cout << "Division by zero" << std::endl;

                skipRemainingMicroOps = true;
                specialRegisters[EC] = 5;
                break;
            }
            result = arg1 % arg2;
            break;
        case 5: // INC
            result = arg1 + 1;
            break;
        case 6: // DEC
            result = arg1 - 1;
            break;
        case 7: // SHL
            result = arg1 << arg2;
            break;
        case 8: // SHR
            result = arg1 >> arg2;
            break;
        case 9: // AND
            result = arg1 & arg2;
            break;
        case 10: // NAND
            result = ~(arg1 & arg2);
            break;
        case 11: // OR
            result = arg1 | arg2;
            break;
        case 12: // NOR
            result = ~(arg1 | arg2);
            break;
        case 13: // XOR
            result = arg1 ^ arg2;
            break;
    };

    if (debug)
        std::cout << "Writing ALU output (" << result << ") to data bus" << std::endl;

    busRegister = result;
}

void ASTRISC_16::microCMPTOFLAGS(const int& param) {
    uint16_t subtraction = arg1 - arg2;
    uint16_t flags = 0;

    bool src1Sign = (arg1 & 0x8000) != 0;
    bool src2Sign = (arg2 & 0x8000) != 0;
    bool resultSign = (subtraction & 0x8000) != 0;
    
    if (subtraction == 0)
        flags |= 0b1000;
    if (subtraction & 0x8000)
        flags |= 0b0100;
    if (arg1 < arg2)
        flags |= 0b0010;
    if ((src1Sign != src2Sign) && (resultSign != src1Sign)) 
        flags |= 0b0001;

    if (debug)
        std::cout << "Setting new flags:"
                  << " Z: " << ((flags & 0b1000) >> 3)
                  << " S: " << ((flags & 0b0100) >> 2)
                  << " C: " << ((flags & 0b0010) >> 1)
                  << " O: " << (flags & 0b0001) << std::endl;

    specialRegisters[FLAGS] = flags;
}

void ASTRISC_16::microCONTINUEIFFLAG(const int& param) {
    if (debug) {
        std::cout << "Checking value of the ";
        if (param == 8) std::cout << "ZERO";
        else if (param == 4) std::cout << "SIGNED";
        else if (param == 2) std::cout << "CARRY";
        else if (param == 1) std::cout << "OVERFLOW";
        else std::cout << "UNKNOWN";
        std::cout << " flag (" << (specialRegisters[FLAGS] & param) << ")" << std::endl;
    }

    if ((specialRegisters[FLAGS] & param) == 0) {
        skipRemainingMicroOps = true;
        return;
    }

    if (debug)
        std::cout << "Flag is active, running remaining micro ops." << std::endl;
}

void ASTRISC_16::microHALT(const int& param) {
    if (debug)
        std::cout << "Halting CPU" << std::endl;
    
    running = false;
}

