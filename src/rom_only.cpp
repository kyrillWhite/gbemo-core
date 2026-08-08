#include "rom_only.h"

RomOnly::RomOnly(u32 _romSize, u8* _romData) :
    CartridgeMemory(_romSize, _romData)
{}

u8 RomOnly::read(u16 address)
{
    if (address <= 0x7FFF) {
        return romData[address];
    }
    else {
        return 0xFF;
    }
}

void RomOnly::write(u16 address, u8 value)
{
    return;
}
