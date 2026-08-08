#pragma once

#include "common.h"
#include "buttons.h"
#include "mono_color.h"

class Memory;
class Core;
class Bus;
class Cartridge;
class Interrupts;
class DbgReader;
class Timer;
class LCD;
class PPU;
class PpuStateMachine;
class DMA;
class SystemClock;
class Joypad;
class APU;

class Emu
{
private:
    Memory *memory;
    Core *core;
    Bus *bus;
    Cartridge *cartridge;
    Interrupts *interrupts;
    DbgReader *dbgReader;
    Timer *timer;
    LCD *lcd;
    PPU *ppu;
    PpuStateMachine *ppuSm;
    DMA *dma;
    SystemClock *clock;
    Joypad *joypad;
    APU *apu;

public:
    Emu(bool skipBoot = false);
    ~Emu();

    Emu(const Emu &) = delete;
    Emu &operator=(const Emu &) = delete;

    int loadRom(const char *romFilename);
    bool tick();

    void pressButton(Button button);
    void releaseButton(Button button);
    MonoColor *getVideoBuffer();
    i16 getLeftAudioSample();
    i16 getRightAudioSample();

    // read 384 tiles from vram
    void debugReadTiles(MonoColor *tiles);
};
