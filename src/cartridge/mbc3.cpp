#include <algorithm>
#include <cstring>
#include "cartridge/mbc3.h"

// One RTC second per second of emulated machine time.
static constexpr u64 TICKS_PER_SECOND = 4194304;

// The RTC trailer every other emulator writes after the SRAM: ten 32-bit
// little-endian counters (live, then latched) and a 64-bit timestamp we do not
// use. Keeping the layout means an existing .sav still carries its clock.
static constexpr u32 RTC_STATE_SIZE = 48;

static u8 bankMaskFor(u32 romSize)
{
    u32 banks = romSize / (16 * KIB);
    u8 mask = 0;
    while (mask + 1u < banks && mask < 0x7F)
    {
        mask = static_cast<u8>((mask << 1) | 1);
    }
    return mask;
}

MBC3::MBC3(u32 _romSize, const u8 *_romData, u32 _ramSize, BatteryRam *_battery, bool _hasRtc) : CartridgeMemory(std::min(_romSize, MBC3_MAX_ROM_SIZE), _romData),
                                                                                                 battery(_battery),
                                                                                                 ramSize(std::min(_ramSize, MBC3_MAX_RAM_SIZE)),
                                                                                                 hasRtc(_hasRtc),
                                                                                                 romBankMask(bankMaskFor(std::min(_romSize, MBC3_MAX_ROM_SIZE))),
                                                                                                 ramEnable(false),
                                                                                                 romBank(1),
                                                                                                 ramBank(0),
                                                                                                 latchState(0xFF),
                                                                                                 rtc{},
                                                                                                 latched{},
                                                                                                 lastTick(globalTicks)
{
    if (ramSize != 0)
    {
        ramData.assign(ramSize, 0xFF);
        if (battery)
        {
            if (battery->isSaveExist())
            {
                battery->load(ramData.data(), ramSize);
            }
            else
            {
                battery->save(ramData.data(), ramSize);
            }
        }
    }

    if (hasRtc && battery)
    {
        loadRtcState();
    }
}

MBC3::~MBC3()
{
    if (hasRtc && battery)
    {
        tickRtc();
        saveRtcState();
    }
}

void MBC3::tickRtc()
{
    if (!hasRtc)
    {
        return;
    }

    u64 elapsed = globalTicks - lastTick;
    u64 seconds = elapsed / TICKS_PER_SECOND;
    lastTick += seconds * TICKS_PER_SECOND;

    // bit 6 of the day counter high byte stops the clock
    if (seconds == 0 || (rtc.dayHigh & 0x40))
    {
        return;
    }

    u64 total = rtc.seconds + seconds;
    rtc.seconds = static_cast<u8>(total % 60);
    u64 carry = total / 60;
    if (carry == 0)
    {
        return;
    }

    total = rtc.minutes + carry;
    rtc.minutes = static_cast<u8>(total % 60);
    carry = total / 60;
    if (carry == 0)
    {
        return;
    }

    total = rtc.hours + carry;
    rtc.hours = static_cast<u8>(total % 24);
    carry = total / 24;
    if (carry == 0)
    {
        return;
    }

    u64 days = (static_cast<u64>(rtc.dayHigh & 1) << 8 | rtc.dayLow) + carry;
    rtc.dayLow = static_cast<u8>(days & 0xFF);
    rtc.dayHigh = static_cast<u8>((rtc.dayHigh & 0xC0) | ((days >> 8) & 1));
    if (days > 0x1FF)
    {
        rtc.dayHigh |= 0x80; // the day counter wrapped and stays flagged
    }
}

u8 *MBC3::rtcRegister(u8 select)
{
    switch (select)
    {
    case 0x08:
        return &latched.seconds;
    case 0x09:
        return &latched.minutes;
    case 0x0A:
        return &latched.hours;
    case 0x0B:
        return &latched.dayLow;
    case 0x0C:
        return &latched.dayHigh;
    default:
        return nullptr;
    }
}

