#pragma once

#include "common.h"

class Timer;
class PPU;
class APU;
class DMA;

class Peripherals
{
private:
    Timer *timer;
    PPU *ppu;
    APU *apu;
    DMA *dma;

    void tickT();

public:
    Peripherals(Timer *_timer, PPU *_ppu, APU *_apu, DMA *_dma);

    void openCycle();
    void closeCycle();
};
