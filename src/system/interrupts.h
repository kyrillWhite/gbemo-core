#pragma once
#include "common.h"
#include "system/interrupt_type.h"
#include "cpu/interrupt_command.h"

class Interrupts
{
private:
    u8 IE;
    u8 IF;

public:
    Interrupts();

    // Get Interrupt enable (0xFFFF)
    u8 getIE();
    // Get Interrupt flag (0xFF0F)
    u8 getIF();
    // SetInterrupt enable (0xFFFF)
    void setIE(u8 value);
    // SetInterrupt flag (0xFF0F)
    void setIF(u8 value);

    bool getIFflag(InterruptType type);
    void setIFflag(InterruptType type, bool value);
    bool getIEflag(InterruptType type);
    void setIEflag(InterruptType type, bool value);
};
