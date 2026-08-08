#pragma once

#include <array>
#include "common.h"

class BatteryRam
{
private:
    static constexpr u32 MAX_FILENAME_SIZE = 260;

    std::array<char, MAX_FILENAME_SIZE> filename;

public:
    BatteryRam(const char *romFilename);

    bool isSaveExist();

    void save(const u8 *ramData, u32 ramSize);
    void load(u8 *ramData, u32 ramSize);
};
