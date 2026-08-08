#pragma once
#include "common.h"
#include "system/timer.h"
#include "video/lcd.h"
#include "system/joypad.h"

class MemoryIO
{
private:
    Interrupts *interrupts;
    Timer *timer;
    LCD *lcd;
    APU *apu;

    bool booted;
    u8 serial[2];
    Joypad *joypad;

public:
    MemoryIO(
        Interrupts *_interrupts,
        Timer *_timer,
        LCD *_lcd,
        APU *_apu,
        Joypad *_joypad);

    bool isBooted();

    u8 read(u16 from);
    void write(u16 to, u8 value);
};
