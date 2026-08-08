#include "cpu/control_unit.h"
#include <cstdint>

void ControlUnit::incrementPC()
{
    registerFile->setPC(idu->increment(registerFile->getPC()));
}

void ControlUnit::M1()
{
    auto rf = registerFile;
    bus->setAddress(rf->getPC());
    rf->setIR(bus->readMemory());
    incrementPC();
    instructionCycle = 0;
    isCbInstruction = false;
}

void ControlUnit::handleInterrupt()
{
    auto rf = registerFile;
    if (instructionCycle == 0)
    {
        bus->setAddress(rf->getPC());
        rf->setPC(idu->decrement(rf->getPC()));
        instructionCycle++;
    }
    if (instructionCycle == 1)
    {
        bus->setAddress(rf->getSP());
        rf->setSP(idu->decrement(rf->getSP()));
        instructionCycle++;
    }
    else if (instructionCycle == 2)
    {
        bus->setAddress(rf->getSP());
        bus->writeMemory(rf->getPCH());
        rf->setSP(idu->decrement(rf->getSP()));
        instructionCycle++;
    }
    else if (instructionCycle == 3)
    {
        bus->setAddress(rf->getSP());
        bus->writeMemory(rf->getPCL());
        rf->setPC(interruptCommand.address);
        instructionCycle++;
    }
    else
    {
        M1();
        interruptCommand.isHandling = false;
        halted = false;
        interrupts->setIFflag(interruptCommand.type, false);
        IME = false;
    }
}

void ControlUnit::scrapInterrupts()
{
    if (interrupts->getIEflag(VBlank) && interrupts->getIFflag(VBlank))
    {
        interruptCommand = {VBlank, 0x40, true};
    }
    else if (interrupts->getIEflag(LCD_) && interrupts->getIFflag(LCD_))
    {
        interruptCommand = {LCD_, 0x48, true};
    }
    else if (interrupts->getIEflag(Timer_) && interrupts->getIFflag(Timer_))
    {
        interruptCommand = {Timer_, 0x50, true};
    }
    else if (interrupts->getIEflag(Serial) && interrupts->getIFflag(Serial))
    {
        interruptCommand = {Serial, 0x58, true};
    }
    else if (interrupts->getIEflag(Joypad_) && interrupts->getIFflag(Joypad_))
    {
        interruptCommand = {Joypad_, 0x60, true};
    }
}

void ControlUnit::printInstruction(Opcode opcode, bool fixCB)
{
#ifdef DEBUG
    if (!instructionCycle && globalTicks % (PRINT_SKIP + 1) == 0)
    {
        auto rf = registerFile;
        printf("Tick: %10I64u; %04X: %40s A: %02X F: %s%s%s%s BC: %04X DE: %04X HL: %04X\n",
               globalTicks,
               fixCB ? currentPC - 2 : currentPC - 1,
               getOpcodeName(opcode),
               rf->getA(),
               rf->getZflag() ? "Z" : "-",
               rf->getNflag() ? "N" : "-",
               rf->getHflag() ? "H" : "-",
               rf->getCflag() ? "C" : "-",
               rf->getBC(),
               rf->getDE(),
               rf->getHL());
        __asm nop
    }
#endif // DEBUG
}

ControlUnit::ControlUnit(
    Bus *_bus,
    RegisterFile *_registerFile,
    IDU *_idu,
    ALU *_alu,
    Interrupts *_interrupts) : bus(_bus), registerFile(_registerFile), idu(_idu), alu(_alu), interrupts(_interrupts),
                               instructionCycle(0), Z(0), W(0), Zsign(true), cCheck(false), currentPC(0),
                               isCbInstruction(false), IME(false), halted(false), enablingIME(false),
                               interruptCommand({InterruptType::VBlank, 0x40, false})
{
}

