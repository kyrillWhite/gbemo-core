#include <random>
#include "memory/memory.h"
#include "memory/dmg_boot.h"

Memory::Memory(
    AddressBus *_addressBus,
    Cartridge *_cartridge,
    Interrupts *_interrupts,
    Timer *_timer,
    LCD *_lcd,
    PPU *_ppu,
    DMA *_dma,
    Joypad *_joypad,
    APU *_apu,
    bool skipBoot) : addressBus(_addressBus),
                     cartridge(_cartridge),
                     interrupts(_interrupts),
                     lcd(_lcd),
                     ppu(_ppu),
                     memoryIO(_interrupts, _timer, _lcd, _apu, _joypad),
                     dma(_dma)
{
    if (skipBoot)
    {
        memoryIO.write(0xFF50, 1);
    }
}

u8 Memory::read()
{
    auto address = addressBus->getAddress();
    return read(address);
}

void Memory::write(u8 value)
{
    auto address = addressBus->getAddress();
    write(address, value);
}

u8 Memory::read(u16 address)
{
    if (!memoryIO.isBooted() && address <= 0x00FF)
    {
        return dmgBoot[address];
    }

    // cartridge static rom
    if (address <= 0x3FFF)
    {
        return cartridge->read(address);
    }
    // cartridge switch rom
    else if (address <= 0x7FFF)
    {
        return cartridge->read(address);
    }
    // video ram
    else if (address <= 0x9FFF)
    {
        return ppu->vramRead(address);
    }
    // cartridge switch ram
    else if (address <= 0xBFFF)
    {
        return cartridge->read(address);
    }
    // work ram 0
    else if (address <= 0xCFFF)
    {
        return wram0[address - 0xC000];
    }
    // work ram 1-7
    else if (address <= 0xDFFF)
    {
        return wramN[address - 0xD000];
    }
    // echo ram
    else if (address <= 0xFDFF)
    {
        return wram0[address - 0xE000];
    }
    // object attribute memory
    else if (address <= 0xFE9F)
    {
        if (dma->transferring())
        {
            return 0xFF;
        }
        return ppu->oamRead(address);
    }
    // not usable
    else if (address <= 0xFEFF)
    {
        return 0; // TODO: https://gbdev.io/pandocs/Memory_Map.html#fea0feff-range
    }
    // i/o registers
    else if (address <= 0xFF7F)
    {
        return memoryIO.read(address);
    }
    // high ram
    else if (address <= 0xFFFE)
    {
        return hram[address - 0xFF80];
    }
    // 0xFFFF Interrupt Enable register (IE)
    else
    {
        return interrupts->getIE();
    }
}

void Memory::write(u16 address, u8 value)
{
    // cartridge registers manipulating
    if (address <= 0x7FFF)
    {
        cartridge->write(address, value);
    }
    // video ram
    else if (address <= 0x9FFF)
    {
        ppu->vramWrite(address, value);
    }
    // cartridge switch ram
    else if (address <= 0xBFFF)
    {
        cartridge->write(address, value);
    }
    // work ram 0
    else if (address <= 0xCFFF)
    {
        wram0[address - 0xC000] = value;
    }
    // work ram 1-7
    else if (address <= 0xDFFF)
    {
        wramN[address - 0xD000] = value;
    }
    // echo ram
    else if (address <= 0xFDFF)
    {
        wram0[address - 0xE000] = value;
    }
    // object attribute memory
    else if (address <= 0xFE9F)
    {
        if (dma->transferring())
        {
            return;
        }
        ppu->oamWrite(address, value);
    }
    // not usable
    else if (address <= 0xFEFF)
    {
        return;
        NOT_AVAILABLE
    }
    // i/o registers
    else if (address <= 0xFF7F)
    {
        memoryIO.write(address, value);
    }
    // high ram
    else if (address <= 0xFFFE)
    {
        hram[address - 0xFF80] = value;
    }
    // 0xFFFF Interrupt Enable register (IE)
    else
    {
        interrupts->setIE(value);
    }
}
