#pragma once

#include "register_file.h"

class ALU
{
private:
    RegisterFile *registerFile;

public:
    ALU(RegisterFile *_registerFile);

    void copyR8ToR8(u8 from, u8 to);
    void copyR16ToR16(u8 from, u8 to);
    void copyValToR8(u8 to, u8 value);
    void copyValToR16(u8 to, u16 value);

    u8 add(u8 v1, u8 v2, u8 &carryPerBit, bool carryFlag = false);
    u8 sub(u8 v1, u8 v2, u8 &carryPerBit, bool carryFlag = false);
    u8 _and(u8 v1, u8 v2);
    u8 _or(u8 v1, u8 v2);
    u8 _xor(u8 v1, u8 v2);
    u8 _not(u8 value);

    void decimalAdjust(u8 &a, bool &z, bool &n, bool &h, bool &c);
    void decimalAdjust(u8 &a, u8 &m_F);
    u8 rotateLeftCircular(u8 value, bool &carry);
    u8 rotateRightCircular(u8 value, bool &carry);
    u8 rotateLeft(u8 value, bool &carry);
    u8 rotateRight(u8 value, bool &carry);
    u8 shiftLeftA(u8 value, bool &carry);
    u8 shiftRightA(u8 value, bool &carry);
    u8 shiftRightL(u8 value, bool &carry);
    u8 swap(u8 value);
    bool bit(u8 value, u8 b);
    u8 reset(u8 value, u8 b);
    u8 set(u8 value, u8 b);
};
