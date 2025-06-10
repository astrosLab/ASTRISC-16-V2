#include "emu/emulator.h"

ASTRISC_16 emulator;

int main() {
    emulator.debug = true;
    emulator.hertz = 1500;
    emulator.startCpu();
    return 0;
}

