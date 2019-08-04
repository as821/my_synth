#include "SynthAudioSource.h"
#include "../JuceLibraryCode/JuceHeader.h"

//  SynthAudioSource() constructor
SynthAudioSource::SynthAudioSource (MidiKeyboardState& keyState ) : keyboardState (keyState) {
    for (auto i = 0; i < NUM_VOICES; ++i)
        synth.addVoice (new WaveVoice());
        
    synth.addSound (new WaveSound());
}   // END SynthAudioSource constructor



// SynthAudioSource destructor
SynthAudioSource::~SynthAudioSource() { }   // END ~SynthAudioSource



//  prepareToPlay()
void SynthAudioSource::prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) {
    synth.setCurrentPlaybackSampleRate (sampleRate);
}   // END prepareToPlay()
    


//  getNextAudioBlock()
void SynthAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
    bufferToFill.clearActiveBufferRegion();
    
    /*** MIDI processing section ***/
    MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
    synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}   // END getNextAudioBlock()
