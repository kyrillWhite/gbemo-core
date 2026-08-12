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
    cartridge = new Cartridge();
    interrupts = new Interrupts();
    bus = new Bus();
    apu = new APU();
    dma = new DMA();
    joypad = new Joypad(interrupts);
    clock = new SystemClock();

    timer = new Timer(interrupts, apu, skipBoot);
    lcd = new LCD(dma, skipBoot);
    ppu = new PPU(lcd, skipBoot);
    ppuSm = new PpuStateMachine(lcd, ppu, interrupts);

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

    core = new Core(bus, interrupts, skipBoot);
    dbgReader = new DbgReader(memory);

    dma->setPPU(ppu);
    ppu->setStateMachine(ppuSm);
    dma->setMemory(memory);
    bus->setMemory(memory);
}

Emu::~Emu()
{
    delete dbgReader;
    delete core;
    delete memory;
    delete ppuSm;
    delete ppu;
    delete lcd;
    delete timer;
    delete clock;
    delete joypad;
    delete dma;
    delete apu;
    delete bus;
    delete interrupts;
    delete cartridge;
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

const char *Emu::getSerialLog()
{
    return memory->getSerialLog();
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
