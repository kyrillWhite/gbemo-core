#pragma once

#include "common.h"

/*
    [+] 8-bit load instructions
    [+] 16-bit load instructions
    [+] 8-bit arithmetic and logical instructions
    [+] 16-bit arithmetic instructions
    [+] rotate, shift, and bit operation instructions
    [+] control flow instructions
    [+] CB prefixed instructions
    [+-] miscellaneous instructions
*/

enum Opcode
{
    // 8-bit load instructions
    LoadRegister_Register,
    LoadRegister_Immediate,
    LoadRegister_IndirectHL,
    LoadFromRegister_IndirectHL,
    LoadFromImmediateData_IndirectHL,
    LoadAccumulator_IndirectBC,
    LoadAccumulator_IndirectDE,
    LoadFromAccumulator_IndirectBC,
    LoadFromAccumulator_IndirectDE,
    LoadAccumulator_Direct,
    LoadFromAccumulator_Direct,
    LoadAccumulator_Indirect0xFF00_C,
    LoadFromAccumulator_Indirect0xFF00_C,
    LoadAccumulator_Indirect0xFF00_n,
    LoadFromAccumulator_Indirect0xFF00_n,
    LoadAccumulator_IndirectHL_dec,
    LoadFromAccumulator_IndirectHL_dec,
    LoadAccumulator_IndirectHL_inc,
    LoadFromAccumulator_IndirectHL_inc,

    // 16-bit load instructions
    Load16bitRegisterOrRegisterPair,
    LoadFromStackPointer_Direct,
    LoadStackPointerFromHL,
    PushToStack,
    PopFromStack,
    LoadHLFromAdjustedStackPointer,

    // 8-bit arithmetic and logical instructions
    Add_Register,
    Add_IndirectHL,
    Add_Immediate,
    AddWithCarry_Register,
    AddWithCarry_IndirectHL,
    AddWithCarry_Immediate,
    Sub_Register,
    Sub_IndirectHL,
    Sub_Immediate,
    SubWithCarry_Register,
    SubWithCarry_IndirectHL,
    SubWithCarry_Immediate,
    Compare_Register,
    Compare_IndirectHL,
    Compare_Immediate,
    Increment_Register,
    Increment_IndirectHL,
    Decrement_Register,
    Decrement_IndirectHL,
    And_Register,
    And_IndirectHL,
    And_Immediate,
    Or_Register,
    Or_IndirectHL,
    Or_Immediate,
    Xor_Register,
    Xor_IndirectHL,
    Xor_Immediate,
    ComplementCarryFlag,
    SetCarryFlag,
    DecimalAdjustAccumulator,
    ComplementAccumulator,

    // 16-bit arithmetic instructions
    Increment16bitRegister,
    Decrement16bitRegister,
    Add_16bitRegister,
    AddToStackPointer_relative,

    // rotate, shift, and bit operation instructions
    RotateLeftCircular_Accumulator,
    RotateRightCircular_Accumulator,
    RotateLeft_Accumulator,
    RotateRight_Accumulator,

    // control flow instructions
    Jump,
    JumpToHL,
    Jump_Conditional,
    RelativeJump,
    RelativeJump_Conditional,
    CallFunction,
    CallFunction_Conditional,
    ReturnFromFunction,
    ReturnFromFunction_Conditional,
    ReturnFromInterruptHandler,
    Restart,

    CB,
    // CB prefixed instructions
    RotateLeftCircular_Register,
    RotateLeftCircular_IndirectHL,
    RotateRightCircular_Register,
    RotateRightCircular_IndirectHL,
    RotateLeft_Register,
    RotateLeft_IndirectHL,
    RotateRight_Register,
    RotateRight_IndirectHL,
    ShiftLeftArithmetic_Register,
    ShiftLeftArithmetic_IndirectHL,
    ShiftRightArithmetic_Register,
    ShiftRightArithmetic_IndirectHL,
    SwapNibbles_Register,
    SwapNibbles_IndirectHL,
    ShiftRightLogical_Register,
    ShiftRightLogical_IndirectHL,
    TestBit_Register,
    TestBit_IndirectHL,
    ResetBit_Register,
    ResetBit_IndirectHL,
    SetBit_Register,
    SetBit_IndirectHL,

    // miscellaneous instructions
    HaltSystemClock,
    StopSystemAndMainClocks,
    DisableInterrupts,
    EnableInterrupts,
    Nop,
};

