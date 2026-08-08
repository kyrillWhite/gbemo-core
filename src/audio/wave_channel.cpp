#include "audio/wave_channel.h"

u8 WaveChannel::getWaveSample(u8 index)
{
    u8 sampleByte = wavePattern[index / 2];
    u8 nibbleIndex = index % 2;
    return sampleByte >> ((1 - nibbleIndex) * 4) & 0b1111;
}

void WaveChannel::resetLengthTimer()
{
    lengthTimer = 256 - NR[1];
}

u8 WaveChannel::getVolume()
{
    return NR[2] >> 5 & 0b11;
}

u16 WaveChannel::getPeriod()
{
    u16 freq = (u16(NR[4] & 0x7) << 8) | NR[3];
    // multiplied by 2 because wave freq is 2 times lower than cpu
    return (2048 - freq) * 2;
}

WaveChannel::WaveChannel() : lengthTimer(0),
                             wavePos(0),
                             volume(0),
                             periodDivider(0)
{
    for (int i = 0; i < 16; i++)
    {
        wavePattern[i] = 0;
    }
    enabled = true;
}

void WaveChannel::tick()
{
    periodDivider--;
    if (periodDivider == 0)
    {
        periodDivider = getPeriod();

        wavePos++;
        if (wavePos == 32)
        {
            wavePos = 0;
        }
    }
}

void WaveChannel::lengthTimerTick()
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

u8 WaveChannel::read(u16 address)
{
    if (address >= 0xFF30 && address <= 0xFF3F)
    {
        return 0xFF;
    }

    address = address - 0xFF1A;
    return NR[address];
}

void WaveChannel::write(u16 address, u8 value)
{
    if (address >= 0xFF30 && address <= 0xFF3F)
    {
        // TODO: write only if channel "enabled"
        address = address - 0xFF30;
        wavePattern[address] = value;
    }
    else
    {
        address = address - 0xFF1A;
        NR[address] = value;
        switch (address)
        {
        case 1:
            resetLengthTimer();
            break;
        case 2:
            volume = getVolume();
            break;
        case 4:
            if (bit(value, 7))
            {
                trigger();
            }
            break;
        }
    }
}

void WaveChannel::trigger()
{
    enabled = true;
    if (lengthTimer == 0)
    {
        resetLengthTimer();
    }
    periodDivider = getPeriod();
    volume = getVolume();
    wavePos = 0;
}

bool WaveChannel::dac()
{
    return bit(NR[0], 7);
}

bool WaveChannel::isEnabled()
{
    return dac() && enabled;
}

double WaveChannel::getSample()
{
    u8 sample = 0;
    u8 waveSample = getWaveSample(wavePos);
    switch (volume)
    {
    case 0:
        sample = 0;
        break;
    case 1:
        sample = waveSample;
        break;
    case 2:
        sample = waveSample >> 1;
        break;
    case 3:
        sample = waveSample >> 2;
        break;
    }
    return (sample * 2 - 15) / 15.;
}
