#include "register_file.h"

RegisterFile::RegisterFile(bool skipBoot)
{
    if (skipBoot) {
        A = 0x01;
        F = 0xB0;
        B = 0x00;
        C = 0x13;
        D = 0x00;
        E = 0xD8;
        H = 0x01;
        L = 0x4D;
        PC = 0x0100;
        SP = 0xFFFE;
        IR = 0x00;
    }
    else {
        A = 0x00;
        F = 0x00;
        B = 0x00;
        C = 0x00;
        D = 0x00;
        E = 0x00;
        H = 0x00;
        L = 0x00;
        SP = 0x0000;
        PC = 0x0000;
        IR = 0x00;
    }
}

u8 RegisterFile::getA()
{
    return A;
}

void RegisterFile::setA(u8 value)
{
    A = value;
}

u8 RegisterFile::getF()
{
    return F;
}

void RegisterFile::setF(u8 value)
{
    F = value & 0xF0;
}

u8 RegisterFile::getB()
{
    return B;
}

void RegisterFile::setB(u8 value)
{
    B = value;
}

u8 RegisterFile::getC()
{
    return C;
}

void RegisterFile::setC(u8 value)
{
    C = value;
}

u8 RegisterFile::getD()
{
    return D;
}

void RegisterFile::setD(u8 value)
{
    D = value;
}

u8 RegisterFile::getE()
{
    return E;
}

void RegisterFile::setE(u8 value)
{
    E = value;
}

u8 RegisterFile::getH()
{
    return H;
}

void RegisterFile::setH(u8 value)
{
    H = value;
}

u8 RegisterFile::getL()
{
    return L;
}

void RegisterFile::setL(u8 value)
{
    L = value;
}

u16 RegisterFile::getAF()
{
    return u16(A) << 8 | F;
}

void RegisterFile::setAF(u16 value)
{
    A = value >> 8;
    F = value & 0xF0;
}

u16 RegisterFile::getBC()
{
    return u16(B) << 8 | C;
}

void RegisterFile::setBC(u16 value)
{
    B = value >> 8;
    C = value & 0xFF;
}

u16 RegisterFile::getDE()
{
    return u16(D) << 8 | E;
}

void RegisterFile::setDE(u16 value)
{
    D = value >> 8;
    E = value & 0xFF;
}

u16 RegisterFile::getHL()
{
    return u16(H) << 8 | L;
}

void RegisterFile::setHL(u16 value)
{
    H = value >> 8;
    L = value & 0xFF;
}

u16 RegisterFile::getSP()
{
    return SP;
}

void RegisterFile::setSP(u16 value)
{
    SP = value;
}

u8 RegisterFile::getSPL()
{
    return SP & 0xFF;
}

u8 RegisterFile::getSPH()
{
    return SP >> 8;
}

u16 RegisterFile::getPC()
{
    return PC;
}

u8 RegisterFile::getPCL()
{
    return PC & 0xFF;
}

u8 RegisterFile::getPCH()
{
    return PC >> 8;
}

void RegisterFile::setPC(u16 value)
{
    PC = value;
}

u8 RegisterFile::getIR()
{
    return IR;
}

void RegisterFile::setIR(u8 value)
{
    IR = value;
}

void RegisterFile::setR8(u8 code, u8 value)
{
    switch (code)
    {
    case 0:
        B = value;
        break;
    case 1:
        C = value;
        break;
    case 2:
        D = value;
        break;
    case 3:
        E = value;
        break;
    case 4:
        H = value;
        break;
    case 5:
        L = value;
        break;
    case 6:
        throw std::invalid_argument("Setting memory data through HL register is not allowing. It must be processed by control unit.");
    case 7:
        A = value;
        break;
    default:
        throw std::invalid_argument("Support only three bit code");
    }
}

u8 RegisterFile::getR8(u8 code)
{
    switch (code)
    {
    case 0:
        return B;
    case 1:
        return C;
    case 2:
        return D;
    case 3:
        return E;
    case 4:
        return H;
    case 5:
        return L;
    case 6:
        throw std::invalid_argument("Getting memory data through HL register is not allowing. It must be processed by control unit.");
    case 7:
        return A;
    default:
        throw std::invalid_argument("Support only three bit code");
    }
}

void RegisterFile::setR16(u8 code, u16 value)
{
    switch (code)
    {
    case 0:
        setBC(value);
        break;
    case 1:
        setDE(value);
        break;
    case 2:
        setHL(value);
        break;
    case 3:
        setSP(value);
        break;
    default:
        throw std::invalid_argument("Support only two bit code");
    }
}

u16 RegisterFile::getR16(u8 code)
{
    switch (code)
    {
    case 0:
        return getBC();
    case 1:
        return getDE();
    case 2:
        return getHL();
    case 3:
        return getSP();
    default:
        throw std::invalid_argument("Support only two bit code");
    }
}

void RegisterFile::setR16stk(u8 code, u16 value)
{
    switch (code)
    {
    case 0:
        setBC(value);
        break;
    case 1:
        setDE(value);
        break;
    case 2:
        setHL(value);
        break;
    case 3:
        setAF(value);
        break;
    default:
        throw std::invalid_argument("Support only two bit code");
    }
}

u16 RegisterFile::getR16stk(u8 code)
{
    switch (code)
    {
    case 0:
        return getBC();
    case 1:
        return getDE();
    case 2:
        return getHL();
    case 3:
        return getAF();
    default:
        throw std::invalid_argument("Support only two bit code");
    }
}

u8 RegisterFile::getR16L(u8 code)
{
    u16 R16 = getR16(code);
    return R16 & 0xFF;
}

u8 RegisterFile::getR16M(u8 code)
{
    u16 R16 = getR16(code);
    return R16 >> 8;
}

u8 RegisterFile::getR16Lstk(u8 code)
{
    u16 R16 = getR16stk(code);
    return R16 & 0xFF;
}

u8 RegisterFile::getR16Mstk(u8 code)
{
    u16 R16 = getR16stk(code);
    return R16 >> 8;
}

void RegisterFile::setZflag(bool value)
{
    F = value ? ( F | 0b10000000) : (F & ~0b10000000);
}

bool RegisterFile::getZflag()
{
    return F >> 7 & 1;
}

void RegisterFile::setNflag(bool value)
{
    F = value ? (F | 0b01000000) : (F & ~0b01000000);
}

bool RegisterFile::getNflag()
{
    return F >> 6 & 1;
}

void RegisterFile::setHflag(bool value)
{
    F = value ? (F | 0b00100000) : (F & ~0b00100000);
}

bool RegisterFile::getHflag()
{
    return F >> 5 & 1;
}

void RegisterFile::setCflag(bool value)
{
    F = value ? (F | 0b00010000) : (F & ~0b00010000);
}

bool RegisterFile::getCflag()
{
    return F >> 4 & 1;
}

bool RegisterFile::checkConditional(u8 type)
{
    switch (type)
    {
    case 0b00000000:
        return !getZflag();
    case 0b00000001:
        return getZflag();
    case 0b00000010:
        return !getCflag();
    case 0b00000011:
        return getCflag();
    default:
        throw std::invalid_argument("Support only two bit code");
    }
}
