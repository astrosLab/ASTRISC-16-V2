#include "emu/emulator.h"

ASTRISC_16 emulator;

int main() {
    emulator.debug = false;
    emulator.hertz = 1500;
    emulator.haltNop = true;
    emulator.startCpu();
    return 0;
}

