#pragma once
#include "common.h"
#include "lcd.h"
#include "interrupts.h"

class PPU;
class PpuStateMachine
{
private:
    LCD* lcd;
    PPU* ppu;
    Interrupts* interrupts;

    void incrementLy();

public:
    PpuStateMachine(
        LCD* _lcd,
        PPU* _ppu,
        Interrupts* _interrupts
    );

    void oamMode();
    void xferMode();
    void vblankMode();
    void hblankMode();
};

