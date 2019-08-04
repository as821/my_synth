#ifndef SynthAudioSource_h
#define SynthAudioSource_h

#include "WaveSoundVoice.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define NUM_VOICES 4


// SynthAudioSource class declaration
class SynthAudioSource   : public AudioSource
{
    
public:
    SynthAudioSource ( MidiKeyboardState& keyState );
    ~SynthAudioSource();
    
    //void setUsingSineWaveSound();
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override;
    
    void releaseResources() override {}
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    Synthesiser synth;      // *** find another way to pass parameterArray to WaveVoice that allows this member to stay private TODO ***

    
private:
    MidiKeyboardState& keyboardState;
    
};  // END SynthAudioSource class



#endif
