#pragma once
#include "common.h"
#include "pulse_channel.h"
#include "noise_channel.h"
#include "wave_channel.h"

class APU
{
private:
    u8 NR52; // 0xFF26 audio master control
    u8 NR51; // 0xFF25 sound panning
    u8 NR50; // 0xFF24 master volume & VIN panning

    u16 div;
    double leftCapacitor;
    double rightCapacitor;

    PulseChannel *ch1;
    PulseChannel *ch2;
    WaveChannel *ch3;
    NoiseChannel *ch4;

    double leftHighPass(double in, bool dacsEnabled);
    double rightHighPass(double in, bool dacsEnabled);

public:
    APU();
    ~APU();

    u8 read(u16 address);
    void write(u16 address, u8 value);

    // before tick
    void increaseDiv();
    void tick();

    i16 getLeftSample();
    i16 getRightSample();
};
