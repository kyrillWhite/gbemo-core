#pragma once
#include "common.h"

class IO
{
private:
public:
    u8 read(u16 address);
    void write(u16 address, u8 value);

    IO();
};
