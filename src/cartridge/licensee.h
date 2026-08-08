#pragma once

#include <array>
#include "common.h"

struct LicenseeName
{
    char code[2];
    const char *name;
};

constexpr std::array<LicenseeName, 64> licenseeNames = {{
    {{'0', '0'}, "None"},
    {{'0', '1'}, "Nintendo Research & Development 1"},
    {{'0', '8'}, "Capcom"},
    {{'1', '3'}, "EA (Electronic Arts)"},
    {{'1', '8'}, "Hudson Soft"},
    {{'1', '9'}, "B-AI"},
    {{'2', '0'}, "KSS"},
    {{'2', '2'}, "Planning Office WADA"},
    {{'2', '4'}, "PCM Complete"},
    {{'2', '5'}, "San-X"},
    {{'2', '8'}, "Kemco"},
    {{'2', '9'}, "SETA Corporation"},
    {{'3', '0'}, "Viacom"},
    {{'3', '1'}, "Nintendo"},
    {{'3', '2'}, "Bandai"},
    {{'3', '3'}, "Ocean Software/Acclaim Entertainment"},
    {{'3', '4'}, "Konami"},
    {{'3', '5'}, "HectorSoft"},
    {{'3', '7'}, "Taito"},
    {{'3', '8'}, "Hudson Soft"},
    {{'3', '9'}, "Banpresto"},
    {{'4', '1'}, "Ubi Soft1"},
    {{'4', '2'}, "Atlus"},
    {{'4', '4'}, "Malibu Interactive"},
    {{'4', '6'}, "Angel"},
    {{'4', '7'}, "Bullet-Proof Software2"},
    {{'4', '9'}, "Irem"},
    {{'5', '0'}, "Absolute"},
    {{'5', '1'}, "Acclaim Entertainment"},
    {{'5', '2'}, "Activision"},
    {{'5', '3'}, "Sammy USA Corporation"},
    {{'5', '4'}, "Konami"},
    {{'5', '5'}, "Hi Tech Expressions"},
    {{'5', '6'}, "LJN"},
    {{'5', '7'}, "Matchbox"},
    {{'5', '8'}, "Mattel"},
    {{'5', '9'}, "Milton Bradley Company"},
    {{'6', '0'}, "Titus Interactive"},
    {{'6', '1'}, "Virgin Games Ltd.3"},
    {{'6', '4'}, "Lucasfilm Games4"},
    {{'6', '7'}, "Ocean Software"},
    {{'6', '9'}, "EA (Electronic Arts)"},
    {{'7', '0'}, "Infogrames5"},
    {{'7', '1'}, "Interplay Entertainment"},
    {{'7', '2'}, "Broderbund"},
    {{'7', '3'}, "Sculptured Software6"},
    {{'7', '5'}, "The Sales Curve Limited7"},
    {{'7', '8'}, "THQ"},
    {{'7', '9'}, "Accolade"},
    {{'8', '0'}, "Misawa Entertainment"},
    {{'8', '3'}, "lozc"},
    {{'8', '6'}, "Tokuma Shoten"},
    {{'8', '7'}, "Tsukuda Original"},
    {{'9', '1'}, "Chunsoft Co.8"},
    {{'9', '2'}, "Video System"},
    {{'9', '3'}, "Ocean Software/Acclaim Entertainment"},
    {{'9', '5'}, "Varie"},
    {{'9', '6'}, "Yonezawa/s'pal"},
    {{'9', '7'}, "Kaneko"},
    {{'9', '9'}, "Pack-In-Video"},
    {{'9', 'H'}, "Bottom Up"},
    {{'A', '4'}, "Konami (Yu-Gi-Oh!)"},
    {{'B', 'L'}, "MTO"},
    {{'D', 'K'}, "Kodansha"},
}};

constexpr const char *getLicenseeName(const char code[2])
{
    for (const auto &entry : licenseeNames)
    {
        if (entry.code[0] == code[0] && entry.code[1] == code[1])
        {
            return entry.name;
        }
    }
    return "Unknown";
}
