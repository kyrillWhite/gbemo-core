#include <algorithm>
#include "cartridge/mbc1.h"

// banks-1, rounded up to a power of two: the cartridge only wires as many bank
// lines as it has banks, so a write above that wraps rather than reading past
// the end of the ROM.
static u8 bankMaskFor(u32 romSize)
{
    u32 banks = romSize / (16 * KIB);
    u8 mask = 0;
    while (mask + 1u < banks && mask < 0x7F)
    {
        mask = static_cast<u8>((mask << 1) | 1);
    }
    return mask;
}

MBC1::MBC1(u32 _romSize, const u8 *_romData, u32 _ramSize, BatteryRam *_battery) : CartridgeMemory(std::min(_romSize, MBC1_MAX_ROM_SIZE), _romData),
                                                                                   battery(_battery),
                                                                                   ramSize(std::min(_ramSize, MBC1_MAX_RAM_SIZE)),
                                                                                   romBankMask(bankMaskFor(std::min(_romSize, MBC1_MAX_ROM_SIZE))),
                                                                                   ramEnable(false),
                                                                                   bank1(1),
                                                                                   bank2(0),
                                                                                   bankingMode(false)
{
    if (ramSize == 0)
    {
        return;
    }

    ramData.assign(ramSize, 0xFF);
    if (battery)
    {
        if (battery->isSaveExist())
        {
            battery->load(ramData.data(), ramSize);
        }
        else
        {
            battery->save(ramData.data(), ramSize);
        }
    }
}

u32 MBC1::romOffset(u16 address) const
{
    // 0000-3FFF is bank 0, except in mode 1 where the two high bank lines
    // reach it as well - that is how the large MBC1 carts see banks 0x20,
    // 0x40 and 0x60.
    if (address <= 0x3FFF)
    {
        u8 bank = bankingMode ? static_cast<u8>((bank2 & 0b11) << 5) : 0;
        return (bank & romBankMask) * 0x4000u + address;
    }

    u8 low5 = bank1 & 0b11111;
    if (low5 == 0)
    {
        low5 = 1;
    }
    u8 bank = static_cast<u8>(((bank2 & 0b11) << 5) | low5);
    return (bank & romBankMask) * 0x4000u + (address - 0x4000u);
}

u32 MBC1::ramOffset(u16 address) const
{
    u8 bank = bankingMode ? (bank2 & 0b11) : 0;
    return (address - 0xA000u) + bank * 0x2000u;
}

u8 MBC1::read(u16 address)
{
    if (address <= 0x7FFF)
    {
        return readRom(romOffset(address));
    }
    // switchable ram
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return 0xFF;
        }

        u32 offset = ramOffset(address);
        return offset < ramSize ? ramData[offset] : 0xFF;
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
        ramEnable = (value & 0x0F) == 0x0A;
    }
    else if (address <= 0x3FFF)
    {
        bank1 = value & 0b11111;
    }
    else if (address <= 0x5FFF)
    {
        bank2 = value & 0b11;
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

        u32 offset = ramOffset(address);
        if (offset >= ramSize)
        {
            return;
        }

        ramData[offset] = value;
        if (battery)
        {
            battery->saveByte(offset, value);
        }
    }
}
