#include "apu.h"
#include <cmath>


double APU::leftHighPass(double in, bool dacsEnabled)
{
    double out = 0.0;
    if (dacsEnabled)
    {
        out = in - leftCapacitor;

        // capacitor slowly charges to 'in' via their difference
        leftCapacitor = in - out * 0.999958; // use 0.998943 for MGB&CGB
    }
    return out;
}

double APU::rightHighPass(double in, bool dacsEnabled)
{
    double out = 0.0;
    if (dacsEnabled)
    {
        out = in - rightCapacitor;

        // capacitor slowly charges to 'in' via their difference
        rightCapacitor = in - out * 0.999958; // use 0.998943 for MGB&CGB
    }
    return out;
}

APU::APU() :
    ch1(new PulseChannel(true)),
    ch2(new PulseChannel(false)),
    ch3(new WaveChannel()),
    ch4(new NoiseChannel()),
    NR52(0),
    NR51(0),
    NR50(0),
    div(0),
    leftCapacitor(0),
    rightCapacitor(0)
{
}

APU::~APU()
{
    delete ch1;
    delete ch2;
    delete ch3;
    delete ch4;
}

u8 APU::read(u16 address)
{
    if (address >= 0xFF10 && address <= 0xFF14) {
        return ch1->read(address);
    }
    else if (address >= 0xFF16 && address <= 0xFF19) {
        return ch2->read(address);
    }
    else if (address >= 0xFF1A && address <= 0xFF1E) {
        return ch3->read(address);
    }
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        // wave pattern
        return 0xFF;
    }
    else if (address >= 0xFF20 && address <= 0xFF23) {
        return ch4->read(address);
    }
    else if (address == 0xFF26) {
        return NR52;
    }
    else if (address == 0xFF25) {
        return NR51;
    }
    else if (address == 0xFF24) {
        return NR50;
    }
    return 0;
}

void APU::write(u16 address, u8 value)
{
    if (address >= 0xFF10 && address <= 0xFF14) {
        ch1->write(address, value);
    }
    else if (address >= 0xFF16 && address <= 0xFF19) {
        ch2->write(address, value);
    }
    else if (address >= 0xFF1A && address <= 0xFF1E) {
        ch3->write(address, value);
    }
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        ch3->write(address, value);
    }
    else if (address >= 0xFF20 && address <= 0xFF23) {
        ch4->write(address, value);
    }
    else if (address == 0xFF26) {
        NR52 = value;
    }
    else if (address == 0xFF25) {
        NR51 = value;
    }
    else if (address == 0xFF24) {
        NR50 = value;
    }
}

void APU::increaseDiv()
{
    u16 prevDiv = div;
    div++;

    bool envelopSweep = (prevDiv & (1 << 10)) && (!(div & (1 << 10)));
    bool ch1FreqSweep = (prevDiv & (1 << 9)) && (!(div & (1 << 9)));
    bool soundLength = (prevDiv & (1 << 8)) && (!(div & (1 << 8)));

    if (envelopSweep) {
        ch1->envelopeTimerTick();
        ch2->envelopeTimerTick();
        ch4->envelopeTimerTick();
    }

    if (soundLength) {
        ch1->lengthTimerTick();
        ch2->lengthTimerTick();
        ch3->lengthTimerTick();
        ch4->lengthTimerTick();
    }

    if (ch1FreqSweep) {
        ch1->sweepTimerTick();
    }
}

void APU::tick()
{
    ch1->tick();
    ch2->tick();
    ch3->tick();
    ch4->tick();
}

//#define M_PI       3.14159265358979323846   // pi
//i16 APU::getSample()
//{
//    // sin
//    //double phase = 2.0 * M_PI * 440.0 * (globalTicks / double(1 << 22));
//    //std::sin(phase) * 32767;
//
//
//    return std::rand() % 32767 - 16384;
//}

i16 APU::getLeftSample()
{
    if (!bit(NR52, 7)) {
        return 0;
    }

    bool ch1Enabled = (NR51 >> 4 & 1) && ch1->isEnabled();
    bool ch2Enabled = (NR51 >> 5 & 1) && ch2->isEnabled();
    bool ch3Enabled = (NR51 >> 6 & 1) && ch3->isEnabled();
    bool ch4Enabled = (NR51 >> 7 & 1) && ch4->isEnabled();

    bool dacsEnabled = ch1->dac() || ch2->dac() || ch3->dac() || ch4->dac();

    double ch1Value = ch1Enabled ? ch1->getSample() : 0;
    double ch2Value = ch2Enabled ? ch2->getSample() : 0;
    double ch3Value = ch3Enabled ? ch3->getSample() : 0;
    double ch4Value = ch4Enabled ? ch4->getSample() : 0;
    
    u8 volume = (NR50 >> 4 & 0b111) + 1;
    double sumValue = ch1Value + ch2Value + ch3Value + ch4Value;
    i16 sample = leftHighPass(sumValue, dacsEnabled) * volume / 8. * 32767 / 4.;

    return sample;
}

i16 APU::getRightSample()
{
    if (!bit(NR52, 7)) {
        return 0;
    }

    bool ch1Enabled = (NR51 >> 0 & 1) && ch1->isEnabled();
    bool ch2Enabled = (NR51 >> 1 & 1) && ch2->isEnabled();
    bool ch3Enabled = (NR51 >> 2 & 1) && ch3->isEnabled();
    bool ch4Enabled = (NR51 >> 3 & 1) && ch4->isEnabled();

    bool dacsEnabled = ch1->dac() || ch2->dac() || ch3->dac() || ch4->dac();

    double ch1Value = ch1Enabled ? ch1->getSample() : 0;
    double ch2Value = ch2Enabled ? ch2->getSample() : 0;
    double ch3Value = ch3Enabled ? ch3->getSample() : 0;
    double ch4Value = ch4Enabled ? ch4->getSample() : 0;

    u8 volume = (NR50 & 0b111) + 1; // 1-8
    double sumValue = ch1Value + ch2Value + ch3Value + ch4Value;
    i16 sample = rightHighPass(sumValue, dacsEnabled) * volume / 8. * 32767 / 4.;

    return sample;
}
