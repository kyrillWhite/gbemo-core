#pragma once

#include "cartridge_memory.h"
#include "battery_ram.h"

class MBC2 : public CartridgeMemory {
private:
    u8* ramData;
    static constexpr std::size_t RAM_SIZE = 512;
    bool ramEnable;
    u8 romBank;
    BatteryRam* battery;

public:
    MBC2(u32 _romSize, u8* _romData, BatteryRam* _battery = nullptr);
    ~MBC2();

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};