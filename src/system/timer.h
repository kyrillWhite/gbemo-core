#pragma once
#include "common.h"
#include "system/interrupts.h"
#include "audio/apu.h"

class Timer
{
private:
    u16 div;
    u8 tima;
    u8 tma;
    u8 tac;

    // TIMA does not reload the instant it overflows: it reads back as zero for
    // four T-cycles, and only then takes TMA and raises the interrupt. Games
    // that rewrite TIMA or TMA inside that window depend on the gap.
    u8 reloadDelay;
    bool reloadedThisTick;

    // TIMA is clocked by the falling edge of (a bit of the divider AND the
    // enable bit), not by a counter of its own. Writing DIV or TAC can drop
    // that signal and clock TIMA as a side effect, which is why the check
    // lives in one place both the tick and the writes go through.
    bool prevEdge;

    Interrupts *interrupts;
    APU *apu;

    void updateEdge();

public:
    Timer(Interrupts *_interrupts, APU *_apu, bool skipBoot = false);

    void tick();
    void write(u16 address, u8 value);
    u8 read(u16 address);
};
