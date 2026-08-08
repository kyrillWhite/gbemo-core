#include "audio/pulse_channel.h"
#include <cstdint>

static constexpr u8 DUTY_TABLE[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5%
    {0, 1, 1, 0, 0, 0, 0, 0}, // 25%
    {0, 1, 1, 1, 1, 0, 0, 0}, // 50%
    {1, 0, 0, 1, 1, 1, 1, 1}  // 75%
};

void PulseChannel::resetTimer()
{
    timer = (2048 - getFrequency()) * 4;
}

u8 PulseChannel::sweepPace()
{
    return NR[0] >> 4 & 0b111;
}

u8 PulseChannel::sweepIndividualStep()
{
    return NR[0] & 0b111;
}

u8 PulseChannel::sweepDirection()
{
    return bit(NR[0], 3);
}

void PulseChannel::resetLengthTimer()
{
    lengthTimer = 64 - (NR[1] & 0b111111);
}

void PulseChannel::resetEnvelopeTimer()
{
    envelopeTimer = NR[2] & 0b111;
}

u8 PulseChannel::duty()
{
    return NR[1] >> 6 & 0b11;
}

void PulseChannel::setFrequency(u16 freq)
{
    NR[3] = freq & 0xFF;
    NR[4] = (NR[4] & 0b11111000) | (freq >> 8 & 0b111);
}

u16 PulseChannel::getFrequency()
{
    return (u16(NR[4] & 0b111) << 8) | NR[3];
}

void PulseChannel::disableSweep()
{
    sweepEnabled = false;
}

void PulseChannel::sweepTrigger()
{
    u8 pace = sweepPace();
    shadowRegister = getFrequency();
    currentSweepPace = pace;
    sweepTimer = pace == 0 ? 8 : pace;
    sweepEnabled = sweepPace() != 0 || sweepIndividualStep() != 0;
    if (sweepIndividualStep() > 0)
    {
        u16 newFreq = frequencyCalculation(shadowRegister);
        if (newFreq > 2047)
        {
            disableSweep();
        }
    }
}

PulseChannel::PulseChannel(bool _isFirst) : isFirst(_isFirst),
                                            sweepTimer(0),
                                            currentSweepPace(0),
                                            sweepEnabled(false),
                                            shadowRegister(0),
                                            lengthTimer(0),
                                            envelopeTimer(0),
                                            dutyPos(0),
                                            volume(15),
                                            envelopeIncrease(true)
{
    for (int i = 0; i < 5; i++)
    {
        NR[i] = 0;
    }
    resetTimer();
    enabled = true;
}

void PulseChannel::tick()
{
    timer--;
    if (timer == 0)
    {
        resetTimer();
        dutyPos = (dutyPos + 1) & 0x7;
    }
}

void PulseChannel::sweepTimerTick()
{
    if (!sweepEnabled)
    {
        return;
    }
    sweepTimer--;

    if (sweepTimer <= 0)
    {
        sweepTimer = currentSweepPace;
        if (currentSweepPace != 0 && sweepIndividualStep() > 0)
        {
            u16 newFreq = frequencyCalculation(shadowRegister);
            if (newFreq > 2047)
            {
                disableSweep();
            }
            else if (sweepIndividualStep() > 0)
            {
                shadowRegister = newFreq;
                setFrequency(newFreq);
                uint16_t secondFreq = frequencyCalculation(shadowRegister);
                if (secondFreq > 2047)
                {
                    disableSweep();
                }
            }
        }
        currentSweepPace = sweepPace();
    }
}

u8 PulseChannel::read(u16 address)
{
    address = address - (isFirst ? 0xFF10 : 0xFF15);
    return NR[address];
}

void PulseChannel::write(u16 address, u8 value)
{
    // TODO: cant't write to sweepPace if sweepTimer is not zero
    // actualy can, but it must affect to value got from
    // sweepPace() until trigger or sweepTimer iterate
    address = address - (isFirst ? 0xFF10 : 0xFF15);
    NR[address] = value;
    switch (address)
    {
    case 1:
        resetLengthTimer();
        break;
    case 2:
        volume = value >> 4 & 0b1111;
        envelopeIncrease = (value & 0b1000) != 0;
        resetEnvelopeTimer();
        break;
    case 4:
        if (bit(value, 7))
        {
            trigger();
        }
        break;
    }
}

void PulseChannel::trigger()
{
    enabled = true;
    if (lengthTimer == 0)
    {
        resetLengthTimer();
    }
    resetEnvelopeTimer();
    volume = NR[2] >> 4 & 0b1111;

    if (isFirst)
    {
        sweepTrigger();
    }
}

bool PulseChannel::dac()
{
    return NR[2] & 0xF8;
}

bool PulseChannel::isEnabled()
{
    return dac() && enabled;
}

double PulseChannel::getSample()
{
    u8 waveBit = DUTY_TABLE[duty()][dutyPos];
    return (1 - waveBit * 2) / 15. * volume;
}

u16 PulseChannel::frequencyCalculation(u16 current)
{
    u16 delta = current >> sweepIndividualStep();
    if (sweepDirection())
    {
        return current - delta;
    }
    else
    {
        return current + delta;
    }
}

void PulseChannel::lengthTimerTick()
{
    if (!bit(NR[4], 6))
    {
        return;
    }
    if (lengthTimer == 0)
    {
        return;
    }
    lengthTimer--;

    if (lengthTimer == 0)
    {
        enabled = false;
    }
}

void PulseChannel::envelopeTimerTick()
{
    u8 envelopePeriod = NR[2] & 0b111;
    if (envelopePeriod == 0)
    {
        return;
    }
    envelopeTimer--;
    if (envelopeTimer == 0)
    {
        envelopeTimer = envelopePeriod;
        if (envelopeIncrease && volume < 15)
        {
            volume++;
        }
        else if (!envelopeIncrease && volume > 0)
        {
            volume--;
        }
    }
}
