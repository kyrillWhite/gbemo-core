#pragma once

#include <array>
#include <optional>
#include <variant>

#include "common.h"
#include "cartridge/cartridge_header.h"
#include "cartridge/cartridge_limits.h"
#include "cartridge/licensee.h"

#include "cartridge/rom_only.h"
#include "cartridge/mbc1.h"
#include "cartridge/mbc2.h"
#include "cartridge/mbc3.h"
#include "cartridge/mbc5.h"

class Cartridge
{
private:
    u32 romSize;
    std::array<u8, MAX_ROM_SIZE> romData;

    std::optional<BatteryRam> battery;
    // Destroyed before `battery`, which the MBC3 clock still needs on its way
    // out - keep it declared last.
    std::variant<std::monostate, RomOnly, MBC1, MBC2, MBC3, MBC5> memoryStorage;

    int validateChecksum();
    void printHeaderInfo(const char *filename);
    int initMemory(const char *filename);
    u32 getRamSize(u8 type);

public:
    CartridgeMemory *memory;
    const CartridgeHeader *header;

    Cartridge();

    int readRomFile(const char *filename);

    u8 read(u16 address);
    void write(u16 address, u8 value);
};