u8 MBC3::read(u16 address)
{
    // cartridge static rom
    if (address <= 0x3FFF)
    {
        return readRom(address);
    }
    // cartridge switch rom
    else if (address <= 0x7FFF)
    {
        u8 bank = romBank & 0x7F;
        if (bank == 0)
        {
            bank = 1;
        }
        return readRom((bank & romBankMask) * 0x4000u + (address - 0x4000u));
    }
    // switchable ram, or one of the clock registers
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return 0xFF;
        }

        if (hasRtc && ramBank >= 0x08)
        {
            const u8 *reg = rtcRegister(ramBank);
            return reg ? *reg : 0xFF;
        }

        u32 offset = (address - 0xA000u) + (ramBank & 0b11) * 0x2000u;
        return offset < ramSize ? ramData[offset] : 0xFF;
    }
    else
    {
        return 0xFF;
    }
}

void MBC3::write(u16 address, u8 value)
{
    if (address <= 0x1FFF)
    {
        ramEnable = (value & 0x0F) == 0x0A;
    }
    // Seven bits, all of them here: this is what an MBC1 cannot do, and why a
    // 1 MiB MBC3 cart driven by MBC1 logic loses every bank from 0x20 up.
    else if (address <= 0x3FFF)
    {
        romBank = value & 0x7F;
    }
    else if (address <= 0x5FFF)
    {
        ramBank = value;
    }
    // 0 then 1 freezes the clock into the registers the game reads
    else if (address <= 0x7FFF)
    {
        if (latchState == 0x00 && value == 0x01)
        {
            tickRtc();
            latched = rtc;
        }
        latchState = value;
    }
    else if (address >= 0xA000 && address <= 0xBFFF)
    {
        if (!ramEnable)
        {
            return;
        }

        if (hasRtc && ramBank >= 0x08)
        {
            tickRtc();
            switch (ramBank)
            {
            case 0x08:
                rtc.seconds = value % 60;
                lastTick = globalTicks; // writing the seconds restarts the divider
                break;
            case 0x09:
                rtc.minutes = value % 60;
                break;
            case 0x0A:
                rtc.hours = value % 24;
                break;
            case 0x0B:
                rtc.dayLow = value;
                break;
            case 0x0C:
                rtc.dayHigh = value & 0xC1;
                break;
            default:
                return;
            }
            latched = rtc;
            return;
        }

        u32 offset = (address - 0xA000u) + (ramBank & 0b11) * 0x2000u;
        if (offset >= ramSize)
        {
            return;
        }

        ramData[offset] = value;
        if (battery)
        {
            battery->saveByte(offset, value);
        }
    }
}

void MBC3::loadRtcState()
{
    u8 raw[RTC_STATE_SIZE];
    if (battery->size() < ramSize + RTC_STATE_SIZE ||
        !battery->loadAt(ramSize, raw, RTC_STATE_SIZE))
    {
        return;
    }

    const u8 *p = raw;
    u8 *fields[] = {&rtc.seconds, &rtc.minutes, &rtc.hours, &rtc.dayLow, &rtc.dayHigh,
                    &latched.seconds, &latched.minutes, &latched.hours, &latched.dayLow, &latched.dayHigh};
    for (u8 *field : fields)
    {
        *field = p[0];
        p += 4;
    }
}

void MBC3::saveRtcState()
{
    u8 raw[RTC_STATE_SIZE];
    std::memset(raw, 0, sizeof(raw));

    u8 *p = raw;
    const u8 fields[] = {rtc.seconds, rtc.minutes, rtc.hours, rtc.dayLow, rtc.dayHigh,
                         latched.seconds, latched.minutes, latched.hours, latched.dayLow, latched.dayHigh};
    for (u8 field : fields)
    {
        p[0] = field;
        p += 4;
    }

    battery->saveAt(ramSize, raw, RTC_STATE_SIZE);
}
