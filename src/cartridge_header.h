#pragma once

#include "cartridge_type.h"

struct CartridgeHeader
{
    u8 entryPoint[4];            // 0100-0103 � Entry point
    u8 logo[48];                 // 0104-0133 � Nintendo logo
    char title[15];              // 0134-0142 � Title
    bool cgbFlag;                // 0143 � CGB flag
    const char licenseeCode[2];  // 0144�0145 � New licensee code
    bool sgbFlag;                // 0146 � SGB flag
    CartridgeType cartridgeType; // 0147 � Cartridge type
    u8 romSize;                  // 0148 � ROM size
    u8 ramSize;                  // 0149 � RAM size
    u8 destinationCode;          // 014A � Destination code
    u8 oldLicenseeCode;          // 014B � Old licensee code
    u8 version;                  // 014C � Mask ROM version number
    u8 headerChecksum;           // 014D � Header checksum
    u8 globalChecksum[2];        // 014E-014F � Global checksum
};