#pragma once

#include "common.h"

class RegisterFile
{
private:
    u8 A; // Hi AF, accum
    u8 F; // Flag register
    u8 B; // Hi BC
    u8 C; // Lo BC
    u8 D; // Hi DE
    u8 E; // Lo DE
    u8 H; // Hi HL
    u8 L; // Lo HL

    u16 SP; // Stack Pointer
    u16 PC; // Program Counter/Pointer

    u8 IR; // Instruction Register

public:
    RegisterFile(bool dmgBootInit = false);

    u8 getA();
    void setA(u8 value);
    u8 getF();
    void setF(u8 value);
    u8 getB();
    void setB(u8 value);
    u8 getC();
    void setC(u8 value);
    u8 getD();
    void setD(u8 value);
    u8 getE();
    void setE(u8 value);
    u8 getH();
    void setH(u8 value);
    u8 getL();
    void setL(u8 value);

    u16 getAF();
    void setAF(u16 value);
    u16 getBC();
    void setBC(u16 value);
    u16 getDE();
    void setDE(u16 value);
    u16 getHL();
    void setHL(u16 value);

    u16 getSP();
    void setSP(u16 value);
    u8 getSPL();
    u8 getSPH();
    u16 getPC();
    u8 getPCL();
    u8 getPCH();
    void setPC(u16 value);

    u8 getIR();
    void setIR(u8 value);

    void setR8(u8 code, u8 value);
    u8 getR8(u8 code);

    void setR16(u8 code, u16 value);
    u16 getR16(u8 code);
    void setR16stk(u8 code, u16 value);
    u16 getR16stk(u8 code);
    u8 getR16L(u8 code);
    u8 getR16M(u8 code);
    u8 getR16Lstk(u8 code);
    u8 getR16Mstk(u8 code);

    void setZflag(bool value);
    bool getZflag();
    void setNflag(bool value);
    bool getNflag();
    void setHflag(bool value);
    bool getHflag();
    void setCflag(bool value);
    bool getCflag();

    bool checkConditional(u8 type);
};
