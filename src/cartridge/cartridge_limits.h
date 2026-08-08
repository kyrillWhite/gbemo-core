#pragma once

#include <algorithm>
#include "common.h"

constexpr u32 KIB = 1024;

// no banking at all: a single 32 KiB ROM, no RAM
constexpr u32 ROM_ONLY_MAX_ROM_SIZE = 32 * KIB;
constexpr u32 ROM_ONLY_MAX_RAM_SIZE = 0;

// 512 KiB ROM with up to 32 KiB of banked RAM
constexpr u32 MBC1_MAX_ROM_SIZE = 512 * KIB;
constexpr u32 MBC1_MAX_RAM_SIZE = 32 * KIB;

// 256 KiB ROM and 512x4 bits of built-in RAM (stored one nibble per byte)
constexpr u32 MBC2_MAX_ROM_SIZE = 256 * KIB;
constexpr u32 MBC2_MAX_RAM_SIZE = 512;

// the ROM buffer Cartridge keeps has to fit the largest of them
constexpr u32 MAX_ROM_SIZE = std::max({ROM_ONLY_MAX_ROM_SIZE,
                                       MBC1_MAX_ROM_SIZE,
                                       MBC2_MAX_ROM_SIZE});