Opcode ControlUnit::decodeStdOpcode(u8 opcode)
{
    switch (opcode)
    {
    case 0b00110110:
        return Opcode::LoadFromImmediateData_IndirectHL;
    case 0b00001010:
        return Opcode::LoadAccumulator_IndirectBC;
    case 0b00011010:
        return Opcode::LoadAccumulator_IndirectDE;
    case 0b00000010:
        return Opcode::LoadFromAccumulator_IndirectBC;
    case 0b00010010:
        return Opcode::LoadFromAccumulator_IndirectDE;
    case 0b11111010:
        return Opcode::LoadAccumulator_Direct;
    case 0b11101010:
        return Opcode::LoadFromAccumulator_Direct;
    case 0b11110010:
        return Opcode::LoadAccumulator_Indirect0xFF00_C;
    case 0b11100010:
        return Opcode::LoadFromAccumulator_Indirect0xFF00_C;
    case 0b11110000:
        return Opcode::LoadAccumulator_Indirect0xFF00_n;
    case 0b11100000:
        return Opcode::LoadFromAccumulator_Indirect0xFF00_n;
    case 0b00111010:
        return Opcode::LoadAccumulator_IndirectHL_dec;
    case 0b00110010:
        return Opcode::LoadFromAccumulator_IndirectHL_dec;
    case 0b00101010:
        return Opcode::LoadAccumulator_IndirectHL_inc;
    case 0b00100010:
        return Opcode::LoadFromAccumulator_IndirectHL_inc;
    case 0b00001000:
        return Opcode::LoadFromStackPointer_Direct;
    case 0b11111001:
        return Opcode::LoadStackPointerFromHL;
    case 0b11111000:
        return Opcode::LoadHLFromAdjustedStackPointer;
    case 0b10000110:
        return Opcode::Add_IndirectHL;
    case 0b11000110:
        return Opcode::Add_Immediate;
    case 0b10001110:
        return Opcode::AddWithCarry_IndirectHL;
    case 0b11001110:
        return Opcode::AddWithCarry_Immediate;
    case 0b10010110:
        return Opcode::Sub_IndirectHL;
    case 0b11010110:
        return Opcode::Sub_Immediate;
    case 0b10011110:
        return Opcode::SubWithCarry_IndirectHL;
    case 0b11011110:
        return Opcode::SubWithCarry_Immediate;
    case 0b10111110:
        return Opcode::Compare_IndirectHL;
    case 0b11111110:
        return Opcode::Compare_Immediate;
    case 0b00110100:
        return Opcode::Increment_IndirectHL;
    case 0b00110101:
        return Opcode::Decrement_IndirectHL;
    case 0b10100110:
        return Opcode::And_IndirectHL;
    case 0b11100110:
        return Opcode::And_Immediate;
    case 0b10110110:
        return Opcode::Or_IndirectHL;
    case 0b11110110:
        return Opcode::Or_Immediate;
    case 0b10101110:
        return Opcode::Xor_IndirectHL;
    case 0b11101110:
        return Opcode::Xor_Immediate;
    case 0b00111111:
        return Opcode::ComplementCarryFlag;
    case 0b00110111:
        return Opcode::SetCarryFlag;
    case 0b00100111:
        return Opcode::DecimalAdjustAccumulator;
    case 0b00101111:
        return Opcode::ComplementAccumulator;
    case 0b11101000:
        return Opcode::AddToStackPointer_relative;
    case 0b11000011:
        return Opcode::Jump;
    case 0b11101001:
        return Opcode::JumpToHL;
    case 0b00011000:
        return Opcode::RelativeJump;
    case 0b11001101:
        return Opcode::CallFunction;
    case 0b11001011:
        return Opcode::CB;
    case 0b11001001:
        return Opcode::ReturnFromFunction;
    case 0b11011001:
        return Opcode::ReturnFromInterruptHandler;
    case 0b00000111:
        return Opcode::RotateLeftCircular_Accumulator;
    case 0b00001111:
        return Opcode::RotateRightCircular_Accumulator;
    case 0b00010111:
        return Opcode::RotateLeft_Accumulator;
    case 0b00011111:
        return Opcode::RotateRight_Accumulator;
    case 0b01110110:
        return Opcode::HaltSystemClock;
    case 0b00010000:
        return Opcode::StopSystemAndMainClocks;
    case 0b11110011:
        return Opcode::DisableInterrupts;
    case 0b11111011:
        return Opcode::EnableInterrupts;
    case 0b00000000:
        return Opcode::Nop;

    default:
        if ((opcode & 0b11000111) == 0b00000110)
        {
            return Opcode::LoadRegister_Immediate;
        }
        else if ((opcode & 0b11000111) == 0b01000110)
        {
            return Opcode::LoadRegister_IndirectHL;
        }
        else if ((opcode & 0b11111000) == 0b01110000)
        {
            return Opcode::LoadFromRegister_IndirectHL;
        }
        else if ((opcode & 0b11000000) == 0b01000000)
        { // must be after HL versions opcodes
            return Opcode::LoadRegister_Register;
        }
        else if ((opcode & 0b11001111) == 0b00000001)
        {
            return Opcode::Load16bitRegisterOrRegisterPair;
        }
        else if ((opcode & 0b11001111) == 0b11000101)
        {
            return Opcode::PushToStack;
        }
        else if ((opcode & 0b11001111) == 0b11000001)
        {
            return Opcode::PopFromStack;
        }
        else if ((opcode & 0b11111000) == 0b10000000)
        {
            return Opcode::Add_Register;
        }
        else if ((opcode & 0b11111000) == 0b10001000)
        {
            return Opcode::AddWithCarry_Register;
        }
        else if ((opcode & 0b11111000) == 0b10010000)
        {
            return Opcode::Sub_Register;
        }
        else if ((opcode & 0b11111000) == 0b10011000)
        {
            return Opcode::SubWithCarry_Register;
        }
        else if ((opcode & 0b11111000) == 0b10111000)
        {
            return Opcode::Compare_Register;
        }
        else if ((opcode & 0b11000111) == 0b00000100)
        {
            return Opcode::Increment_Register;
        }
        else if ((opcode & 0b11000111) == 0b00000101)
        {
            return Opcode::Decrement_Register;
        }
        else if ((opcode & 0b11111000) == 0b10100000)
        {
            return Opcode::And_Register;
        }
        else if ((opcode & 0b11111000) == 0b10110000)
        {
            return Opcode::Or_Register;
        }
        else if ((opcode & 0b11111000) == 0b10101000)
        {
            return Opcode::Xor_Register;
        }
        else if ((opcode & 0b11001111) == 0b00000011)
        {
            return Opcode::Increment16bitRegister;
        }
        else if ((opcode & 0b11001111) == 0b00001011)
        {
            return Opcode::Decrement16bitRegister;
        }
        else if ((opcode & 0b11001111) == 0b00001001)
        {
            return Opcode::Add_16bitRegister;
        }
        else if ((opcode & 0b11100111) == 0b11000010)
        {
            return Opcode::Jump_Conditional;
        }
        else if ((opcode & 0b11100111) == 0b00100000)
        {
            return Opcode::RelativeJump_Conditional;
        }
        else if ((opcode & 0b11100111) == 0b11000100)
        {
            return Opcode::CallFunction_Conditional;
        }
        else if ((opcode & 0b11100111) == 0b11000000)
        {
            return Opcode::ReturnFromFunction_Conditional;
        }
        else if ((opcode & 0b11000111) == 0b11000111)
        {
            return Opcode::Restart;
        }
        else
        {
            return Nop;
        }
    }
}

