#pragma once
#include "audio_channel.h"

class WaveChannel : AudioChannel
{
private:
    u8 wavePattern[16];
    u8 getWaveSample(u8 index);

    // NR31

    u8 lengthTimer;
    void resetLengthTimer();
    u8 wavePos;

    // NR32

    u8 volume;
    u8 getVolume();

    // NR33

    u16 periodDivider;
    u16 getPeriod();

public:
    WaveChannel();

    void tick();
    void lengthTimerTick();

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
    void trigger() override;
    bool dac() override;
    bool isEnabled() override;
    double getSample() override;
};
