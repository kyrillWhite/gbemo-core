#pragma once

#include <array>
#include "cartridge/cartridge_memory.h"
#include "cartridge/cartridge_limits.h"
#include "cartridge/cartridge_type.h"
#include "cartridge/battery_ram.h"

class MBC1 : public CartridgeMemory
{
private:
    BatteryRam *battery;
    u32 ramSize;
    std::array<u8, MBC1_MAX_RAM_SIZE> ramData;

    // registers
    bool ramEnable;
    u8 romBankNumber;
    u8 ramBankNumber;
    u8 secondRomBankNumber;
    bool bankingMode;

public:
    MBC1(u32 _romSize, const u8 *_romData, u32 _ramSize = 0, BatteryRam *_battery = nullptr);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