Opcode ControlUnit::decodeCbOpcode(u8 opcode)
{
    switch (opcode)
    {
    case 0b00000110:
        return Opcode::RotateLeftCircular_IndirectHL;
    case 0b00001110:
        return Opcode::RotateRightCircular_IndirectHL;
    case 0b00010110:
        return Opcode::RotateLeft_IndirectHL;
    case 0b00011110:
        return Opcode::RotateRight_IndirectHL;
    case 0b00100110:
        return Opcode::ShiftLeftArithmetic_IndirectHL;
    case 0b00101110:
        return Opcode::ShiftRightArithmetic_IndirectHL;
    case 0b00110110:
        return Opcode::SwapNibbles_IndirectHL;
    case 0b00111110:
        return Opcode::ShiftRightLogical_IndirectHL;

    default:
        if ((opcode & 0b11111000) == 0b00000000)
        {
            return Opcode::RotateLeftCircular_Register;
        }
        else if ((opcode & 0b11111000) == 0b00001000)
        {
            return Opcode::RotateRightCircular_Register;
        }
        else if ((opcode & 0b11111000) == 0b00010000)
        {
            return Opcode::RotateLeft_Register;
        }
        else if ((opcode & 0b11111000) == 0b00011000)
        {
            return Opcode::RotateRight_Register;
        }
        else if ((opcode & 0b11111000) == 0b00100000)
        {
            return Opcode::ShiftLeftArithmetic_Register;
        }
        else if ((opcode & 0b11111000) == 0b00101000)
        {
            return Opcode::ShiftRightArithmetic_Register;
        }
        else if ((opcode & 0b11111000) == 0b00110000)
        {
            return Opcode::SwapNibbles_Register;
        }
        else if ((opcode & 0b11111000) == 0b00111000)
        {
            return Opcode::ShiftRightLogical_Register;
        }
        else if ((opcode & 0b11000111) == 0b01000110)
        {
            return Opcode::TestBit_IndirectHL;
        }
        else if ((opcode & 0b11000000) == 0b01000000)
        {
            return Opcode::TestBit_Register;
        }
        else if ((opcode & 0b11000111) == 0b10000110)
        {
            return Opcode::ResetBit_IndirectHL;
        }
        else if ((opcode & 0b11000000) == 0b10000000)
        {
            return Opcode::ResetBit_Register;
        }
        else if ((opcode & 0b11000111) == 0b11000110)
        {
            return Opcode::SetBit_IndirectHL;
        }
        else if ((opcode & 0b11000000) == 0b11000000)
        {
            return Opcode::SetBit_Register;
        }
        else
        {
            throw std::invalid_argument("Unknown opcode");
        }
    }
}

