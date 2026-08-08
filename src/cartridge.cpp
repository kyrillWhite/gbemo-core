#include <filesystem>
#include "cartridge.h"

Cartridge::Cartridge():
    romSize(0),
    romData(nullptr),
    memory(nullptr),
    header(nullptr)
{}

Cartridge::~Cartridge()
{
    if (memory != nullptr) {
        delete memory;
    }
    if (romData != nullptr) {
        delete[] romData;
    }
}

int Cartridge::validateChecksum()
{
    u8 checksum = 0;
    for (u16 address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - romData[address] - 1;
    }

    if (romData[0x014D] != checksum) {
        printf("Invalid checksum: %#04X\n\n", checksum);
        return -1;
    }
    else {
        printf("Checksum is valid: %#04X\n\n", checksum);
        return 0;
    }
}

void Cartridge::printHeaderInfo(const char* filename)
{
    auto cartridgeTypeName = cartridgeTypeNames.at(header->cartridgeType);
    std::string keyStr(header->licenseeCode, 2);
    const char* licenseeName;
    if (licenseeNames.contains(keyStr)) {
        licenseeName = licenseeNames.at(keyStr);
    }
    else {
        licenseeName = "Unknown";
    }

    printf("ROM \"%s\" info\n", filename);
    printf("\tTitle                 %40.*s\n", 15, header->title);
    printf("\tGameBoy Color support %40s\n", header->cgbFlag ? "true" : "false");
    printf("\tLicensee              %40s\n", licenseeName);
    printf("\tSuper GameBoy support %40s\n", header->sgbFlag ? "true" : "false");
    printf("\tCartidge type         %40s\n", cartridgeTypeName);
    printf("\tROM size              %#37d KB\n", 32 << header->romSize);
    printf("\tRAM size              %#37d KB\n", getRamSize(header->ramSize) >> 10);
    printf("\tDestination           %40s\n", header->destinationCode ? "USA, Europe" : "Japan");
    printf("\tOld licencee code     %#40X\n", header->oldLicenseeCode);
    printf("\tVersion               %#40X\n", header->version);
    printf("\tChecksum              %#40X\n\n", header->headerChecksum);
}

int Cartridge::readRomFile(const char* filename)
{
    FILE* filp;
    fopen_s(&filp, filename, "rb");
    if (!filp) {
        printf("Could not open file %s\n", filename);
        return -1;
    }
    fseek(filp, 0, SEEK_END);
    romSize = ftell(filp);
    romData = new u8[romSize];
    rewind(filp);

    fread(romData, sizeof(u8), romSize, filp);
    fclose(filp);

    header = (CartridgeHeader*)(romData + 0x0100);
    printHeaderInfo(filename);

    if (validateChecksum()) {
        return -1;
    }
    if (initMemory(filename)) {
        return -1;
    }

    return 0;
}

int Cartridge::initMemory(const char* filename)
{
    BatteryRam* battery = nullptr;
    u32 ramSize = getRamSize(header->ramSize);
    
    switch (header->cartridgeType)
    {
    case CartridgeType::ROM_ONLY:
        memory = new RomOnly(romSize, romData);
        break;
    case CartridgeType::MBC1:
        memory = new MBC1(romSize, romData);
        break;
    case CartridgeType::MBC1_RAM:
        memory = new MBC1(romSize, romData, ramSize);
        break;
    case CartridgeType::MBC1_RAM_BATTERY:
        battery = new BatteryRam(getSaveFilename(filename));
        memory = new MBC1(romSize, romData, ramSize, battery);
        break;
    case CartridgeType::MBC2:
        memory = new MBC2(romSize, romData);
        break;
    case CartridgeType::MBC2_BATTERY:
        battery = new BatteryRam(getSaveFilename(filename));
        memory = new MBC2(romSize, romData, battery);
        break;
    case CartridgeType::MBC3:
        memory = new MBC1(romSize, romData);
        break;
    case CartridgeType::MBC3_RAM_10:
        memory = new MBC1(romSize, romData, ramSize);
        break;
    case CartridgeType::MBC3_RAM_BATTERY_10:
        battery = new BatteryRam(getSaveFilename(filename));
        memory = new MBC1(romSize, romData, ramSize, battery);
        break;
    default:
        auto cartridgeTypeName = cartridgeTypeNames.at(header->cartridgeType);
        printf("Cartridge with type \"%s\" is not supported\n\n", cartridgeTypeName);
        return -1;
    }

    return 0;
}

const char* Cartridge::getSaveFilename(const char* filename)
{
    thread_local std::string buf;
    std::filesystem::path p(filename);
    buf = p.stem().string() + ".sav";
    return buf.c_str();
}

u32 Cartridge::getRamSize(u8 type)
{
    switch (type) {
    case 0:
    case 1:
        return 0;
    case 2:
        return 1 << 13; // 8KiB
    case 3:
        return 1 << 15; // 32KiB
    case 4:
        return 1 << 17; // 128KiB
    case 5:
        return 1 << 16; // 64KiB
    default:
        printf("Ivalid ram size: \"%X\"\n\n", type);
        return 0;
    }
}

u8 Cartridge::read(u16 address) {
    return memory->read(address);
}

void Cartridge::write(u16 address, u8 value) {
    memory->write(address, value);
}
