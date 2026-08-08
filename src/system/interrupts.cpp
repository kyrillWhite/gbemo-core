#include "system/interrupts.h"

Interrupts::Interrupts() : IE(0),
                           IF(0)
{
}

u8 Interrupts::getIE()
{
    return IE;
}

u8 Interrupts::getIF()
{
    return IF;
}

void Interrupts::setIE(u8 value)
{
    IE = value;
}

void Interrupts::setIF(u8 value)
{
    IF = value;
}

bool Interrupts::getIFflag(InterruptType type)
{
    return bool(IF & type);
}

void Interrupts::setIFflag(InterruptType type, bool value)
{
    IF = value ? (IF | type) : (IF & ~type);
}

bool Interrupts::getIEflag(InterruptType type)
{
    return bool(IE & type);
}

void Interrupts::setIEflag(InterruptType type, bool value)
{
    IE = value ? (IE | type) : (IE & ~type);
}
