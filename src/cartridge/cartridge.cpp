#include <cstdio>
#include "cartridge/cartridge.h"

Cartridge::Cartridge() : romSize(0),
                         romData{},
                         memory(nullptr),
                         header(nullptr)
{
}

int Cartridge::validateChecksum()
{
    u8 checksum = 0;
    for (u16 address = 0x0134; address <= 0x014C; address++)
    {
        checksum = checksum - romData[address] - 1;
    }

    if (romData[0x014D] != checksum)
    {
        printf("Invalid checksum: %#04X\n\n", checksum);
        return -1;
    }
    else
    {
        printf("Checksum is valid: %#04X\n\n", checksum);
        return 0;
    }
}

void Cartridge::printHeaderInfo(const char *filename)
{
    printf("ROM \"%s\" info\n", filename);
    printf("\tTitle                 %40.*s\n", 15, header->title);
    printf("\tGameBoy Color support %40s\n", header->cgbFlag ? "true" : "false");
    printf("\tLicensee              %40s\n", getLicenseeName(header->licenseeCode));
    printf("\tSuper GameBoy support %40s\n", header->sgbFlag ? "true" : "false");
    printf("\tCartidge type         %40s\n", getCartridgeTypeName(header->cartridgeType));
    printf("\tROM size              %#37d KB\n", 32 << header->romSize);
    printf("\tRAM size              %#37d KB\n", getRamSize(header->ramSize) >> 10);
    printf("\tDestination           %40s\n", header->destinationCode ? "USA, Europe" : "Japan");
    printf("\tOld licencee code     %#40X\n", header->oldLicenseeCode);
    printf("\tVersion               %#40X\n", header->version);
    printf("\tChecksum              %#40X\n\n", header->headerChecksum);
}

int Cartridge::readRomFile(const char *filename)
{
    FILE *filp;
    fopen_s(&filp, filename, "rb");
    if (!filp)
    {
        printf("Could not open file %s\n", filename);
        return -1;
    }
    fseek(filp, 0, SEEK_END);

    romSize = static_cast<u32>(ftell(filp));
    if (romSize > MAX_ROM_SIZE)
    {
        printf("ROM is %u KiB, only the first %u KiB are kept\n\n", romSize / KIB, MAX_ROM_SIZE / KIB);
        romSize = MAX_ROM_SIZE;
    }
    rewind(filp);

    fread(romData.data(), sizeof(u8), romSize, filp);
    fclose(filp);

    header = reinterpret_cast<const CartridgeHeader *>(romData.data() + 0x0100);
    printHeaderInfo(filename);

    if (validateChecksum())
    {
        return -1;
    }
    if (initMemory(filename))
    {
        return -1;
    }

    return 0;
}

int Cartridge::initMemory(const char *filename)
{
    const u32 ramSize = getRamSize(header->ramSize);

    switch (header->cartridgeType)
    {
    case CartridgeType::ROM_ONLY:
        memory = &memoryStorage.emplace<RomOnly>(romSize, romData.data());
        break;
    case CartridgeType::MBC1:
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data());
        break;
    case CartridgeType::MBC1_RAM:
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data(), ramSize);
        break;
    case CartridgeType::MBC1_RAM_BATTERY:
        battery.emplace(filename);
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data(), ramSize, &battery.value());
        break;
    case CartridgeType::MBC2:
        memory = &memoryStorage.emplace<MBC2>(romSize, romData.data());
        break;
    case CartridgeType::MBC2_BATTERY:
        battery.emplace(filename);
        memory = &memoryStorage.emplace<MBC2>(romSize, romData.data(), &battery.value());
        break;
    case CartridgeType::MBC3:
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data());
        break;
    case CartridgeType::MBC3_RAM_10:
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data(), ramSize);
        break;
    case CartridgeType::MBC3_RAM_BATTERY_10:
        battery.emplace(filename);
        memory = &memoryStorage.emplace<MBC1>(romSize, romData.data(), ramSize, &battery.value());
        break;
    default:
        printf("Cartridge with type \"%s\" is not supported\n\n", getCartridgeTypeName(header->cartridgeType));
        return -1;
    }

    return 0;
}

u32 Cartridge::getRamSize(u8 type)
{
    switch (type)
    {
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

u8 Cartridge::read(u16 address)
{
    return memory->read(address);
}

void Cartridge::write(u16 address, u8 value)
{
    memory->write(address, value);
}
