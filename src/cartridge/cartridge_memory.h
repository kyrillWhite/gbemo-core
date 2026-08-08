#pragma once
#include "common.h"

class CartridgeMemory
{
protected:
    u32 romSize;
    const u8 *romData;

    u8 readRom(u32 offset) const;

public:
    CartridgeMemory(u32 _romSize, const u8 *_romData);
    virtual ~CartridgeMemory() = default;

    virtual u8 read(u16 address) = 0;
    virtual void write(u16 address, u8 value) = 0;
};
