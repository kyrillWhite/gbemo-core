#pragma once

#include "common.h"
#include "memory/address_bus.h"
#include "cartridge/cartridge.h"
#include "memory/memory_io.h"
#include "memory/io.h"
#include "video/ppu.h"
#include "audio/apu.h"

class Memory
{
private:
    AddressBus *addressBus;
    Cartridge *cartridge;
    MemoryIO memoryIO;
    Interrupts *interrupts;
    LCD *lcd;
    PPU *ppu;
    DMA *dma;

    u8 wram0[4096];
    u8 wramN[4096];
    u8 hram[128];

public:
    Memory(
        AddressBus *_addressBus,
        Cartridge *_cartridge,
        Interrupts *interrupts,
        Timer *_timer,
        LCD *_lcd,
        PPU *_ppu,
        DMA *_dma,
        Joypad *_joypad,
        APU *_apu,
        bool skipBoot = false);

    u8 read();
    void write(u8 value);

    u8 read(u16 address);
    void write(u16 address, u8 value);

    const char *getSerialLog() const;
    u32 getSerialLogSize() const;
};
