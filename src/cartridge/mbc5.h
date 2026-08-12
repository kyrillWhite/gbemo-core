#pragma once

#include <array>
#include "cartridge/cartridge_memory.h"
#include "cartridge/cartridge_limits.h"
#include "cartridge/battery_ram.h"

class MBC5 : public CartridgeMemory
{
private:
    BatteryRam *battery;
    u32 ramSize;
    std::array<u8, MBC5_MAX_RAM_SIZE> ramData;
    // rumble carts steal bit 3 of the RAM bank register for the motor, so it
    // must not reach the bank number
    bool hasRumble;

    u16 romBankMask;

    // registers
    bool ramEnable;
    u16 romBank;
    u8 ramBank;

public:
    MBC5(u32 _romSize, const u8 *_romData, u32 _ramSize = 0, BatteryRam *_battery = nullptr, bool _hasRumble = false);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
