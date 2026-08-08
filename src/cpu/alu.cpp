#include "cpu/alu.h"

ALU::ALU(RegisterFile *_registerFile) : registerFile(_registerFile)
{
}

void ALU::copyR8ToR8(u8 from, u8 to)
{
    registerFile->setR8(to, registerFile->getR8(from));
}

void ALU::copyR16ToR16(u8 from, u8 to)
{
    registerFile->setR16(to, registerFile->getR16(from));
}

void ALU::copyValToR8(u8 to, u8 value)
{
    registerFile->setR8(to, value);
}

void ALU::copyValToR16(u8 to, u16 value)
{
    registerFile->setR16(to, value);
}

u8 ALU::add(u8 v1, u8 v2, u8 &carryPerBit, bool carryFlag)
{
    unsigned int carry_in = carryFlag ? 1 : 0;
    unsigned int result = v1 + v2 + carry_in;

    bool halfCarry = (((v1 & 0xF) + (v2 & 0xF) + carry_in) > 0xF);
    bool fullCarry = (result > 0xFF);

    carryPerBit = 0;
    if (halfCarry)
        carryPerBit |= (1 << 3);
    if (fullCarry)
        carryPerBit |= (1 << 7);

    return static_cast<u8>(result & 0xFF);
}

u8 ALU::sub(u8 v1, u8 v2, u8 &carryPerBit, bool carryFlag)
{
    unsigned int carry_in = carryFlag ? 1 : 0;
    int diff = static_cast<int>(v1) - static_cast<int>(v2) - carry_in;

    bool halfCarry = ((v1 & 0xF) < ((v2 & 0xF) + carry_in));
    bool fullCarry = (v1 < (v2 + carry_in));

    carryPerBit = 0;
    if (halfCarry)
        carryPerBit |= (1 << 3);
    if (fullCarry)
        carryPerBit |= (1 << 7);

    return static_cast<u8>(diff & 0xFF);
}

u8 ALU::_and(u8 v1, u8 v2)
{
    return v1 & v2;
}

u8 ALU::_or(u8 v1, u8 v2)
{
    return v1 | v2;
}

u8 ALU::_xor(u8 v1, u8 v2)
{
    return v1 ^ v2;
}

u8 ALU::_not(u8 value)
{
    return ~value;
}

void ALU::decimalAdjust(u8 &a, bool &z, bool &n, bool &h, bool &c)
{
    int tmp = a;
    u8 FLAG_Z = z << 7;
    u8 FLAG_N = n << 6;
    u8 FLAG_H = h << 5;
    u8 FLAG_C = c << 4;
    u8 m_F = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    if (!(m_F & FLAG_N))
    {
        if ((m_F & FLAG_H) || (tmp & 0x0F) > 9)
            tmp += 6;
        if ((m_F & FLAG_C) || tmp > 0x9F)
            tmp += 0x60;
    }
    else
    {
        if (m_F & FLAG_H)
        {
            tmp -= 6;
            if (!(m_F & FLAG_C))
                tmp &= 0xFF;
        }
        if (m_F & FLAG_C)
            tmp -= 0x60;
    }
    m_F &= ~(FLAG_H | FLAG_Z);
    if (tmp & 0x100)
        m_F |= FLAG_C;
    a = tmp & 0xFF;
    if (!a)
        m_F |= FLAG_Z;
}

void ALU::decimalAdjust(u8 &a, u8 &m_F)
{
    int tmp = a;
    u8 FLAG_Z = 1 << 7;
    u8 FLAG_N = 1 << 6;
    u8 FLAG_H = 1 << 5;
    u8 FLAG_C = 1 << 4;

    if (!(m_F & FLAG_N))
    {
        if ((m_F & FLAG_H) || (tmp & 0x0F) > 9)
            tmp += 6;
        if ((m_F & FLAG_C) || tmp > 0x9F)
            tmp += 0x60;
    }
    else
    {
        if (m_F & FLAG_H)
        {
            tmp -= 6;
            if (!(m_F & FLAG_C))
                tmp &= 0xFF;
        }
        if (m_F & FLAG_C)
            tmp -= 0x60;
    }
    m_F &= ~(FLAG_H | FLAG_Z);
    if (tmp & 0x100)
        m_F |= FLAG_C;
    a = tmp & 0xFF;
    if (!a)
        m_F |= FLAG_Z;
}

u8 ALU::rotateLeftCircular(u8 value, bool &carry)
{
    bool bit7 = bit(value, 7);
    carry = bit7;
    return value << 1 | bit7;
}

u8 ALU::rotateRightCircular(u8 value, bool &carry)
{
    bool bit0 = value & 1;
    carry = bit0;
    return value >> 1 | bit0 << 7;
}

u8 ALU::rotateLeft(u8 value, bool &carry)
{
    bool prevCarry = carry;
    carry = bit(value, 7);
    return value << 1 | prevCarry;
}

u8 ALU::rotateRight(u8 value, bool &carry)
{
    bool prevCarry = carry;
    carry = value & 1;
    return value >> 1 | prevCarry << 7;
}

u8 ALU::shiftLeftA(u8 value, bool &carry)
{
    carry = bit(value, 7);
    return value << 1;
}

u8 ALU::shiftRightA(u8 value, bool &carry)
{
    carry = value & 1;
    return (value & 0b10000000) | value >> 1;
}

u8 ALU::shiftRightL(u8 value, bool &carry)
{
    carry = value & 1;
    return value >> 1;
}

u8 ALU::swap(u8 value)
{
    return (value << 4 & 0b11110000) | (value >> 4 & 0b00001111);
}

bool ALU::bit(u8 value, u8 b)
{
    return value >> b & 1;
}

u8 ALU::reset(u8 value, u8 b)
{
    return value & ~(1 << b);
}

u8 ALU::set(u8 value, u8 b)
{
    return value | 1 << b;
}
