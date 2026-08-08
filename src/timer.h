#pragma once
#include "common.h"
#include "interrupts.h"
#include "apu.h"

class Timer
{
private:
    u16 div;
    u8 tima;
    u8 tma;
    u8 tac;

    Interrupts *interrupts;
    APU *apu;

public:
    Timer(Interrupts *_interrupts, APU *_apu, bool skipBoot = false);

    void tick();
    void write(u16 address, u8 value);
    u8 read(u16 address);
};
