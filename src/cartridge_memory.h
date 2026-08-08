#pragma once
#include "common.h"

class CartridgeMemory
{
protected:
    u32 romSize;
    u8* romData;

public:
    CartridgeMemory(u32 _romSize, u8* _romData);

    virtual u8 read(u16 address) = 0;
    virtual void write(u16 address, u8 value) = 0;
};