void ControlUnit::executeInstruction()
{
#ifdef DEBUG
    if (registerFile->getPC() == 0xc2fc)
    {
        __asm nop
    }
#endif // DEBUG
    if (instructionCycle == 0)
    {
        if (IME)
        {
            scrapInterrupts();
        }
        if (enablingIME)
        {
            IME = true;
            enablingIME = false;
        }
    }
    if (halted && interrupts->getIF())
    {
        halted = false; // if IME is disable has strange behavior
    }
    if (!interruptCommand.isHandling)
    {
        if (!halted)
        {
            if (instructionCycle == 0)
            {
                currentPC = registerFile->getPC();
            }
            if (isCbInstruction)
            {
                executeCbInstruction();
            }
            else
            {
                executeStdInstruction();
            }
        }
    }
    else
    {
        handleInterrupt();
    }
}

void ControlUnit::executeStdInstruction()
{
    auto rf = registerFile;
    auto opcode = rf->getIR();
    auto opcodeType = decodeStdOpcode(opcode);

#ifdef DEBUG
    if (globalTicks > DEBUG_START && opcodeType != Opcode::CB)
    {
        printInstruction(opcodeType);
    }
#endif // DEBUG

    switch (opcodeType)
    {
    case Opcode::LoadRegister_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 toR8 = opcode >> 3 & 0b00000111;
        alu->copyR8ToR8(fromR8, toR8);
        break;
    }
    case Opcode::LoadRegister_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 toR8 = opcode >> 3 & 0b00000111;
            alu->copyValToR8(toR8, Z);
        }
        break;
    }
    case Opcode::LoadRegister_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 toR8 = opcode >> 3 & 0b00000111;
            alu->copyValToR8(toR8, Z);
        }
        break;
    }
    case Opcode::LoadFromRegister_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            u8 codeR8 = opcode & 0b00000111;
            u8 r8 = rf->getR8(codeR8);
            bus->writeMemory(r8);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadFromImmediateData_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bus->writeMemory(Z);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_IndirectBC:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getBC());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadAccumulator_IndirectDE:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getDE());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_IndirectBC:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getBC());
            bus->writeMemory(rf->getA());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadFromAccumulator_IndirectDE:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getDE());
            bus->writeMemory(rf->getA());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_Direct:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(getWZ());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_Direct:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(getWZ());
            bus->writeMemory(rf->getA());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_Indirect0xFF00_C:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(0xFF00 + rf->getC());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_Indirect0xFF00_C:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(0xFF00 + rf->getC());
            bus->writeMemory(rf->getA());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_Indirect0xFF00_n:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(0xFF00 + Z);
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_Indirect0xFF00_n:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(0xFF00 + Z);
            bus->writeMemory(rf->getA());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_IndirectHL_dec:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            rf->setHL(idu->decrement(rf->getHL()));
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_IndirectHL_dec:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            bus->writeMemory(rf->getA());
            rf->setHL(idu->decrement(rf->getHL()));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadAccumulator_IndirectHL_inc:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            rf->setHL(idu->increment(rf->getHL()));
            instructionCycle++;
        }
        else
        {
            M1();
            alu->copyValToR8(0b00000111, Z); // load to A register
        }
        break;
    }
    case Opcode::LoadFromAccumulator_IndirectHL_inc:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            bus->writeMemory(rf->getA());
            rf->setHL(idu->increment(rf->getHL()));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::Load16bitRegisterOrRegisterPair:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 toR16 = opcode >> 4 & 0b00000011;
            u16 wz = getWZ();
            alu->copyValToR16(toR16, wz);
        }
        break;
    }
    case Opcode::LoadFromStackPointer_Direct:
    {
        // five cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(getWZ());
            bus->writeMemory(rf->getSPL());
            setWZ(idu->increment(getWZ()));
            instructionCycle++;
        }
        else if (instructionCycle == 3)
        {
            bus->setAddress(getWZ());
            bus->writeMemory(rf->getSPH());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::LoadStackPointerFromHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            alu->copyR16ToR16(2, 3); // HL to SP
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::PushToStack:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getSP());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getSP());
            u8 fromR16 = opcode >> 4 & 0b00000011;
            u8 R16M = rf->getR16Mstk(fromR16);
            bus->writeMemory(R16M);
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(rf->getSP());
            u8 fromR16 = opcode >> 4 & 0b00000011;
            u8 R16L = rf->getR16Lstk(fromR16);
            bus->writeMemory(R16L);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::PopFromStack:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getSP());
            Z = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getSP());
            W = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else
        {
            M1();
            u8 toR16 = opcode >> 4 & 0b00000011;
            rf->setR16stk(toR16, getWZ());
        }
        break;
    }
    case Opcode::LoadHLFromAdjustedStackPointer:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(0x0000);
            u8 carryPerBit = 0;
            rf->setL(alu->add(rf->getSPL(), Z, carryPerBit));
            u8 Hflag = (carryPerBit & 0b00001000) << 2;
            u8 Cflag = (carryPerBit & 0b10000000) >> 3;
            rf->setF(Hflag | Cflag);
            Zsign = bit(Z, 7);
            instructionCycle++;
        }
        else
        {
            M1();
            u8 adj = Zsign ? 0xFF : 0x00;
            u8 carryPerBit = 0;
            rf->setH(alu->add(rf->getSPH(), adj, carryPerBit, rf->getCflag()));
        }
        break;
    }
    case Opcode::Add_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->add(rf->getA(), rf->getR8(fromR8), carryPerBit);
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setCflag(bit(carryPerBit, 7));
        break;
    }
    case Opcode::Add_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->add(rf->getA(), Z, carryPerBit);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Add_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->add(rf->getA(), Z, carryPerBit);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::AddWithCarry_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->add(rf->getA(), rf->getR8(fromR8), carryPerBit, rf->getCflag());
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setCflag(bit(carryPerBit, 7));
        break;
    }
    case Opcode::AddWithCarry_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->add(rf->getA(), Z, carryPerBit, rf->getCflag());
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::AddWithCarry_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->add(rf->getA(), Z, carryPerBit, rf->getCflag());
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Sub_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->sub(rf->getA(), rf->getR8(fromR8), carryPerBit);
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(true);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setCflag(bit(carryPerBit, 7));
        break;
    }
    case Opcode::Sub_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Sub_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::SubWithCarry_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->sub(rf->getA(), rf->getR8(fromR8), carryPerBit, rf->getCflag());
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(true);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setCflag(bit(carryPerBit, 7));
        break;
    }
    case Opcode::SubWithCarry_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit, rf->getCflag());
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::SubWithCarry_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit, rf->getCflag());
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Compare_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->sub(rf->getA(), rf->getR8(fromR8), carryPerBit);
        rf->setZflag(result == 0);
        rf->setNflag(true);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setCflag(bit(carryPerBit, 7));
        break;
    }
    case Opcode::Compare_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Compare_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 carryPerBit;
            u8 result = alu->sub(rf->getA(), Z, carryPerBit);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::Increment_Register:
    {
        // one cycle
        M1();
        u8 toR8 = opcode >> 3 & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->add(rf->getR8(toR8), 1, carryPerBit);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setR8(toR8, result);
        break;
    }
    case Opcode::Increment_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            u8 carryPerBit;
            u8 result = alu->add(Z, 1, carryPerBit);
            bus->writeMemory(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::Decrement_Register:
    {
        // one cycle
        M1();
        u8 toR8 = opcode >> 3 & 0b00000111;
        u8 carryPerBit;
        u8 result = alu->sub(rf->getR8(toR8), 1, carryPerBit);
        rf->setZflag(result == 0);
        rf->setNflag(true);
        rf->setHflag(bit(carryPerBit, 3));
        rf->setR8(toR8, result);
        break;
    }
    case Opcode::Decrement_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            u8 carryPerBit;
            u8 result = alu->sub(Z, 1, carryPerBit);
            bus->writeMemory(result);
            rf->setZflag(result == 0);
            rf->setNflag(true);
            rf->setHflag(bit(carryPerBit, 3));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::And_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 result = alu->_and(rf->getA(), rf->getR8(fromR8));
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(true);
        rf->setCflag(false);
        break;
    }
    case Opcode::And_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_and(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(true);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::And_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_and(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(true);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::Or_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 result = alu->_or(rf->getA(), rf->getR8(fromR8));
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(false);
        break;
    }
    case Opcode::Or_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_or(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::Or_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_or(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::Xor_Register:
    {
        // one cycle
        M1();
        u8 fromR8 = opcode & 0b00000111;
        u8 result = alu->_xor(rf->getA(), rf->getR8(fromR8));
        rf->setA(result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(false);
        break;
    }
    case Opcode::Xor_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_xor(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::Xor_Immediate:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 result = alu->_xor(rf->getA(), Z);
            rf->setA(result);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(false);
        }
        break;
    }
    case Opcode::ComplementCarryFlag:
    {
        M1();
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(!rf->getCflag());
        break;
    }
    case Opcode::SetCarryFlag:
    {
        M1();
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(true);
        break;
    }
    case Opcode::DecimalAdjustAccumulator:
    {
        M1();
        u8 A = rf->getA();
        u8 F = rf->getF();
        alu->decimalAdjust(A, F);
        rf->setA(A);
        rf->setF(F);

        break;
    }
    case Opcode::ComplementAccumulator:
    {
        M1();
        rf->setA(alu->_not(rf->getA()));
        rf->setNflag(true);
        rf->setHflag(true);
        break;
    }
    case Opcode::Increment16bitRegister:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            u8 toR16 = opcode >> 4 & 0b00000011;
            rf->setR16(toR16, idu->increment(rf->getR16(toR16)));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::Decrement16bitRegister:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            u8 toR16 = opcode >> 4 & 0b00000011;
            rf->setR16(toR16, idu->decrement(rf->getR16(toR16)));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::Add_16bitRegister:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(0);
            u8 R16code = opcode >> 4 & 0b00000011;
            u8 lsb = rf->getR16L(R16code);
            u8 carryPerBit;
            rf->setL(alu->add(rf->getL(), lsb, carryPerBit));
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
            instructionCycle++;
        }
        else
        {
            M1();
            u8 R16code = opcode >> 4 & 0b00000011;
            u8 hsb = rf->getR16M(R16code);
            u8 carryPerBit;
            rf->setH(alu->add(rf->getH(), hsb, carryPerBit, rf->getCflag()));
            rf->setNflag(false);
            rf->setHflag(bit(carryPerBit, 3));
            rf->setCflag(bit(carryPerBit, 7));
        }
        break;
    }
    case Opcode::AddToStackPointer_relative:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            // bus->setAddress(0);
            // u8 carryPerBit;
            // Z = alu->add(rf->getSPL(), Z, carryPerBit);
            // Zsign = bit(7, Z);
            // rf->setZflag(false);
            // rf->setNflag(false);
            // rf->setHflag(bit(carryPerBit, 3));
            // rf->setCflag(bit(carryPerBit, 7));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            // bus->setAddress(0);
            // u8 adj = Zsign ? 0xFF : 0x00;
            // u8 carryPerBit;
            // W = alu->add(rf->getSPH(), adj, carryPerBit, rf->getCflag());
            instructionCycle++;
        }
        else
        {
            M1();
            int8_t offset = static_cast<int8_t>(Z);
            u16 sp = rf->getSP();
            u16 result = sp + offset;
            bool halfCarry = (((sp & 0xF) + (offset & 0xF)) > 0xF);
            bool fullCarry = (((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
            rf->setHflag(halfCarry);
            rf->setCflag(fullCarry);
            rf->setZflag(false);
            rf->setNflag(false);
            rf->setSP(result);
            instructionCycle = 0;
        }
        break;
    }
    case Opcode::RotateLeftCircular_Accumulator:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        rf->setA(alu->rotateLeftCircular(rf->getA(), carry));
        rf->setZflag(false);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateRightCircular_Accumulator:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        rf->setA(alu->rotateRightCircular(rf->getA(), carry));
        rf->setZflag(false);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateLeft_Accumulator:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        rf->setA(alu->rotateLeft(rf->getA(), carry));
        rf->setZflag(false);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateRight_Accumulator:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        rf->setA(alu->rotateRight(rf->getA(), carry));
        rf->setZflag(false);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::Jump:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(0);
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::JumpToHL:
    {
        // one cycle
        bus->setAddress(rf->getHL());
        rf->setIR(bus->readMemory());
        rf->setPC(idu->increment(rf->getHL()));
        break;
    }
    case Opcode::Jump_Conditional:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            cCheck = rf->checkConditional(opcode >> 3 & 0b00000011);
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            if (!cCheck)
            {
                M1();
                break;
            }
            bus->setAddress(0);
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::RelativeJump:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            /*bool Z_sign = bit(Z, 7);
            u8 carryPerBit;
            Z = alu->add(Z, rf->getPCL(), carryPerBit);
            char adj = 0;
            if (bit(carryPerBit, 7) && !Z_sign) {
                adj = 1;
            }
            else if (!bit(carryPerBit, 7) && Z_sign) {
                adj = -1;
            }
            W = rf->getPCH() + adj;*/

            setWZ(rf->getPC() + static_cast<int8_t>(Z));
            instructionCycle++;
        }
        else
        {
            bus->setAddress(getWZ());
            rf->setIR(bus->readMemory());
            rf->setPC(idu->increment(getWZ()));
            instructionCycle = 0;
        }
        break;
    }
    case Opcode::RelativeJump_Conditional:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            cCheck = rf->checkConditional(opcode >> 3 & 0b00000011);
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            if (!cCheck)
            {
                M1();
                break;
            }
            /*bus->setAddress(rf->getPCH());
            bool Z_sign = bit(Z, 7);
            u8 carryPerBit;
            Z = alu->add(Z, rf->getPCL(), carryPerBit);
            char adj = 0;
            if (bit(carryPerBit, 7) && !Z_sign) {
                adj = 1;
            }
            else if (!bit(carryPerBit, 7) && Z_sign) {
                adj = -1;
            }
            W = rf->getPCH() + adj;*/
            setWZ(rf->getPC() + static_cast<int8_t>(Z));
            instructionCycle++;
        }
        else
        {
            bus->setAddress(getWZ());
            rf->setIR(bus->readMemory());
            rf->setPC(idu->increment(getWZ()));
            instructionCycle = 0;
        }
        break;
    }
    case Opcode::CallFunction:
    {
        // six cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(rf->getSP());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 3)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCH());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 4)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCL());
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::CallFunction_Conditional:
    {
        // six cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getPC());
            Z = bus->readMemory();
            incrementPC();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getPC());
            W = bus->readMemory();
            incrementPC();
            cCheck = rf->checkConditional(opcode >> 3 & 0b00000011);
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            if (!cCheck)
            {
                M1();
                break;
            }
            bus->setAddress(rf->getSP());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 3)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCH());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 4)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCL());
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ReturnFromFunction:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getSP());
            Z = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getSP());
            W = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(0);
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ReturnFromFunction_Conditional:
    {
        // five cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(0);
            cCheck = rf->checkConditional(opcode >> 3 & 0b00000011);
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            if (!cCheck)
            {
                M1();
                break;
            }
            bus->setAddress(rf->getSP());
            Z = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(rf->getSP());
            W = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 3)
        {
            bus->setAddress(0);
            rf->setPC(getWZ());
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ReturnFromInterruptHandler:
    {
        // four cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getSP());
            Z = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getSP());
            W = bus->readMemory();
            rf->setSP(idu->increment(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(0);
            rf->setPC(getWZ());
            IME = true;
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::Restart:
    {
        // five cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getSP());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCH());
            rf->setSP(idu->decrement(rf->getSP()));
            instructionCycle++;
        }
        else if (instructionCycle == 2)
        {
            bus->setAddress(rf->getSP());
            bus->writeMemory(rf->getPCL());
            u16 address = opcode & 0b00111000; // 0b11xxx111, xxx - is address divided by 8
            rf->setPC(address);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::HaltSystemClock:
    {
        halted = true;
        M1();
        break;
    }
    case Opcode::StopSystemAndMainClocks:
    {
        // one cycle
        M1();
        break;
    }
    case Opcode::Nop:
    {
        // one cycle
        M1();
        break;
    }
    case Opcode::EnableInterrupts:
    {
        // one cycle
        M1();
        enablingIME = true;
        break;
    }
    case Opcode::DisableInterrupts:
    {
        // one cycle
        M1();
        IME = false;
        break;
    }
    case Opcode::CB:
    {
        M1();
        isCbInstruction = true;
        break;
    }

    default:
        throw std::invalid_argument("Unknown opcode");
    }
}

void ControlUnit::executeCbInstruction()
{
    auto rf = registerFile;
    auto opcode = rf->getIR();
    auto opcodeType = decodeCbOpcode(opcode);

#ifdef DEBUG
    if (globalTicks > DEBUG_START)
    {
        printInstruction(opcodeType, true);
    }
#endif // DEBUG

    switch (opcodeType)
    {
    case Opcode::RotateLeftCircular_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->rotateLeftCircular(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateLeftCircular_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->rotateLeftCircular(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::RotateRightCircular_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->rotateRightCircular(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateRightCircular_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->rotateRightCircular(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::RotateLeft_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->rotateLeft(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateLeft_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->rotateLeft(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::RotateRight_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->rotateRight(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::RotateRight_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->rotateRight(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ShiftLeftArithmetic_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->shiftLeftA(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::ShiftLeftArithmetic_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->shiftLeftA(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ShiftRightArithmetic_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->shiftRightA(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::ShiftRightArithmetic_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->shiftRightA(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::SwapNibbles_Register:
    {
        // one cycle
        M1();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->swap(rf->getR8(R8code));
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(false);
        break;
    }
    case Opcode::SwapNibbles_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            u8 result = alu->swap(Z);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(false);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::ShiftRightLogical_Register:
    {
        // one cycle
        M1();
        bool carry = rf->getCflag();
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->shiftRightL(rf->getR8(R8code), carry);
        rf->setR8(R8code, result);
        rf->setZflag(result == 0);
        rf->setNflag(false);
        rf->setHflag(false);
        rf->setCflag(carry);
        break;
    }
    case Opcode::ShiftRightLogical_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            bool carry = rf->getCflag();
            u8 result = alu->shiftRightL(Z, carry);
            rf->setZflag(result == 0);
            rf->setNflag(false);
            rf->setHflag(false);
            rf->setCflag(carry);
            bus->writeMemory(result);
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::TestBit_Register:
    {
        // one cycle
        M1();
        u8 b = opcode >> 3 & 0b00000111;
        u8 R8code = opcode & 0b00000111;
        u8 result = alu->bit(rf->getR8(R8code), b);
        rf->setZflag(!result);
        rf->setNflag(false);
        rf->setHflag(true);
        break;
    }
    case Opcode::TestBit_IndirectHL:
    {
        // two cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else
        {
            M1();
            u8 b = opcode >> 3 & 0b00000111;
            u8 result = alu->bit(Z, b);
            rf->setZflag(!result);
            rf->setNflag(false);
            rf->setHflag(true);
        }
        break;
    }
    case Opcode::ResetBit_Register:
    {
        // one cycle
        M1();
        u8 b = opcode >> 3 & 0b00000111;
        u8 R8code = opcode & 0b00000111;
        rf->setR8(R8code, alu->reset(rf->getR8(R8code), b));
        break;
    }
    case Opcode::ResetBit_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            u8 b = opcode >> 3 & 0b00000111;
            bus->writeMemory(alu->reset(Z, b));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }
    case Opcode::SetBit_Register:
    {
        // one cycle
        M1();
        u8 b = opcode >> 3 & 0b00000111;
        u8 R8code = opcode & 0b00000111;
        rf->setR8(R8code, alu->set(rf->getR8(R8code), b));
        break;
    }
    case Opcode::SetBit_IndirectHL:
    {
        // three cycle
        if (instructionCycle == 0)
        {
            bus->setAddress(rf->getHL());
            Z = bus->readMemory();
            instructionCycle++;
        }
        else if (instructionCycle == 1)
        {
            bus->setAddress(rf->getHL());
            u8 b = opcode >> 3 & 0b00000111;
            bus->writeMemory(alu->set(Z, b));
            instructionCycle++;
        }
        else
        {
            M1();
        }
        break;
    }

    default:
        throw std::invalid_argument("Unknown opcode");
    }
}

u16 ControlUnit::getWZ()
{
    return u16(W) << 8 | Z;
}

void ControlUnit::setWZ(u16 value)
{
    W = value >> 8;
    Z = value & 0xFF;
}
