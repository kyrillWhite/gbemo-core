#pragma once
#include "cartridge_memory.h"

class RomOnly : public CartridgeMemory
{
public:
    RomOnly(u32 _romSize, u8 *_romData);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
