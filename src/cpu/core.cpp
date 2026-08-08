#include "cpu/core.h"

Core::Core(Bus *bus, Interrupts *_interrupts, bool skipBoot) : registerFile(skipBoot),
                                                               idu(),
                                                               alu(&registerFile),
                                                               controlUnit(bus, &registerFile, &idu, &alu, _interrupts)
{
}

void Core::tick()
{
    controlUnit.executeInstruction();
}
