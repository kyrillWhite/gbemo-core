#pragma once
#include "cartridge/cartridge_memory.h"
#include "cartridge/cartridge_limits.h"

class RomOnly : public CartridgeMemory
{
public:
    RomOnly(u32 _romSize, const u8 *_romData);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
