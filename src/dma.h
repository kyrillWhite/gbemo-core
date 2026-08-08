#pragma once
#include "common.h"

class Memory;
class PPU;
class DMA
{
private:
    PPU* ppu;
    Memory* memory;

    bool active;
    u8 byte;
    u8 value;
    u8 startDelay;

public:
    DMA();

    void setMemory(Memory* _memory);
    void setPPU(PPU* _ppu);

    void start(u8 start);
    void tick();

    bool transferring();
};

