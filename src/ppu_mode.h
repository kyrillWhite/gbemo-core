#pragma once

enum PpuMode {
    HBLANK,
    VBLANK,
    OAM,
    XFER,
};

enum StatSrc {
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6),
};