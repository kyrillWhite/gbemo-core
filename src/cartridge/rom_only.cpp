#include <algorithm>
#include "cartridge/rom_only.h"

RomOnly::RomOnly(u32 _romSize, const u8 *_romData) : CartridgeMemory(std::min(_romSize, ROM_ONLY_MAX_ROM_SIZE), _romData)
{
}

u8 RomOnly::read(u16 address)
{
    if (address <= 0x7FFF)
    {
        return readRom(address);
    }
    else
    {
        return 0xFF;
    }
}

void RomOnly::write(u16 address, u8 value)
{
    return;
}
