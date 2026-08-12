#include <algorithm>
#include "cartridge/mbc5.h"

static u16 bankMaskFor(u32 romSize)
{
    u32 banks = romSize / (16 * KIB);
    u16 mask = 0;
    while (mask + 1u < banks && mask < 0x1FF)
    {
        mask = static_cast<u16>((mask << 1) | 1);
    }
    return mask;
}

MBC5::MBC5(u32 _romSize, const u8 *_romData, u32 _ramSize, BatteryRam *_battery, bool _hasRumble) : CartridgeMemory(std::min(_romSize, MBC5_MAX_ROM_SIZE), _romData),
                                                                                                    battery(_battery),
                                                                                                    ramSize(std::min(_ramSize, MBC5_MAX_RAM_SIZE)),
                                                                                                    hasRumble(_hasRumble),
                                                                                                    romBankMask(bankMaskFor(std::min(_romSize, MBC5_MAX_ROM_SIZE))),
                                                                                                    ramEnable(false),
                                                                                                    romBank(1),
                                                                                                    ramBank(0)
{
    if (ramSize == 0)
    {
        return;
    }

    std::fill_n(ramData.begin(), ramSize, 0xFF);
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

u8 MBC5::read(u16 address)
{
    // cartridge static rom
    if (address <= 0x3FFF)
    {
        return readRom(address);
    }
    // cartridge switch rom - unlike every earlier mapper, bank 0 can be
    // selected here and means bank 0, not bank 1
    else if (address <= 0x7FFF)
    {
        return readRom((romBank & romBankMask) * 0x4000u + (address - 0x4000u));
    }
    // switchable ram
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return 0xFF;
        }

        u32 offset = (address - 0xA000u) + ramBank * 0x2000u;
        return offset < ramSize ? ramData[offset] : 0xFF;
    }
    else
    {
        return 0xFF;
    }
}

void MBC5::write(u16 address, u8 value)
{
    if (address <= 0x1FFF)
    {
        ramEnable = (value & 0x0F) == 0x0A;
    }
    else if (address <= 0x2FFF)
    {
        romBank = static_cast<u16>((romBank & 0x100) | value);
    }
    else if (address <= 0x3FFF)
    {
        romBank = static_cast<u16>((romBank & 0x0FF) | ((value & 1) << 8));
    }
    else if (address <= 0x5FFF)
    {
        ramBank = value & (hasRumble ? 0b0111 : 0b1111);
    }
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return;
        }

        u32 offset = (address - 0xA000u) + ramBank * 0x2000u;
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
