#pragma once
#include "audio_channel.h"

class NoiseChannel : AudioChannel
{
private:
    // NR41
    u8 lengthTimer;
    void resetLengthTimer();

    // NR42
    u8 envelopeTimer;
    void resetEnvelopeTimer();
    bool envelopeIncrease;
    u8 volume; //0-15

    // NR43
    u16 LFSR;
    u32 timer;
    // is short mode or not
    u8 LFSRWidth();
    u16 timerPeriod();

public:
    NoiseChannel();

    void tick();
    void lengthTimerTick();
    void envelopeTimerTick();

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
    void trigger() override;
    bool dac() override;
    bool isEnabled() override;
    double getSample() override;
};

