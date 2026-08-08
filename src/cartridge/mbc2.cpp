#include "cartridge/mbc2.h"

MBC2::MBC2(u32 _romSize, u8 *_romData, BatteryRam *_battery) : CartridgeMemory(_romSize, _romData),
                                                               ramEnable(false),
                                                               romBank(1),
                                                               battery(_battery)
{
    ramData = new u8[RAM_SIZE];
    if (battery && battery->saveExists())
    {
        battery->load(ramData, RAM_SIZE);
    }
    else
    {
        for (int i = 0; i < RAM_SIZE; i++)
        {
            ramData[i] = 0x0F;
        }
    }
}

MBC2::~MBC2()
{
    if (battery)
    {
        delete battery;
    }
    delete[] ramData;
}

u8 MBC2::read(u16 address)
{
    // cartridge static rom
    if (address <= 0x3FFF)
    {
        return romData[address];
    }
    // cartridge switch rom
    else if (address <= 0x7FFF)
    {
        u8 bank = romBank & 0x0F;
        if (bank == 0)
            bank = 1;
        u32 base = bank * 0x4000;
        return romData[base + (address - 0x4000)];
    }
    // built-in RAM
    else if (address >= 0xA000 && address <= 0xA1FF)
    {
        if (!ramEnable)
            return 0xFF;
        u16 offs = address & 0x01FF;
        return ramData[offs] & 0x0F;
    }
    else if (address >= 0xA200 && address <= 0xBFFF)
    {
        if (!ramEnable)
            return 0xFF;
        u16 offs = address & 0x01FF;
        return ramData[offs] & 0x0F;
    }
    else
    {
        return 0xFF;
    }
}

void MBC2::write(u16 address, u8 value)
{
    if (address <= 0x3FFF)
    {
        bool highBit = (address & 0x0100) != 0;

        if (!highBit)
        {
            ramEnable = ((value & 0x0F) == 0x0A);
        }
        else
        {
            romBank = value & 0x0F;
            if (romBank == 0)
                romBank = 1;
        }
    }
    // write to RAM if enabled
    else if (address >= 0xA000 && address <= 0xA1FF)
    {
        if (!ramEnable)
            return;
        u16 offs = address & 0x01FF;
        ramData[offs] = value & 0x0F;
        if (battery)
        {
            battery->save(ramData, RAM_SIZE);
        }
    }
    else if (address >= 0xA200 && address <= 0xBFFF)
    {
        if (!ramEnable)
            return;
        u16 offs = address & 0x01FF;
        ramData[offs] = value & 0x0F;
        if (battery)
        {
            battery->save(ramData, RAM_SIZE);
        }
    }
}