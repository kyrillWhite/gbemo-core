#include "core.h"

Core::Core(Bus* bus, Interrupts* _interrupts, bool skipBoot)
{
    registerFile = new RegisterFile(skipBoot);
    idu = new IDU();
    alu = new ALU(registerFile);
    controlUnit = new ControlUnit(bus, registerFile, idu, alu, _interrupts);
}

Core::~Core()
{
    delete registerFile;
    delete idu;
    delete alu;
    delete controlUnit;
}

void Core::tick()
{
    controlUnit->executeInstruction();
}
