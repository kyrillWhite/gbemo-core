#include "emu.h"
#include "memory/memory.h"
#include "cpu/core.h"
#include "memory/bus.h"
#include "memory/io.h"
#include "debug/dbg_reader.h"
#include "system/system_clock.h"
#include "system/joypad.h"
#include "audio/apu.h"

Emu::Emu(bool skipBoot)
{
    apu = new APU();
    interrupts = new Interrupts();
    timer = new Timer(interrupts, apu, skipBoot);
    cartridge = new Cartridge();
    bus = new Bus();
    dma = new DMA();
    lcd = new LCD(dma, skipBoot);
    ppu = new PPU(lcd, skipBoot);
    dma->setPPU(ppu);
    ppuSm = new PpuStateMachine(lcd, ppu, interrupts);
    ppu->setStateMachine(ppuSm);
    joypad = new Joypad();
    memory = new Memory(
        bus->getAddressBus(),
        cartridge,
        interrupts,
        timer,
        lcd,
        ppu,
        dma,
        joypad,
        apu,
        skipBoot);
    dma->setMemory(memory);
    bus->setMemory(memory);
    core = new Core(bus, interrupts, skipBoot);
    dbgReader = new DbgReader(memory);
    clock = new SystemClock();
}

Emu::~Emu()
{
    delete interrupts;
    delete timer;
    delete cartridge;
    delete bus;
    delete memory;
    delete core;
    delete dbgReader;
    delete clock;
    delete lcd;
    delete ppu;
    delete ppuSm;
    delete dma;
    delete joypad;
    delete apu;
}

int Emu::loadRom(const char *romFilename)
{
    return cartridge->readRomFile(romFilename);
}

bool Emu::tick()
{
    // if (clock->isAllowTick()) {
#ifdef DELAY_PRINT
    if (globalTicks % PRINT_SKIP == 0)
    {
        printf("Tick: %10I64u; Delay: %10I64u ns\n", globalTicks, clock->getDelay());
    }
#endif // DELAY_PRINT

    // timer tick must be before core tick
    timer->tick();

    if (globalTicks % 4 == 0)
    {
        core->tick();
        dma->tick();

        dbgReader->update();
        dbgReader->print();
    }

    ppu->tick();
    apu->tick();

    globalTicks++;

    return true;
    //}
    // return false;
}

void Emu::pressButton(Button button)
{
    joypad->pressButton(button);
}

i16 Emu::getLeftAudioSample()
{
    return apu->getLeftSample();
}

i16 Emu::getRightAudioSample()
{
    return apu->getRightSample();
}

void Emu::releaseButton(Button button)
{
    joypad->releaseButton(button);
}

MonoColor *Emu::getVideoBuffer()
{
    return (MonoColor *)ppu->getVideoBuffer();
}

void Emu::debugReadTiles(MonoColor *tiles)
{
    u8 *vram = ppu->getVRAM();
    u8 palette = memory->read(0xFF47);
    for (int i = 0; i < 384; i++)
    {
        u8 *tilePtr = vram + i * 16;

        for (int j = 0; j < 64; j++)
        {
            u8 byte = j / 8;
            u8 bit = (7 - j % 8);
            u8 colorId = ((tilePtr[2 * byte] >> bit) & 0x01) << 1 | ((tilePtr[2 * byte + 1] >> bit) & 0x01);
            u8 color = (palette >> (colorId * 2)) & 0x03;

            tiles[i * 64 + j] = MonoColor((palette >> (2 * colorId)) & 0b00000011);
        }
    }
}
