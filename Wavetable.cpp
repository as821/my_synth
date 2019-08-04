#include "Wavetable.h"

Wavetable::Wavetable (const AudioSampleBuffer& wavetableToUse) : wavetable (wavetableToUse), tableSize (wavetable.getNumSamples() - 1)
{
    jassert (wavetable.getNumChannels() == 1);
}

void Wavetable::setFrequency (float frequency, float sampleRate)
{
    auto tableSizeOverSampleRate = tableSize / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}
