#pragma once

#include "control_unit.h"

class Core
{
private:
    RegisterFile* registerFile;
    IDU* idu;
    ALU* alu;
    ControlUnit* controlUnit;

public:
    Core(Bus* bus, Interrupts* _interrupts, bool skipBoot);
    ~Core();

    void tick();
};

