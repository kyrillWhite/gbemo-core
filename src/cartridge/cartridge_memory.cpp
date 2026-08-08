#include "cartridge/cartridge_memory.h"

CartridgeMemory::CartridgeMemory(u32 _romSize, const u8 *_romData) : romSize(_romSize),
                                                                     romData(_romData)
{
}

u8 CartridgeMemory::readRom(u32 offset) const
{
    return offset < romSize ? romData[offset] : 0xFF;
}
