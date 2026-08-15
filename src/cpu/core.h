#pragma once

#include "cpu/control_unit.h"

class Core
{
private:
    RegisterFile registerFile;
    IDU idu;
    ALU alu;
    ControlUnit controlUnit;

public:
    Core(Bus *bus, Interrupts *_interrupts, Peripherals *_peripherals, bool skipBoot);

    u8 step();
};
