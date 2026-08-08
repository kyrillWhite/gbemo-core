#pragma once
#include "common.h"

class BatteryRam
{
private:
    const char *filename;

public:
    BatteryRam(const char *_filename);

    bool saveExists();

    void save(u8 *ramData, u32 ramSize);
    void load(u8 *ramData, u32 ramSize);
};
