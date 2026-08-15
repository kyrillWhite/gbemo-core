#pragma once

#include "cpu/opcodes.h"
#include "memory/bus.h"
#include "cpu/register_file.h"
#include "cpu/idu.h"
#include "cpu/alu.h"
#include "system/peripherals.h"

class ControlUnit
{
private:
    Bus *bus;
    RegisterFile *registerFile;
    IDU *idu;
    ALU *alu;
    Interrupts *interrupts;
    Peripherals *peripherals;

    u8 mCycles;

    bool halted;
    InterruptCommand interruptCommand;

    u8 Z;
    u8 W;
    bool Zsign;
    bool enablingIME;
    bool IME;

    u16 currentPC;

    u8 read(u16 address);
    void write(u16 address, u8 value);
    void idle();

    void incrementPC();
    void M1();

    u16 getWZ();
    void setWZ(u16 value);

    Opcode decodeStdOpcode(u8 opcode);
    Opcode decodeCbOpcode(u8 opcode);
    void executeStdInstruction();
    void executeCbInstruction();

    void handleInterrupt();
    void scrapInterrupts();

    void printInstruction(Opcode opcode);

public:
    ControlUnit(
        Bus *_bus,
        RegisterFile *_registerFile,
        IDU *_idu,
        ALU *_alu,
        Interrupts *_interrupts,
        Peripherals *_peripherals);

    u8 step();
};
