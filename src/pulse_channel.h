#pragma once
#include "audio_channel.h"

class PulseChannel : AudioChannel
{
private:
    bool isFirst;
    u16 timer;
    void resetTimer();

    // ch1 NR10

    int sweepTimer;
    bool sweepEnabled;
    u16 shadowRegister;
    u8 currentSweepPace;
    u8 sweepPace();
    u8 sweepIndividualStep();
    u8 sweepDirection();
    void disableSweep();
    void sweepTrigger();

    // NR11

    u8 lengthTimer;
    void resetLengthTimer();
    u8 duty();
    u8 dutyPos;

    // NR12
    u8 envelopeTimer;
    void resetEnvelopeTimer();
    bool envelopeIncrease;
    u8 volume; // 0-15

    void setFrequency(u16 freq);
    u16 getFrequency();

public:
    PulseChannel(bool _isFirst);

    void tick();
    void sweepTimerTick();
    void lengthTimerTick();
    void envelopeTimerTick();

    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;
    void trigger() override;
    bool dac() override;
    bool isEnabled() override;
    double getSample() override;
    u16 frequencyCalculation(u16 current);
};
