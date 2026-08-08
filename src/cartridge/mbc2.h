#pragma once

#include <array>
#include "cartridge/cartridge_memory.h"
#include "cartridge/cartridge_limits.h"
#include "cartridge/battery_ram.h"

class MBC2 : public CartridgeMemory
{
private:
    std::array<u8, MBC2_MAX_RAM_SIZE> ramData;
    bool ramEnable;
    u8 romBank;
    BatteryRam *battery;

public:
    MBC2(u32 _romSize, const u8 *_romData, BatteryRam *_battery = nullptr);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
