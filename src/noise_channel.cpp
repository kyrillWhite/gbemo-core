#include "noise_channel.h"

void NoiseChannel::resetLengthTimer()
{
    lengthTimer = 64 - (NR[1] & 0b111111);
}

void NoiseChannel::resetEnvelopeTimer()
{
    envelopeTimer = NR[2] & 0b111;
}

u8 NoiseChannel::LFSRWidth()
{
    return NR[3] >> 3 & 1;
}

u16 NoiseChannel::timerPeriod()
{
    u8 divider = NR[3] & 0b111;
    u8 shift = NR[3] >> 4 & 0b1111;
    if (divider) {
        return u64(4194304) * u64(divider) * (u64(1) << shift) / u64(262144);
    }
    else {
        return u64(4194304) * (u64(1) << shift) / u64(524288);
    }
}

NoiseChannel::NoiseChannel() :
    lengthTimer(0),
    envelopeTimer(0),
    volume(15),
    envelopeIncrease(true),
    timer(timerPeriod()),
    LFSR(0x7FFF)
{
    for (int i = 0; i < 5; i++) {
        NR[i] = 0;
    }
    enabled = true;
}

void NoiseChannel::tick() {
    timer--;
    if (timer == 0) {
        timer = timerPeriod();

        bool b0 = bit(LFSR, 0);
        bool b1 = bit(LFSR, 1);
        bool value = b0 ^ b1;
        LFSR = set_bit(LFSR, value, 15);
        if (LFSRWidth()) {
            LFSR = set_bit(LFSR, value, 7);
        }
        LFSR >>= 1;
    }
}

void NoiseChannel::lengthTimerTick()
{
    if (!bit(NR[4], 6)) {
        return;
    }
    if (lengthTimer == 0) {
        return;
    }
    lengthTimer--;

    if (lengthTimer == 0) {
        enabled = false;
    }
}

u8 NoiseChannel::read(u16 address)
{
    address = address - 0xFF1F;
    return NR[address];
}

void NoiseChannel::write(u16 address, u8 value)
{
    address = address - 0xFF1F;
    NR[address] = value;
    switch (address) {
    case 1:
        resetLengthTimer();
        break;
    case 2:
        volume = value >> 4 & 0b1111;
        envelopeIncrease = (value & 0b1000) != 0;
        resetEnvelopeTimer();
        break;
    case 4:
        if (bit(value, 7)) {
            trigger();
        }
        break;
    }
}

void NoiseChannel::trigger()
{
    enabled = true;
    if (lengthTimer == 0) {
        resetLengthTimer();
    }
    timer = timerPeriod();
    resetEnvelopeTimer();
    volume = NR[2] >> 4 & 0b1111;
    LFSR = 0x7FFF;
}

bool NoiseChannel::dac()
{
    return NR[2] & 0xF8;
}

bool NoiseChannel::isEnabled()
{
    return dac() && enabled;
}

double NoiseChannel::getSample()
{
    u8 noiseBit = bit(LFSR, 0);
    return (1 - noiseBit * 2) / 15. * volume;
}

void NoiseChannel::envelopeTimerTick()
{
    u8 envelopePeriod = NR[2] & 0b111;
    if (envelopePeriod == 0) {
        return;
    }
    envelopeTimer--;
    if (envelopeTimer == 0) {
        envelopeTimer = envelopePeriod;
        if (envelopeIncrease && volume < 15) {
            volume++;
        }
        else if (!envelopeIncrease && volume > 0) {
            volume--;
        }
    }
}
