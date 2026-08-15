#include "system/peripherals.h"
#include "system/timer.h"
#include "video/ppu.h"
#include "video/dma.h"
#include "audio/apu.h"

Peripherals::Peripherals(Timer *_timer, PPU *_ppu, APU *_apu, DMA *_dma) : timer(_timer),
                                                                           ppu(_ppu),
                                                                           apu(_apu),
                                                                           dma(_dma)
{
}

void Peripherals::tickT()
{
    timer->tick();
    ppu->tick();
    apu->tick();

    globalTicks++;
}

void Peripherals::openCycle()
{
    timer->tick();
}

void Peripherals::closeCycle()
{
    dma->tick();
    ppu->tick();
    apu->tick();

    globalTicks++;

    tickT();
    tickT();
    tickT();
}
