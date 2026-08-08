#include "battery_ram.h"
#include <fstream>
#include <filesystem>

BatteryRam::BatteryRam(const char *_filename) : filename(_filename)
{
}

bool BatteryRam::saveExists()
{
    return std::filesystem::exists(filename);
}

void BatteryRam::save(u8 *ramData, u32 ramSize)
{
    std::ofstream out(filename, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        printf("Save error: cannot open \'%s\' for writing\n", filename);
        return;
    }
    out.write(reinterpret_cast<const char *>(ramData), static_cast<std::streamsize>(ramSize));
    if (!out)
    {
        printf("Save error: failed to write all SRAM to \'%s\'\n", filename);
    }
}

void BatteryRam::load(u8 *ramData, u32 ramSize)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
    {
        return;
    }
    in.read(reinterpret_cast<char *>(ramData), static_cast<std::streamsize>(ramSize));
    if (!in)
    {
        printf("Load warning: only partially read SRAM from \'%s\'\n", filename);
    }
}
