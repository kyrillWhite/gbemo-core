#include <cstdio>
#include <cstring>
#include "cartridge/battery_ram.h"

static constexpr char SAVE_EXTENSION[] = ".sav";

BatteryRam::BatteryRam(const char *romFilename) : filename{}
{
    const char *stem = romFilename;
    for (const char *c = romFilename; *c != '\0'; c++)
    {
        if (*c == '/' || *c == '\\')
        {
            stem = c + 1;
        }
    }

    const char *dot = std::strrchr(stem, '.');
    u32 stemSize = static_cast<u32>(dot ? dot - stem : std::strlen(stem));

    // leave room for the extension and the terminator
    const u32 maxStemSize = MAX_FILENAME_SIZE - sizeof(SAVE_EXTENSION);
    if (stemSize > maxStemSize)
    {
        stemSize = maxStemSize;
    }

    std::memcpy(filename.data(), stem, stemSize);
    std::memcpy(filename.data() + stemSize, SAVE_EXTENSION, sizeof(SAVE_EXTENSION));

    if (fopen_s(&writeStream, filename.data(), "r+b") != 0 || !writeStream)
    {
        fopen_s(&writeStream, filename.data(), "wb");
    }
}

BatteryRam::~BatteryRam()
{
    if (writeStream)
    {
        fclose(writeStream);
    }
}

bool BatteryRam::isSaveExist()
{
    FILE *filp;
    fopen_s(&filp, filename.data(), "rb");
    if (!filp)
    {
        return false;
    }
    fclose(filp);
    return true;
}

void BatteryRam::save(const u8 *ramData, u32 ramSize)
{
    if (!writeStream)
    {
        printf("Save error: cannot open \'%s\' for writing\n", filename.data());
        return;
    }

    fseek(writeStream, 0, SEEK_SET);

    if (fwrite(ramData, sizeof(u8), ramSize, writeStream) != ramSize)
    {
        printf("Save error: failed to write all SRAM to \'%s\'\n", filename.data());
    }
    fflush(writeStream);
}

void BatteryRam::load(u8 *ramData, u32 ramSize)
{
    FILE *filp;
    fopen_s(&filp, filename.data(), "rb");
    if (!filp)
    {
        return;
    }

    if (fread(ramData, sizeof(u8), ramSize, filp) != ramSize)
    {
        printf("Load warning: only partially read SRAM from \'%s\'\n", filename.data());
    }
    fclose(filp);
}
