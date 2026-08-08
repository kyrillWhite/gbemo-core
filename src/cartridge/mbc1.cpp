#include <algorithm>
#include "cartridge/mbc1.h"

MBC1::MBC1(u32 _romSize, const u8 *_romData, u32 _ramSize, BatteryRam *_battery) : CartridgeMemory(std::min(_romSize, MBC1_MAX_ROM_SIZE), _romData),
                                                                                   battery(_battery),
                                                                                   ramSize(std::min(_ramSize, MBC1_MAX_RAM_SIZE)),
                                                                                   ramData{},
                                                                                   ramEnable(false),
                                                                                   romBankNumber(0),
                                                                                   ramBankNumber(0),
                                                                                   secondRomBankNumber(0),
                                                                                   bankingMode(false)
{
    if (ramSize == 0)
    {
        return;
    }

    ramData.fill(0x0F);
    if (battery && battery->isSaveExist())
    {
        battery->load(ramData.data(), ramSize);
    }
}

u8 MBC1::read(u16 address)
{
    // cartridge static rom
    if (address <= 0x3FFF)
    {
        u8 high = bankingMode ? 0 : (secondRomBankNumber & 0b11) << 5;
        return readRom(address + high * 0x4000);
    }
    // cartridge switch rom
    else if (address <= 0x7FFF)
    {
        u8 low5 = romBankNumber & 0b11111;
        if (low5 == 0)
            low5 = 1;
        u8 high2 = (secondRomBankNumber & 0b11) << 5;
        return readRom((address - 0x4000) + (high2 | low5) * 0x4000);
    }
    // switchable ram
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return 0xFF;
        }
        u8 bankNumber = bankingMode ? (ramBankNumber & 0b11) : 0;
        u32 offset = (address - 0xA000) + bankNumber * 0x2000;

        if (offset < ramSize)
        {
            return ramData[offset];
        }
        else
        {
            return 0xFF;
        }
    }
    else
    {
        return 0xFF;
    }
}

void MBC1::write(u16 address, u8 value)
{
    if (address <= 0x1FFF)
    {
        ramEnable = value == 0xA;
    }
    else if (address <= 0x3FFF)
    {
        romBankNumber = value & 0b11111;
    }
    else if (address <= 0x5FFF)
    {
        if (bankingMode == 0)
        {
            secondRomBankNumber = value & 0b11;
        }
        else
        {
            ramBankNumber = value & 0b11;
        }
    }
    else if (address <= 0x7FFF)
    {
        bankingMode = value & 1;
    }
    // switchable ram
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return;
        }
        u8 bankNumber = bankingMode ? (ramBankNumber & 0b11) : 0;
        u32 offset = address - 0xA000 + ramBankNumber * 0x2000;

        if (offset < ramSize)
        {
            ramData[offset] = value;
        }

        if (battery)
        {
            battery->save(ramData.data(), ramSize);
        }
    }
}
