#include "emu/emulator.h"

ASTRISC_16 emulator;

int main() {
    emulator.debug = true;
    emulator.startCpu();
    return 0;
}

