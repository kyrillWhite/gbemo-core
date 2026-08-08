#pragma once
#include "common.h"

class AudioChannel
{
protected:
    u8 NR[5];
    bool enabled;

public:
    virtual u8 read(u16 address) = 0;
    virtual void write(u16 address, u8 value) = 0;
    virtual void trigger() = 0;
    virtual bool dac() = 0;
    virtual bool isEnabled() = 0; // including dac

    // return value in range between -1.0 and 1.0
    virtual double getSample() = 0;
};
