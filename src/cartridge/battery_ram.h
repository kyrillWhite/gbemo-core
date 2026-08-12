#pragma once

#include <array>
#include <cstdio>
#include "common.h"

class BatteryRam
{
private:
    FILE *stream;
    bool saveExists;
    u32 fileSize;

    static constexpr u32 MAX_FILENAME_SIZE = 260;

    std::array<char, MAX_FILENAME_SIZE> filename;

public:
    BatteryRam(const char *romFilename);
    ~BatteryRam();

    bool isSaveExist();
    u32 size() const;

    void save(const u8 *ramData, u32 ramSize);
    void load(u8 *ramData, u32 ramSize);

    void saveByte(u32 offset, u8 value);

    void saveAt(u32 offset, const u8 *data, u32 size);
    bool loadAt(u32 offset, u8 *data, u32 size);
};
