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

    // 0 for a cartridge without banking, otherwise banks-1: the ROM bank
    // number is masked with it, exactly like the missing address lines do.
    u8 romBankMask;

    // registers
    bool ramEnable;
    // 2000-3FFF, five bits; 4000-5FFF, two bits. bank2 doubles as the RAM bank
    // number: which one it drives is decided by bankingMode when the access
    // happens, not when the register is written.
    u8 bank1;
    u8 bank2;
    bool bankingMode;

    u32 romOffset(u16 address) const;
    u32 ramOffset(u16 address) const;

public:
    MBC1(u32 _romSize, const u8 *_romData, u32 _ramSize = 0, BatteryRam *_battery = nullptr);

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
