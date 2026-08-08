#pragma once

#include <array>
#include "common.h"

enum class CartridgeType : u8
{
    ROM_ONLY = 0x0000,
    MBC1 = 0x0001,
    MBC1_RAM = 0x0002,
    MBC1_RAM_BATTERY = 0x0003,
    MBC2 = 0x0005,
    MBC2_BATTERY = 0x0006,
    ROM_RAM_9 = 0x0008,
    ROM_RAM_BATTERY_9 = 0x0009,
    MMM01 = 0x000B,
    MMM01_RAM = 0x000C,
    MMM01_RAM_BATTERY = 0x000D,
    MBC3_TIMER_BATTERY = 0x000F,
    MBC3_TIMER_RAM_BATTERY_10 = 0x0010,
    MBC3 = 0x0011,
    MBC3_RAM_10 = 0x0012,
    MBC3_RAM_BATTERY_10 = 0x0013,
    MBC5 = 0x0019,
    MBC5_RAM = 0x001A,
    MBC5_RAM_BATTERY = 0x001B,
    MBC5_RUMBLE = 0x001C,
    MBC5_RUMBLE_RAM = 0x001D,
    MBC5_RUMBLE_RAM_BATTERY = 0x001E,
    MBC6 = 0x0020,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x0022,
    POCKET_CAMERA = 0x00FC,
    BANDAI_TAMA5 = 0x00FD,
    HuC3 = 0x00FE,
    HuC1_RAM_BATTERY = 0x00FF,
};

struct CartridgeTypeName
{
    CartridgeType type;
    const char *name;
};

constexpr std::array<CartridgeTypeName, 28> cartridgeTypeNames = {{
    {CartridgeType::ROM_ONLY, "ROM ONLY"},
    {CartridgeType::MBC1, "MBC1"},
    {CartridgeType::MBC1_RAM, "MBC1 + RAM"},
    {CartridgeType::MBC1_RAM_BATTERY, "MBC1 + RAM + BATTERY"},
    {CartridgeType::MBC2, "MBC2"},
    {CartridgeType::MBC2_BATTERY, "MBC2 + BATTERY"},
    {CartridgeType::ROM_RAM_9, "ROM + RAM 9"},
    {CartridgeType::ROM_RAM_BATTERY_9, "ROM + RAM + BATTERY 9"},
    {CartridgeType::MMM01, "MMM01"},
    {CartridgeType::MMM01_RAM, "MMM01 + RAM"},
    {CartridgeType::MMM01_RAM_BATTERY, "MMM01 + RAM + BATTERY"},
    {CartridgeType::MBC3_TIMER_BATTERY, "MBC3 + TIMER + BATTERY"},
    {CartridgeType::MBC3_TIMER_RAM_BATTERY_10, "MBC3 + TIMER + RAM + BATTERY 10"},
    {CartridgeType::MBC3, "MBC3"},
    {CartridgeType::MBC3_RAM_10, "MBC3 + RAM 10"},
    {CartridgeType::MBC3_RAM_BATTERY_10, "MBC3 + RAM + BATTERY 10"},
    {CartridgeType::MBC5, "MBC5"},
    {CartridgeType::MBC5_RAM, "MBC5 + RAM"},
    {CartridgeType::MBC5_RAM_BATTERY, "MBC5 + RAM + BATTERY"},
    {CartridgeType::MBC5_RUMBLE, "MBC5 + RUMBLE"},
    {CartridgeType::MBC5_RUMBLE_RAM, "MBC5 + RUMBLE + RAM"},
    {CartridgeType::MBC5_RUMBLE_RAM_BATTERY, "MBC5 + RUMBLE + RAM + BATTERY"},
    {CartridgeType::MBC6, "MBC6"},
    {CartridgeType::MBC7_SENSOR_RUMBLE_RAM_BATTERY, "MBC7 + SENSOR + RUMBLE + RAM + BATTERY"},
    {CartridgeType::POCKET_CAMERA, "POCKET CAMERA"},
    {CartridgeType::BANDAI_TAMA5, "BANDAI TAMA5"},
    {CartridgeType::HuC3, "HuC3"},
    {CartridgeType::HuC1_RAM_BATTERY, "HuC1 + RAM + BATTERY"},
}};

constexpr const char *getCartridgeTypeName(CartridgeType type)
{
    for (const auto &entry : cartridgeTypeNames)
    {
        if (entry.type == type)
        {
            return entry.name;
        }
    }
    return "Unknown";
}
