#pragma once

#include <array>
#include "cartridge/cartridge_memory.h"
#include "cartridge/cartridge_limits.h"
#include "cartridge/battery_ram.h"

// The real-time clock the MBC3 carts with a battery-backed crystal expose
// through A000-BFFF. Five counters, plus the frozen copy the game reads after
// writing the latch sequence to 6000-7FFF.
struct Rtc
{
    u8 seconds;
    u8 minutes;
    u8 hours;
    u8 dayLow;
    u8 dayHigh; // bit 0: day counter bit 8, bit 6: halt, bit 7: day overflow
};

class MBC3 : public CartridgeMemory
{
private:
    BatteryRam *battery;
    u32 ramSize;
    std::array<u8, MBC3_MAX_RAM_SIZE> ramData;
    bool hasRtc;

    u8 romBankMask;

    // registers
    bool ramEnable;
    u8 romBank;
    // 0x00-0x07 selects a RAM bank, 0x08-0x0C an RTC register
    u8 ramBank;
    u8 latchState;

    Rtc rtc;
    Rtc latched;
    u64 lastTick;

    void tickRtc();
    u8 *rtcRegister(u8 select);

    void loadRtcState();
    void saveRtcState();

public:
    MBC3(u32 _romSize, const u8 *_romData, u32 _ramSize = 0, BatteryRam *_battery = nullptr, bool _hasRtc = false);
    ~MBC3() override;

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
};