#ifdef DEBUG
static const char *getOpcodeName(const Opcode value)
{
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
    switch (value)
    {
        PROCESS_VAL(LoadRegister_Register);
        PROCESS_VAL(LoadRegister_Immediate);
        PROCESS_VAL(LoadRegister_IndirectHL);
        PROCESS_VAL(LoadFromRegister_IndirectHL);
        PROCESS_VAL(LoadFromImmediateData_IndirectHL);
        PROCESS_VAL(LoadAccumulator_IndirectBC);
        PROCESS_VAL(LoadAccumulator_IndirectDE);
        PROCESS_VAL(LoadFromAccumulator_IndirectBC);
        PROCESS_VAL(LoadFromAccumulator_IndirectDE);
        PROCESS_VAL(LoadAccumulator_Direct);
        PROCESS_VAL(LoadFromAccumulator_Direct);
        PROCESS_VAL(LoadAccumulator_Indirect0xFF00_C);
        PROCESS_VAL(LoadFromAccumulator_Indirect0xFF00_C);
        PROCESS_VAL(LoadAccumulator_Indirect0xFF00_n);
        PROCESS_VAL(LoadFromAccumulator_Indirect0xFF00_n);
        PROCESS_VAL(LoadAccumulator_IndirectHL_dec);
        PROCESS_VAL(LoadFromAccumulator_IndirectHL_dec);
        PROCESS_VAL(LoadAccumulator_IndirectHL_inc);
        PROCESS_VAL(LoadFromAccumulator_IndirectHL_inc);
        PROCESS_VAL(Load16bitRegisterOrRegisterPair);
        PROCESS_VAL(LoadFromStackPointer_Direct);
        PROCESS_VAL(LoadStackPointerFromHL);
        PROCESS_VAL(PushToStack);
        PROCESS_VAL(PopFromStack);
        PROCESS_VAL(LoadHLFromAdjustedStackPointer);
        PROCESS_VAL(Add_Register);
        PROCESS_VAL(Add_IndirectHL);
        PROCESS_VAL(Add_Immediate);
        PROCESS_VAL(AddWithCarry_Register);
        PROCESS_VAL(AddWithCarry_IndirectHL);
        PROCESS_VAL(AddWithCarry_Immediate);
        PROCESS_VAL(Sub_Register);
        PROCESS_VAL(Sub_IndirectHL);
        PROCESS_VAL(Sub_Immediate);
        PROCESS_VAL(SubWithCarry_Register);
        PROCESS_VAL(SubWithCarry_IndirectHL);
        PROCESS_VAL(SubWithCarry_Immediate);
        PROCESS_VAL(Compare_Register);
        PROCESS_VAL(Compare_IndirectHL);
        PROCESS_VAL(Compare_Immediate);
        PROCESS_VAL(Increment_Register);
        PROCESS_VAL(Increment_IndirectHL);
        PROCESS_VAL(Decrement_Register);
        PROCESS_VAL(Decrement_IndirectHL);
        PROCESS_VAL(And_Register);
        PROCESS_VAL(And_IndirectHL);
        PROCESS_VAL(And_Immediate);
        PROCESS_VAL(Or_Register);
        PROCESS_VAL(Or_IndirectHL);
        PROCESS_VAL(Or_Immediate);
        PROCESS_VAL(Xor_Register);
        PROCESS_VAL(Xor_IndirectHL);
        PROCESS_VAL(Xor_Immediate);
        PROCESS_VAL(ComplementCarryFlag);
        PROCESS_VAL(SetCarryFlag);
        PROCESS_VAL(DecimalAdjustAccumulator);
        PROCESS_VAL(ComplementAccumulator);
        PROCESS_VAL(Increment16bitRegister);
        PROCESS_VAL(Decrement16bitRegister);
        PROCESS_VAL(Add_16bitRegister);
        PROCESS_VAL(AddToStackPointer_relative);
        PROCESS_VAL(RotateLeftCircular_Accumulator);
        PROCESS_VAL(RotateRightCircular_Accumulator);
        PROCESS_VAL(RotateLeft_Accumulator);
        PROCESS_VAL(RotateRight_Accumulator);
        PROCESS_VAL(Jump);
        PROCESS_VAL(JumpToHL);
        PROCESS_VAL(Jump_Conditional);
        PROCESS_VAL(RelativeJump);
        PROCESS_VAL(RelativeJump_Conditional);
        PROCESS_VAL(CallFunction);
        PROCESS_VAL(CallFunction_Conditional);
        PROCESS_VAL(ReturnFromFunction);
        PROCESS_VAL(ReturnFromFunction_Conditional);
        PROCESS_VAL(ReturnFromInterruptHandler);
        PROCESS_VAL(Restart);
        PROCESS_VAL(CB);
        PROCESS_VAL(RotateLeftCircular_Register);
        PROCESS_VAL(RotateLeftCircular_IndirectHL);
        PROCESS_VAL(RotateRightCircular_Register);
        PROCESS_VAL(RotateRightCircular_IndirectHL);
        PROCESS_VAL(RotateLeft_Register);
        PROCESS_VAL(RotateLeft_IndirectHL);
        PROCESS_VAL(RotateRight_Register);
        PROCESS_VAL(RotateRight_IndirectHL);
        PROCESS_VAL(ShiftLeftArithmetic_Register);
        PROCESS_VAL(ShiftLeftArithmetic_IndirectHL);
        PROCESS_VAL(ShiftRightArithmetic_Register);
        PROCESS_VAL(ShiftRightArithmetic_IndirectHL);
        PROCESS_VAL(SwapNibbles_Register);
        PROCESS_VAL(SwapNibbles_IndirectHL);
        PROCESS_VAL(ShiftRightLogical_Register);
        PROCESS_VAL(ShiftRightLogical_IndirectHL);
        PROCESS_VAL(TestBit_Register);
        PROCESS_VAL(TestBit_IndirectHL);
        PROCESS_VAL(ResetBit_Register);
        PROCESS_VAL(ResetBit_IndirectHL);
        PROCESS_VAL(SetBit_Register);
        PROCESS_VAL(SetBit_IndirectHL);
        PROCESS_VAL(HaltSystemClock);
        PROCESS_VAL(StopSystemAndMainClocks);
        PROCESS_VAL(DisableInterrupts);
        PROCESS_VAL(EnableInterrupts);
        PROCESS_VAL(Nop);
    default:
        return "Unknown";
    }
#undef PROCESS_VAL
}
#endif // DEBUG