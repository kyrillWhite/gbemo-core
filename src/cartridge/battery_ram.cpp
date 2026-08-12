#include <cstdio>
#include <cstring>
#include "cartridge/battery_ram.h"

static constexpr char SAVE_EXTENSION[] = ".sav";

BatteryRam::BatteryRam(const char *romFilename) : stream(nullptr), saveExists(false), fileSize(0), filename{}
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

    saveExists = fopen_s(&stream, filename.data(), "r+b") == 0 && stream;
    if (!saveExists)
    {
        fopen_s(&stream, filename.data(), "w+b");
    }
    else
    {
        fseek(stream, 0, SEEK_END);
        fileSize = static_cast<u32>(ftell(stream));
    }
}

BatteryRam::~BatteryRam()
{
    if (stream)
    {
        fclose(stream);
    }
}

bool BatteryRam::isSaveExist()
{
    return saveExists;
}

u32 BatteryRam::size() const
{
    return fileSize;
}

void BatteryRam::save(const u8 *ramData, u32 ramSize)
{
    saveAt(0, ramData, ramSize);
}

void BatteryRam::saveAt(u32 offset, const u8 *data, u32 size)
{
    if (!stream)
    {
        printf("Save error: cannot open \'%s\' for writing\n", filename.data());
        return;
    }

    fseek(stream, static_cast<long>(offset), SEEK_SET);

    if (fwrite(data, sizeof(u8), size, stream) != size)
    {
        printf("Save error: failed to write all SRAM to \'%s\'\n", filename.data());
    }
    fflush(stream);

    if (offset + size > fileSize)
    {
        fileSize = offset + size;
    }
}

void BatteryRam::saveByte(u32 offset, u8 value)
{
    saveAt(offset, &value, 1);
}

void BatteryRam::load(u8 *ramData, u32 ramSize)
{
    if (!loadAt(0, ramData, ramSize))
    {
        printf("Load warning: only partially read SRAM from \'%s\'\n", filename.data());
    }
}

bool BatteryRam::loadAt(u32 offset, u8 *data, u32 size)
{
    if (!stream)
    {
        return false;
    }

    fseek(stream, static_cast<long>(offset), SEEK_SET);

    return fread(data, sizeof(u8), size, stream) == size;
}
