#pragma once

#include "opcodes.h"
#include "bus.h"
#include "register_file.h"
#include "idu.h"
#include "alu.h"

class ControlUnit
{
private:
    Bus *bus;
    RegisterFile *registerFile;
    IDU *idu;
    ALU *alu;
    Interrupts *interrupts;

    u8 instructionCycle;
    bool isCbInstruction;
    bool cCheck;
    bool halted;
    InterruptCommand interruptCommand;

    u8 Z;
    u8 W;
    bool Zsign;
    bool enablingIME;
    bool IME;

    u16 currentPC;

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

    void printInstruction(Opcode opcode, bool fixCB = false);

public:
    ControlUnit(
        Bus *_bus,
        RegisterFile *_registerFile,
        IDU *_idu,
        ALU *_alu,
        Interrupts *_interrupts);

    void executeInstruction();
};
