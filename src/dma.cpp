#include "dma.h"
#include "memory.h"
#include "ppu.h"

DMA::DMA() : active(false), byte(0), value(0), startDelay(0), memory(nullptr), ppu(nullptr)
{
}

void DMA::setMemory(Memory *_memory)
{
    memory = _memory;
}

void DMA::setPPU(PPU *_ppu)
{
    ppu = _ppu;
}

void DMA::start(u8 start)
{
    active = true;
    byte = 0;
    value = start;
    startDelay = 2;
}

void DMA::tick()
{
    if (!active)
    {
        return;
    }

    if (startDelay)
    {
        startDelay--;
        return;
    }

    ppu->oamWrite(byte, memory->read(value * 0x100 + byte), true);

    byte++;

    active = byte < 0xA0;
}

bool DMA::transferring()
{
    return active;
}
