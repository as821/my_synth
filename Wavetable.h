#ifndef Wavetable_h
#define Wavetable_h

#include "../JuceLibraryCode/JuceHeader.h"


class Wavetable
{
public:
    Wavetable (const AudioSampleBuffer& wavetableToUse);

    void setFrequency (float frequency, float sampleRate);

    forcedinline float getNextSample() noexcept
    {
        auto index0 = (unsigned int) currentIndex;
        auto index1 = index0 + 1;

        auto frac = currentIndex - (float) index0;

        auto* table = wavetable.getReadPointer (0);
        auto value0 = table[index0];
        auto value1 = table[index1];

        auto currentSample = value0 + frac * (value1 - value0);

        if ((currentIndex += tableDelta) > tableSize)
            currentIndex -= tableSize;

        return currentSample;
    }

private:
    const AudioSampleBuffer& wavetable;
    const int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f;
};


#endif
