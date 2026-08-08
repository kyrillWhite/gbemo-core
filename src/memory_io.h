#pragma once
#include "common.h"
#include "timer.h"
#include "lcd.h"
#include "joypad.h"

class MemoryIO
{
private:
    Interrupts *interrupts;
    Timer *timer;
    LCD *lcd;
    APU *apu;

    bool booted;
    u8 *serial;
    Joypad *joypad;

public:
    MemoryIO(
        Interrupts *_interrupts,
        Timer *_timer,
        LCD *_lcd,
        Joypad *_joypad,
        APU *_apu);
    ~MemoryIO();

    bool isBooted();

    u8 read(u16 from);
    void write(u16 to, u8 value);
};
