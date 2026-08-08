# gbemo

A Game Boy (DMG) emulation core in C++20, packaged as a library.

This repository contains **only the machine** — CPU, memory, cartridge
mappers, PPU, APU, timers and interrupts. It draws nothing, plays nothing and
reads no keyboard. It builds a DLL / static library plus headers that a
frontend links against; the window, audio device and input handling live in a
separate repository that consumes this package.

## API

The whole surface is `Emu` in [emu.h](include/emu.h):

```cpp
#include "emu.h"

Emu emu(/* skipBoot = */ true);

if (int err = emu.loadRom("rom.gb"))
    return err;                      // non-zero on failure

for (;;) {
    emu.tick();                      // advances one T-cycle (~4.194304 MHz)

    i16 left  = emu.getLeftAudioSample();
    i16 right = emu.getRightAudioSample();

    MonoColor* frame = emu.getVideoBuffer();   // 160 * 144, 0..3 per pixel
}

emu.pressButton(Button::B_A);
emu.releaseButton(Button::B_A);
```

`MonoColor` is `White | LightGray | DarkGray | Black` (0–3) — map it to
whatever palette you like. `Button` covers `B_START`, `B_SELECT`, `B_A`,
`B_B`, `B_UP`, `B_DOWN`, `B_LEFT`, `B_RIGHT`. `debugReadTiles()` fills a
`MonoColor[384 * 64]` buffer with the VRAM tile atlas for debug views.

Pacing is the caller's job. Tick the core against your own clock, sample audio
at your output rate, and present the framebuffer when you want a frame —
one Game Boy frame is 17556 machine cycles (70224 T-cycles).

## Building

Requires MinGW `g++` with C++20 support and GNU `make`.

```
make            # debug
make release    # optimized
make clean
make x86 release   # 32-bit (needs an i686-w64-mingw32 toolchain)
make x64 release   # 64-bit (default)
```

`x86` and `x64` are selectors: they set the architecture for the goal named
alongside them and build nothing on their own. The 32-bit toolchain is found
either on `PATH` under its cross name, or as an MSYS2 MINGW32 install at
`MINGW32_BIN` (default `/c/msys64/mingw32/bin`).

A build produces the static library `bin/<debug|release>/<arch>/libgbemo.a`;
add the `shared` selector to also get `gbemo.dll` and `libgbemo.dll.a`. There
is no executable target.

## Releases

```
make VERSION=1.0.0 x64 dist
```

packs `dist/gbemo-1.0.0-x64.zip`, one archive per architecture:

```
include/   emu.h  common.h  buttons.h  mono_color.h
lib/       libgbemo.a
README.md
```

That header set is the entire API — `emu.h` forward-declares the subsystems, so
nothing internal leaks into it. Static only by default: `Emu` is a C++ class
with no C ABI façade, so a DLL would tie you to this exact compiler and runtime
version; `make shared dist` adds one if you want it anyway.

To consume the package:

```
g++ -std=c++20 -Igbemo-1.0.0-x64/include main.cpp \
    gbemo-1.0.0-x64/lib/libgbemo.a -o app
```

## Layout

```
include/       public headers (emu.h, common.h, buttons.h, mono_color.h)
src/cpu/       CPU: core, control unit, ALU, IDU, register file
src/memory/    memory map, buses, I/O registers
src/cartridge/ cartridge header and MBCs
src/video/     PPU, pixel FIFO, LCD, OAM DMA
src/audio/     APU and its channels
src/system/    Emu façade, clock, timer, interrupts, joypad
src/debug/     serial-output test harness reader
```
