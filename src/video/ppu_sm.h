#pragma once
#include "common.h"
#include "video/lcd.h"
#include "system/interrupts.h"

class PPU;
class PpuStateMachine
{
private:
    LCD *lcd;
    PPU *ppu;
    Interrupts *interrupts;

    // The STAT interrupt is one wired-OR line, not four separate events: it
    // fires on the rising edge of "any enabled source is true" and stays quiet
    // while it is held high. Without this a mode-2 entry that arrives while
    // LY=LYC is already asserting the line raises a second, spurious request.
    bool statLine;

    void incrementLy();

public:
    PpuStateMachine(
        LCD *_lcd,
        PPU *_ppu,
        Interrupts *_interrupts);

    void oamMode();
    void xferMode();
    void vblankMode();
    void hblankMode();

    void updateStatLine();
};
