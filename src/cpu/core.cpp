#include "cpu/core.h"

Core::Core(Bus *bus, Interrupts *_interrupts, Peripherals *_peripherals, bool skipBoot) : registerFile(skipBoot),
                                                                                          idu(),
                                                                                          alu(&registerFile),
                                                                                          controlUnit(bus, &registerFile, &idu, &alu, _interrupts, _peripherals)
{
}

u8 Core::step()
{
    return controlUnit.step();
}
