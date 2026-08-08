#pragma once

#include "common.h"
#include "cartridge_header.h"
#include "licensee.h"

#include "rom_only.h"
#include "mbc1.h"
#include "mbc2.h"

class Cartridge
{
private:
    u32 romSize;
    u8* romData;

    int validateChecksum();
    void printHeaderInfo(const char* filename);
    int initMemory(const char* filename);
    const char* getSaveFilename(const char* filename);
    u32 getRamSize(u8 type);

public:
    CartridgeMemory* memory;
    CartridgeHeader* header;

    Cartridge();
    ~Cartridge();
    int readRomFile(const char* filename);

    u8 read(u16 address);
    void write(u16 address, u8 value);
};

